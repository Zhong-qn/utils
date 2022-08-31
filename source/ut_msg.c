/**
 * @file msg.c
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "ut_msg.h"


errno_t ut_msg_recv_by_socket(ut_msg_t** msg, ut_socket_t* sock)
{
    errno_t      retval = UT_ERRNO_OK;
    ut_msg_t        msg_header = {0};
    char*           str_msg = NULL;
    ssize_t         readlen = 0;

    retval = ut_socket_msg_recv(sock, &msg_header, MSG_HEADER_SIZE, &readlen);
    CHECK_VAL_NEQ(retval, UT_ERRNO_OK, NULL, TAG_OUT);
    CHECK_VAL_NEQ(readlen, MSG_HEADER_SIZE, NULL, TAG_OUT);

    str_msg = zero_alloc(MSG_HEADER_SIZE + msg_header.message_size + 1);
    CHECK_PTR_RET(str_msg, retval, UT_ERRNO_OUTOFMEM);
    str_msg += MSG_HEADER_SIZE;

    if (msg_header.message_size > 0) {
        retval = ut_socket_msg_recv(sock, str_msg, msg_header.message_size, &readlen);
        CHECK_VAL_NEQ(retval, UT_ERRNO_OK, free(str_msg - MSG_HEADER_SIZE), TAG_OUT);
        CHECK_VAL_NEQ(readlen, msg_header.message_size, free(str_msg - MSG_HEADER_SIZE), TAG_OUT);
    }
    memcpy(str_msg - MSG_HEADER_SIZE, &msg_header, MSG_HEADER_SIZE);
    *msg = (ut_msg_t*)(str_msg - MSG_HEADER_SIZE);

TAG_OUT:
    return retval;
}

errno_t ut_msg_send_by_socket(ut_msg_t* msg, ut_socket_t* sock)
{
    return ut_socket_msg_send(sock, msg, MSG_SIZE(msg));
}

errno_t msg_read_from_socket_trywait(ut_msg_t** msg, ut_socket_t* sock)
{
    return UT_ERRNO_OK;
}