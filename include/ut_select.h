/**
 * @file ut_select.h
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 一个基于select实现的事件引擎，通过select来实现
 *        对文件描述符fd、定时器事件event的处理。
 * @version 0.1
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __UTILS_SELECT_H__
#define __UTILS_SELECT_H__

#include "utils.h"

typedef struct ut_select_engine_t ut_select_engine_t;



__BEGIN_DECLS

/**
 * @brief 定时器事件的回调函数
 * 
 * @param [in] context 回调者的上下文
 */
typedef void (*ut_select_schedule_cb)(void* context);

/**
 * @brief 文件描述符可读监视的回调函数
 * 
 * @param [in] context 回调者的上下文
 */
typedef void (*ut_select_fd_cb)(ut_fd_t fd, void* context);

/**
 * @brief 创建一个基于select实现的事件引擎
 * 
 * @param [out] engine 传出参数，select事件引擎描述结构体
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_create(ut_select_engine_t **engine);

/**
 * @brief 销毁select事件引擎
 * 
 * @param [in] engine select事件引擎描述结构体
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_destroy(ut_select_engine_t* engine);

/**
 * @brief 闭合的死循环，通过select来实现对fd、定时器事件的处理，并调用回调。
 *        直到出现异常或调用select_stop后停止
 * 
 * @param [in] engine select事件引擎描述结构体
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_run(ut_select_engine_t* engine);

/**
 * @brief 停止select事件引擎的运行
 * 
 * @param [in] engine select事件引擎描述结构体
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_stop(ut_select_engine_t* engine);

/**
 * @brief 向select事件引擎中添加一个定时器事件
 * 
 * @param [in] engine select事件引擎描述结构体
 * @param [in] callback 定时器到期后，触发的回调函数
 * @param [in] context 传递给回调函数的上下文
 * @param [in] timeout_us 定时器时长，单位微秒us
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_schedule_add(ut_select_engine_t* engine, ut_select_schedule_cb callback, void* context, int64_t timeout_us);

/**
 * @brief 设置一个一次性的文件描述符监视，在该文件描述符可读一次之后，就删除
 * 
 * @param [in] engine select事件引擎描述结构体
 * @param [in] fd 文件描述符
 * @param [in] callback 当fd可读时，将会调用的回调函数
 * @param [in] context  传递给回调函数的上下文
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_fd_add_once(ut_select_engine_t* engine, ut_fd_t fd, ut_select_fd_cb callback, void* context);

/**
 * @brief 设置一个永久性的文件描述符监视
 * 
 * @param [in] engine select事件引擎描述结构体
 * @param [in] fd 文件描述符
 * @param [in] callback 当fd可读时，将会调用的回调函数
 * @param [in] context  传递给回调函数的上下文
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_fd_add_forever(ut_select_engine_t* engine, ut_fd_t fd, ut_select_fd_cb callback, void* context);

/**
 * @brief 将之前放入select事件引擎监视的文件描述符删除
 * 
 * @param [in] engine select事件引擎描述结构体
 * @param [in] fd 文件描述符
 * @return ut_errno_t 
 */
ut_errno_t ut_select_engine_fd_del(ut_select_engine_t* engine, ut_fd_t fd);

__END_DECLS
#endif