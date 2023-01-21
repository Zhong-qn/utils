/**
 * @file socket.c
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "ut/ut_socket.h"
#include "ut/ut_hash.h"
#include "ut/ut_select.h"

struct ut_socket_t {
    ut_socket_fd_t         fd;                 /* socket file descriptor */
    union {
        struct {

        } tcp;
        struct {
            ut_hash_t*      hh;
            ut_bool_t       registered;         /* has registered to select engine or not */
            ut_select_engine_t* engine;         /* 如果已注册，则记录注册时使用的select引擎 */
            ut_fd_t         pipe[2];            /* make UDP similar to TCP. If it's origin socket, use it for new connection,
                                                   if it's received socket, use it to transfer message */
            ut_socket_t*    belong_to;      /* current client socket belongs to which local socket */
        }udp;
    } diff;
    ut_bool_t           non_block;
    int32_t             max_num;
    ut_trans_mode_t     trans_mode;         /* socket transport mode */
    struct sockaddr_in  st_local_addr;      /* structure of local socket address */
    struct sockaddr_in  st_remote_addr;     /* structure of remote socket address */
    char                bind_if[IFNAMSIZ];  /* netcard name */
};


static uint32_t __udp_hash_func(const char *key);
static void __udp_reg_callback(ut_fd_t fd, void* context);
static void __udp_reg_callback2(ut_fd_t fd, void* context);


ut_errno_t ut_socket_create(ut_socket_t** out, in_addr_t local_addr, in_port_t local_port, ut_trans_mode_t mode, const char* bind_if)
{
    ut_socket_t*        new = NULL;
    struct ifreq        ifr = {0};
    ut_errno_t          retval = UT_ERRNO_OK;
    int32_t             tmpval = 0;

    CHECK_PTR_RET(out, retval, UT_ERRNO_NULLPTR);

    *out = ut_zero_alloc(sizeof(ut_socket_t));
    CHECK_PTR_RET(*out, retval, UT_ERRNO_OUTOFMEM);
    new = *out;

    new->fd = socket(AF_INET, mode, mode == UT_TRANS_UDP ? IPPROTO_UDP : IPPROTO_TCP);
    CHECK_VAL_EQ(new->fd < 0, UT_TRUE, retval = UT_ERRNO_UNKNOWN, TAG_ERR);

    new->trans_mode = mode;

    /* socket绑定网卡 */
    if (bind_if != NULL) {
        strncpy(new->bind_if, bind_if, IFNAMSIZ - 1);
        strncpy(ifr.ifr_ifrn.ifrn_name, bind_if, IFNAMSIZ - 1);
        if (setsockopt(new->fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0) {
            retval = UT_ERRNO_INVALID;
        }
    }

    if (setsockopt(new->fd, SOL_SOCKET, SO_REUSEADDR, &tmpval, sizeof(int32_t)) < 0) {
        retval = UT_ERRNO_RESOURCE;
    }

    /* socket绑定IP地址 */
    new->st_local_addr.sin_family = AF_INET;
    new->st_local_addr.sin_addr.s_addr = local_addr;
    new->st_local_addr.sin_port = local_port;
    UT_LOG_INFO("socket create, bind to %s:%hd\n", inet_ntoa(new->st_local_addr.sin_addr), ntohs(local_port));

    if (bind(new->fd, (struct sockaddr*)&new->st_local_addr, sizeof(struct sockaddr_in)) < 0) {
        retval = UT_ERRNO_UNKNOWN;
        goto TAG_ERR;
    }

    CHECK_VAL_NEQ(retval, UT_ERRNO_OK, NULL, TAG_ERR);

TAG_OUT:
    return retval;

TAG_ERR:
    free(new);
    *out = NULL;
    goto TAG_OUT;
}

ut_errno_t ut_socket_destroy(ut_socket_t* sock)
{
    ut_errno_t          retval = UT_ERRNO_OK;

    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);

    /* 如果是TCP模式，直接关闭fd */
    if (sock->trans_mode == UT_TRANS_TCP) {
        close(sock->fd);

    /* 如果是UDP模式 */
    } else if (sock->trans_mode == UT_TRANS_UDP) {
        /* 如果是create创建出来的socket，关闭 */
        if (sock->fd > 0) {
            close(sock->fd);
        }
        /* 如果是accept所创建出来的socket，在父socket的哈希表中删除自己 */
        if (sock->diff.udp.belong_to != NULL) {
            char addrbuffer[UT_LEN_128] = {0};
            snprintf(addrbuffer, UT_LEN_32 - 1, "%d:%hd", 
                     sock->st_remote_addr.sin_addr.s_addr, 
                     sock->st_remote_addr.sin_port);
            if (ut_hash_pop(sock->diff.udp.belong_to->diff.udp.hh, addrbuffer) == NULL) {
                retval = UT_ERRNO_UNKNOWN;
            }
        }
        /* 如果哈希表已创建，则销毁所有已连接的子socket并销毁哈希表 */
        if (sock->diff.udp.hh) {
            ut_hash_foreach(sock->diff.udp.hh, NULL, sock);   /* 销毁已连接的socket */
            ut_hash_destroy(sock->diff.udp.hh);  /* 销毁哈希表 */
        }
        /* 如果管道已创建，则销毁管道 */
        if (PIPE_RD_FD(sock->diff.udp.pipe) != 0) {
            close(PIPE_RD_FD(sock->diff.udp.pipe));
            close(PIPE_WR_FD(sock->diff.udp.pipe));
            PIPE_RD_FD(sock->diff.udp.pipe) = 0;
            PIPE_WR_FD(sock->diff.udp.pipe) = 0;
        }
        /* 如果已经注册到select引擎，取消注册。注册只会发生在accept和connect时 */
        if (sock->diff.udp.registered) {
            retval = ut_select_engine_fd_del(sock->diff.udp.engine, sock->fd);
        }
    }

    /* 释放内存 */
    free(sock);

TAG_OUT:
    return retval;
}

ut_errno_t ut_socket_connect(ut_socket_t* sock, ut_select_engine_t* engine, in_addr_t remote_addr, in_port_t remote_port)
{
    ut_errno_t      retval = UT_ERRNO_OK;

    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);

    sock->st_remote_addr.sin_family = AF_INET;
    sock->st_remote_addr.sin_addr.s_addr = remote_addr;
    sock->st_remote_addr.sin_port = remote_port;

    /* 如果是TCP模式，还需要进行三次握手 */
    if (sock->trans_mode == UT_TRANS_TCP) {
        if (connect(sock->fd, (struct sockaddr*)&sock->st_remote_addr, sizeof(struct sockaddr_in)) < 0) {
            retval = UT_ERRNO_UNKNOWN;
            goto TAG_OUT;
        }

    /* 如果是UDP模式 */
    } else if (sock->trans_mode == UT_TRANS_UDP) {
        /* 首次调用，还未注册到select engine */
        if (!sock->diff.udp.registered) {
            pipe(sock->diff.udp.pipe);   /* 这个管道用于接收数据时存放的位置 */

            /* 将socket注册到select engine，如果不是新的连接，是旧的数据，那么就会发往对应的管道 */
            retval = ut_select_engine_fd_add_forever(engine, sock->fd, __udp_reg_callback2, sock);
            CHECK_VAL_NEQ(retval, UT_ERRNO_OK, NULL, TAG_OUT);
            sock->diff.udp.registered = UT_TRUE;
            sock->diff.udp.engine = engine;
        }

        /* 则任意发送一段信息，激活服务端 */
        sendto(sock->fd, "hello", 5, 0, (struct sockaddr*)&sock->st_remote_addr, sizeof(struct sockaddr_in));
    }

TAG_OUT:
    return retval;
}

ut_errno_t ut_socket_set_max_accept(ut_socket_t* sock, int32_t num)
{
    ut_errno_t          retval = UT_ERRNO_OK;

    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);
    CHECK_VAL_EQ(num <= 0, UT_TRUE, retval = UT_ERRNO_INVALID, TAG_OUT);
    sock->max_num = num;
    if (sock->trans_mode == UT_TRANS_TCP) {
        listen(sock->fd, num*2);
    }

TAG_OUT:
    return retval;
}

ut_errno_t ut_socket_accept(ut_socket_t** out, ut_socket_t* sock, ut_select_engine_t* engine)
{
    ut_errno_t          retval = UT_ERRNO_OK;
    ut_socket_t*        accepted_sock = NULL;
    ut_socket_t         tmp_sock;
    struct sockaddr_in  tmp_addr;
    ut_fd_t             tmp_fd = 0;

    CHECK_PTR_RET(out, retval, UT_ERRNO_NULLPTR);
    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);
    CHECK_PTR_RET(engine, retval, UT_ERRNO_NULLPTR);

    switch (sock->trans_mode) {
        case UT_TRANS_TCP:
        {
            socklen_t   socklen;

            tmp_fd = accept(sock->fd, (struct sockaddr*)&tmp_addr, &socklen);
            UT_LOG_INFO("got a new connection on %s:%hd\n", 
                        inet_ntoa(sock->st_local_addr.sin_addr), 
                        ntohs(sock->st_local_addr.sin_port));
            CHECK_VAL_EQ(tmp_fd, -1, retval = UT_ERRNO_UNKNOWN, TAG_OUT);

            accepted_sock = ut_zero_alloc(sizeof(ut_socket_t));
            CHECK_PTR_RET(accepted_sock, retval, UT_ERRNO_OUTOFMEM);

            accepted_sock->fd = tmp_fd;
            accepted_sock->trans_mode = sock->trans_mode;
            memcpy(&accepted_sock->st_local_addr, &sock->st_local_addr, sizeof(struct sockaddr_in));
            memcpy(&accepted_sock->st_remote_addr, &tmp_addr, sizeof(struct sockaddr_in));

            break;
        }
        case UT_TRANS_UDP:
        {
            char        addrbuffer[UT_LEN_32] = {0};
            size_t      readlen = 0;

            /* 首次调用，还未注册到select engine */
            if (!sock->diff.udp.registered) {
                pipe(sock->diff.udp.pipe);   /* 这个管道用于新的连接时，会通过该管道传输过来 */
                ut_hash_create(&sock->diff.udp.hh, sock->max_num, __udp_hash_func);

                /* 将socket注册到select engine，如果不是新的连接，是旧的数据，那么就会发往对应的管道 */
                retval = ut_select_engine_fd_add_forever(engine, sock->fd, __udp_reg_callback, sock);
                CHECK_VAL_NEQ(retval, UT_ERRNO_OK, NULL, TAG_OUT);
                sock->diff.udp.registered = UT_TRUE;
                sock->diff.udp.engine = engine;
            }

            /* 读取管道，等待新的UDP连接 */
            readlen = read(PIPE_RD_FD(sock->diff.udp.pipe), &tmp_sock, sizeof(ut_socket_t));
            if (readlen != sizeof(ut_socket_t)) {
                retval = UT_ERRNO_UNKNOWN;
                goto TAG_OUT;
            }

            accepted_sock = ut_zero_alloc(sizeof(ut_socket_t));
            CHECK_PTR_RET(accepted_sock, retval, UT_ERRNO_OUTOFMEM);
            memcpy(accepted_sock, &tmp_sock, sizeof(ut_socket_t));

            snprintf(addrbuffer, UT_LEN_32 - 1, "%d:%hd", 
                                                accepted_sock->st_remote_addr.sin_addr.s_addr, 
                                                accepted_sock->st_remote_addr.sin_port);
            ut_hash_push(sock->diff.udp.hh, addrbuffer, accepted_sock);   /* 将新的UDP连接放进哈希表中 */
            break;
        }
        default:
            retval = UT_ERRNO_INVALID;
            goto TAG_OUT;
    }

    *out = accepted_sock;

TAG_OUT:
    return retval;
}

ut_fd_t ut_socket_read_fd_get(const ut_socket_t* sock)
{
    if (sock == NULL) {
        return -1;
    }

    switch (sock->trans_mode) {
        case UT_TRANS_TCP:
            return sock->fd;
        case UT_TRANS_UDP:
            if (PIPE_RD_FD(sock->diff.udp.pipe) > 0) {
                return PIPE_RD_FD(sock->diff.udp.pipe);
            } else {
                return sock->fd;
            }
        default:
            return -1;
    }
}

ut_errno_t ut_socket_msg_send(ut_socket_t* sock, const void* msg, size_t msg_size)
{
    ut_errno_t      retval = UT_ERRNO_OK;
    ssize_t         sendlen = 0;

    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);
    CHECK_PTR_RET(msg, retval, UT_ERRNO_NULLPTR);

    switch (sock->trans_mode) {
        case UT_TRANS_TCP:
            sendlen = send(sock->fd, msg, msg_size, 0);
            break;
        case UT_TRANS_UDP:
        {
            ut_fd_t     fd = 0;

            /* 通过accept接收到的远端socket连接，发送时使用使用衍生出这个socket的原生socket进行发送 */
            if (sock->fd == -1 && sock->diff.udp.belong_to != NULL) { 
                fd = sock->diff.udp.belong_to->fd;
            /* 正常通过create创建出的socket，直接使用fd来发送即可 */
            } else if (sock->fd > 0) {
                fd = sock->fd;
            } else {
                retval = UT_ERRNO_INVALID;
                break;
            }
            sendlen = sendto(fd, msg, msg_size, 0,
                            (struct sockaddr*)&sock->st_remote_addr, sizeof(struct sockaddr_in));
            break;
        }
        default:
            break;
    }
    if (sendlen != msg_size) {
        UT_LOG_DEBUG("msg send %ld bytes, but need to send %ld bytes, err=%s\n", sendlen, msg_size, strerror(errno));
        retval = UT_ERRNO_UNKNOWN;
    } else {
        UT_LOG_DEBUG("send %ld bytes data\n", sendlen);
    }

TAG_OUT:
    return retval;
}

ut_errno_t ut_socket_msg_recv(ut_socket_t* sock, void* msg, size_t msg_size, ssize_t* actual_size)
{
    ut_errno_t      retval = UT_ERRNO_OK;
    int32_t         block_flag = 0;

    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);
    CHECK_PTR_RET(msg, retval, UT_ERRNO_NULLPTR);
    CHECK_PTR_RET(actual_size, retval, UT_ERRNO_NULLPTR);
    if (sock->non_block) {
        block_flag = MSG_DONTWAIT;
    }

    switch (sock->trans_mode) {
        case UT_TRANS_TCP:
            *actual_size = recv(sock->fd, msg, msg_size, block_flag);
            break;
        case UT_TRANS_UDP:
            *actual_size = read(ut_socket_read_fd_get(sock), msg, msg_size);
            break;
        default:
            break;
    }
    if (*actual_size != msg_size) {
        UT_LOG_DEBUG("[%p::%d] actual read %ld bytes, need read %ld bytes\n", 
                     sock, ut_socket_read_fd_get(sock), *actual_size, msg_size);
        retval = UT_ERRNO_UNKNOWN;
    } else {
        UT_LOG_DEBUG("recv %ld bytes data.\n", *actual_size);
    }

TAG_OUT:
    return retval;
}

ut_errno_t ut_socket_set_block(ut_socket_t* sock, ut_bool_t block)
{
    ut_errno_t      retval = UT_ERRNO_OK;

    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);

    sock->non_block = !block;
    switch (sock->trans_mode) {
        case UT_TRANS_TCP:
            ut_fd_block(sock->fd, block);
            break;
        case UT_TRANS_UDP:
            if (PIPE_RD_FD(sock->diff.udp.pipe)) {
                ut_fd_block(PIPE_RD_FD(sock->diff.udp.pipe), block);
            }
            break;
        default:
            retval = UT_ERRNO_UNKNOWN;
    }

TAG_OUT:
    return retval;
}




static uint32_t __udp_hash_func(const char *key)
{
    in_addr_t   addr = 0;
    in_port_t   port = 0;

    sscanf(key, "%d:%hd", &addr, &port);
    return (addr << 16) | port;
}

static void __udp_reg_callback(ut_fd_t fd, void* context)
{
    ut_socket_t*        sock = (ut_socket_t*)context;
    ut_socket_t*        remote_sock = NULL;
    char                buffer[UT_LEN_1024 + 1] = {0};
    char                addrbuffer[UT_LEN_32] = {0};
    struct sockaddr_in  sockaddr;
    socklen_t           socklen = sizeof(struct sockaddr_in);
    ssize_t             recvlen = 0;

    /*  
        由于是是无连接的UDP，因此为了将不同远端的信息分发到对应的位置，使用recvfrom获取远端的信息，
        并判断该远端是否是曾经接受过信息的远端，如果是新的远端，那么创建一个无名管道，以后再次收到来自
        该远端的信息，就将信息直接存入该管道，给对应的处理线程去处理。通过这种方式，来将UDP、TCP两种
        传输模式统一起来。
     */
    recvlen = recvfrom(sock->fd, buffer, UT_LEN_1024, 0, (struct sockaddr*)&sockaddr, &socklen);
    if (recvlen <= 0) {
        UT_LOG_ERROR("Unknown error occurred when call recvfrom!\n");
        goto TAG_OUT;
    }

    snprintf(addrbuffer, UT_LEN_32 - 1, "%d:%hd", sockaddr.sin_addr.s_addr, sockaddr.sin_port);
    remote_sock = (ut_socket_t*)ut_hash_peek(sock->diff.udp.hh, addrbuffer);
    if (remote_sock != NULL) {      /* 如果消息不是第一次收到了，转发给消息管道 */
        write(PIPE_WR_FD(remote_sock->diff.udp.pipe), buffer, recvlen);
    } else {                        /* 消息第一次收到，构造新的远端结构体 */
        ut_socket_t     new_sock = {0};

        UT_LOG_DEBUG("new connection!\n");
        new_sock.trans_mode = UT_TRANS_UDP;
        new_sock.diff.udp.belong_to = sock;
        pipe(new_sock.diff.udp.pipe);       /* 创建消息管道 */
        new_sock.fd = -1;                   /* UDP通过管道来获取来自相同远端的信息 */
        memcpy(&new_sock.st_local_addr, &sock->st_local_addr, sizeof(struct sockaddr_in));
        memcpy(&new_sock.st_remote_addr, &sockaddr, sizeof(struct sockaddr_in));

        write(PIPE_WR_FD(new_sock.diff.udp.pipe), buffer, recvlen);      /* 保存新远端的信息到管道 */
        write(PIPE_WR_FD(sock->diff.udp.pipe), &new_sock, sizeof(ut_socket_t));   /* 通知accept的位置，有来自新的远端的数据 */
    }

TAG_OUT:
    return ;
}

static void __udp_reg_callback2(ut_fd_t fd, void* context)
{
    ut_socket_t*        sock = (ut_socket_t*)context;
    char                buffer[UT_LEN_1024 + 1] = {0};
    struct sockaddr_in  sockaddr;
    socklen_t           socklen = sizeof(struct sockaddr_in);
    ssize_t             recvlen = 0;

    /*  
        由于是是无连接的UDP，为了避免被其他的角色攻击导致收到异常信息，使用recvfrom获取远端的信息，
        并判断该远端是否是指定的remote addr，如果是不是，那么忽视它；如果是指定的远端地址，那么就将
        信息直接存入该管道，给对应的处理线程去处理。通过这种方式，来将UDP、TCP两种传输模式统一起来。
     */
    recvlen = recvfrom(sock->fd, buffer, UT_LEN_1024, 0, (struct sockaddr*)&sockaddr, &socklen);
    if (recvlen <= 0) {
        UT_LOG_ERROR("Unknown error occurred when call recvfrom!\n");
        goto TAG_OUT;
    }

    /* 如果消息来自指定的远端，则写入管道 */
    if (!memcmp(&sockaddr, &sock->st_remote_addr, sizeof(struct sockaddr_in))) {
        UT_LOG_DEBUG("received %ld bytes data, write to pipe %d\n", recvlen, PIPE_RD_FD(sock->diff.udp.pipe));
        write(PIPE_WR_FD(sock->diff.udp.pipe), buffer, recvlen);

    /* 丢弃数据 */
    } else {
        UT_LOG_INFO("discard %ld bytes message from \"%s:%hd\"\n", recvlen,
                    inet_ntoa(sock->st_local_addr.sin_addr), 
                    ntohs(sock->st_local_addr.sin_port));
    }

TAG_OUT:
    return ;
}
