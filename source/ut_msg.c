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

#include <string.h>


ut_errno_t ut_msg_recv_by_socket(ut_msg_t** msg, ut_socket_t* sock)
{
    ut_errno_t      retval = UT_ERRNO_OK;
    ut_msg_t        msg_header = {0};
    char*           str_msg = NULL;
    ssize_t         readlen = 0;

    retval = ut_socket_msg_recv(sock, &msg_header, MSG_HEADER_SIZE, &readlen);
    CHECK_VAL_NEQ(retval, UT_ERRNO_OK, NULL, TAG_OUT);
    CHECK_VAL_NEQ(readlen, MSG_HEADER_SIZE, NULL, TAG_OUT);

    str_msg = ut_zero_alloc(MSG_HEADER_SIZE + msg_header.message_size + 1);
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

ut_errno_t ut_msg_send_by_socket(ut_msg_type_t type, void* msg_text, uint32_t text_size, ut_socket_t* sock)
{
    ut_msg_t*   msg = NULL;
    ut_errno_t  retval = UT_ERRNO_OK;

    CHECK_PTR_RET(msg_text, retval, UT_ERRNO_NULLPTR);
    CHECK_VAL_EQ(text_size, 0, retval = UT_ERRNO_INVALID, TAG_OUT);
    CHECK_VAL_EQ(type < UT_MSG_TYPE_AUTH_REQUEST || type > UT_MSG_TYPE_QUIT, UT_TRUE, retval = UT_ERRNO_INVALID, TAG_OUT);

    msg = ut_zero_alloc(MSG_HEADER_SIZE + text_size);
    msg->message_type = type;
    msg->message_size = text_size;
    memcpy(msg->message, msg_text, text_size);

    retval = ut_socket_msg_send(sock, msg, MSG_SIZE(msg));
    free(msg);

TAG_OUT:
    return retval;
}

ut_errno_t msg_read_from_socket_trywait(ut_msg_t** msg, ut_socket_t* sock)
{
    return UT_ERRNO_OK;
}