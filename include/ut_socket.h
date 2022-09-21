/**
 * @file ut_socket.h
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __CR_SOCK_H__
#define __CR_SOCK_H__

#include <netinet/in.h>
#include <sys/select.h>
#include "ut.h"
#include "ut_select.h"

typedef struct ut_socket_t ut_socket_t;

typedef enum {
    UT_TRANS_TCP = SOCK_STREAM,
    UT_TRANS_UDP = SOCK_DGRAM,
    UT_TRANS_HYBRID,              /* 混合传输模式，暂不支持 */
} ut_trans_mode_t;



__BEGIN_DECLS

/**
 * @brief 获取socket的读取fd
 * 
 * @param [in] sock socket结构体
 * @return ut_socket_fd_t 
 */
ut_socket_fd_t ut_socket_read_fd_get(const ut_socket_t* sock);

/**
 * @brief 创建一个socket对象
 * 
 * @param [inout] out 传出创建的socket结构体
 * @param [in] local_addr socket绑定的IPv4地址
 * @param [in] local_port socket绑定的端口
 * @param [in] mode socket的传输模式，TCP/UDP
 * @param [in] bind_if socket绑定的网卡，如果传入NULL则不绑定网卡
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_create(ut_socket_t** out, in_addr_t local_addr, in_port_t local_port, ut_trans_mode_t mode, const char* bind_if);

/**
 * @brief 销毁一个socket对象
 * 
 * @param [in] sock socket对象
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_destroy(ut_socket_t* sock);

/**
 * @brief 连接到远端
 * 
 * @param [in] sock socket对象
 * @param [in] engine select引擎
 * @param [in] remote_addr 要连接到的远端IPv4地址
 * @param [in] remote_port 要连接到的远端端口
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_connect(ut_socket_t* sock, ut_select_engine_t* engine, in_addr_t remote_addr, in_port_t remote_port);

/**
 * @brief 接收从远端来的连接
 * 
 * @param [in] out 传出新连接的远端对象
 * @param [in] sock socket对象
 * @param [in] engine select引擎
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_accept(ut_socket_t** out, ut_socket_t* sock, ut_select_engine_t* engine);

/**
 * @brief 设置socket的最大链接数量
 * 
 * @param [in] sock socket对象
 * @param [in] num 最大链接数量
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_set_max_accept(ut_socket_t* sock, int32_t num);

/**
 * @brief 通过socket对象发送一段信息
 * 
 * @param [in] sock socket对象
 * @param [in] msg 信息指针
 * @param [in] msg_size 待发送的信息长度
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_msg_send(ut_socket_t* sock, const void* msg, size_t msg_size);

/**
 * @brief 通过socket对象接收一段信息
 * 
 * @param [in] sock socket对象
 * @param [in] msg 信息指针
 * @param [in] msg_size 待接收的信息长度
 * @param [out] actual_size 实际的接收长度
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_msg_recv(ut_socket_t* sock, void* msg, size_t msg_size, ssize_t* actual_size);

/**
 * @brief 设置socket是否阻塞
 * 
 * @param [in] sock socket对象
 * @param [in] block 是否阻塞
 * @return ut_errno_t 
 */
ut_errno_t ut_socket_set_block(ut_socket_t* sock, ut_bool_t block);


/**
 * @brief 功能同FD_SET
 * 
 * @param [in] sock socket结构体
 * @param [inout] fds fd_set结构体
 * @return ut_errno_t 
 */
static inline ut_errno_t ut_socket_fd_set(ut_socket_t* sock, fd_set* fds)
{
    ut_errno_t     retval = UT_ERRNO_OK;
    ut_fd_t     fd = ut_socket_read_fd_get(sock);

    CHECK_PTR_RET(sock, retval, UT_ERRNO_NULLPTR);
    CHECK_PTR_RET(fds, retval, UT_ERRNO_NULLPTR);
    FD_SET(fd, fds);

TAG_OUT:
    return retval;
}

/**
 * @brief 功能同FD_ISSET
 * 
 * @param [in] sock socket结构体
 * @param [inout] fds fd_set结构体
 * @return ut_bool_t 
 */
static inline ut_bool_t ut_socket_fd_isset(ut_socket_t* sock, fd_set* fds)
{
    ut_bool_t      retval = UT_FALSE;
    ut_fd_t     fd = ut_socket_read_fd_get(sock);

    CHECK_PTR(sock);
    CHECK_PTR(fds);
    retval = !!(FD_ISSET(fd, fds));

TAG_OUT:
    return retval;
}

__END_DECLS
#endif