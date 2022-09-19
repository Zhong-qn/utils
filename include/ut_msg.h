/**
 * @file ut_msg.h
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __UTILS_MSG_H__
#define __UTILS_MSG_H__

#include "utils.h"
#include "ut_socket.h"


#define USR_MGMT_SENDER_NAME        "CR_USR_MGMT"
#define USR_MGMT_DEST_NAME          "UNKNOWN_USER"
#define USR_AUTH_EXTRA_SIZE         11
#define USR_AUTH_FORMAT             "usr=%s pwd=%s"
#define USR_INFO_FORMAT             "%s %s"

#define MAX_MSG_SIZE                1024U
#define MSG_HEADER_SIZE             sizeof(ut_msg_t)
#define MAX_MSG_BODY_SIZE           (MAX_MSG_SIZE - MSG_HEADER_SIZE)
#define MSG_SENDER_FORMAT           "[src=\"%s\",dst=\"%s\"]"                  /* 格式为[发送者::目的地] */
#define MSG_SENDER_EXTRA_SIZE       15
#define MSG_SIZE(msg)               (MSG_HEADER_SIZE + (msg)->message_size)
#define MSG_CONTAINER(ptr)          PTR_CAST(ptr, ut_msg_t*)

#define MSG_DISPLAY_SELF_FORMAT(format)     ESC_STR2(COLOR_BG_BLUE, COLOR_CH_WHITE, format)
#define MSG_DISPLAY_OTHERS_FORMAT(format)   ESC_STR2(COLOR_BG_GREEN, COLOR_CH_WHITE, format)


typedef enum {
    UT_MSG_TYPE_AUTH_REQUEST,   /* 鉴权请求 */
    UT_MSG_TYPE_AUTH_ANSWER,    /* 鉴权响应 */
    UT_MSG_TYPE_REMOTE_LOGIN,   /* 异地登录通知 */
    UT_MSG_TYPE_MESSAGE,        /* 发送聊天信息 */
    UT_MSG_TYPE_KEEPALIVE,      /* 保活信息 */
    UT_MSG_TYPE_QUIT,           /* 客户端退出 */
} ut_msg_type_t;

typedef struct ut_msg_t {
    ut_msg_type_t   message_type;   /* 消息类型 */
    uint32_t        message_size;   /* 消息体大小 */
    char            message[0];     /* 消息体 */
} ut_msg_t;



__BEGIN_DECLS

/**
 * @brief 从socket对象中读取一段消息，注意！！！msg必须被free！
 * 
 * @param [inout] msg 读取到的数据内容注意！！！msg必须被free！
 * @param [in] sock 发送使用的socket结构体
 * @return ut_errno_t 
 */
ut_errno_t ut_msg_recv_by_socket(ut_msg_t** msg, ut_socket_t* sock);

/**
 * @brief 通过socket对象发送一段消息
 * 
 * @param [in] type 待发送的消息的类型
 * @param [in] msg_text 待发送的消息的正文
 * @param [in] text_size 待发送的消息的正文大小
 * @param [in] from_sock 发送使用的socket结构体
 * @return ut_errno_t 
 */
ut_errno_t ut_msg_send_by_socket(ut_msg_type_t type, void* msg_text, uint32_t text_size, ut_socket_t* sock);

__END_DECLS
#endif