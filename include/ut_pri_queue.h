/**
 * @file ut_pri_queue.h
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 优先级队列
 * @version 1.0
 * @date 2022-04-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __UTILS_PRI_QUEUE_H__
#define __UTILS_PRI_QUEUE_H__

#include "utils.h"

/* 基本优先级队列类型 */
typedef struct pri_queue ut_pri_queue_t;

/* 比较输入两个保存的数据的优先级，true表示elem1的优先级更高，false反之 */
typedef ut_bool_t (*ut_pri_comp_func)(void* elem1, void* elem2);

typedef struct {
    ut_pri_comp_func    priority_compare;   /* 优先级比较的回调函数 */
} ut_pri_queue_cb_t;



__BEGIN_DECLS

/**
 * 创建一个优先级队列
 * @param [in] initial_size 初始大小
 * @param [in] size_adaption 是否允许队列大小自动扩容
 * @param [in] cb 需要注册的回调函数
 * @retval ut_pri_queue_t* 优先级队列指针
 */
ut_pri_queue_t *ut_pri_queue_create(int32_t initial_size, ut_bool_t size_adaption, ut_pri_comp_func cb);

/**
 * 销毁一个优先级队列
 * @param [in] pri_queue 优先级队列指针优先级队列指针
 * @retval errno_t UT_ERRNO_OK : 成功, 其他失败
 */
errno_t ut_pri_queue_destroy(ut_pri_queue_t *pri_queue);

/**
 * 查看优先级队列首部的数据
 * @param [in] pri_queue 优先级队列指针优先级队列指针
 * @param [in] pdata 保存数据指针的指针
 * @retval errno_t UT_ERRNO_OK : 成功, 其他失败
 */
errno_t ut_pri_queue_peek(ut_pri_queue_t * pri_queue, void** pdata);

/**
 * 将数据存入优先级队列
 * @param [in] pri_queue 优先级队列指针优先级队列指针
 * @param data 数据指针
 * @retval errno_t UT_ERRNO_OK : 成功, 其他失败
 */
errno_t ut_pri_queue_push(ut_pri_queue_t * pri_queue, void* data);

/**
 * 取出优先级队列的首个数据，如果不存在数据，阻塞等待
 * @param [in] pri_queue 优先级队列指针
 * @param [in] pdata 保存数据指针的指针
 * @retval errno_t UT_ERRNO_OK : 成功, 其他失败
 */
errno_t ut_pri_queue_pop_wait(ut_pri_queue_t *pri_queue, void* *pdata);

/**
 * 取出优先级队列的首个数据，如果不存在数据，直接返回
 * @param [in] pri_queue 优先级队列指针
 * @param [in] pdata 保存数据指针的指针
 * @retval errno_t UT_ERRNO_OK : 成功, 其他失败
 */
errno_t ut_pri_queue_pop_trywait(ut_pri_queue_t *pri_queue, void* *pdata);

/**
 * 取出优先级队列的首个数据，如果不存在数据，阻塞等待指定时间
 * @param [in] pri_queue 优先级队列指针
 * @param [in] pdata 保存数据指针的指针
 * @param [in] timeout 阻塞等待的时间
 * @retval errno_t UT_ERRNO_OK : 成功, 其他失败
 */
errno_t ut_pri_queue_pop_timedwait(ut_pri_queue_t *pri_queue, void* *pdata, int32_t timeout);

/**
 * 获取一个优先级队列内还有多少个数据
 * @param [in] pri_queue 优先级队列指针
 * @retval int32_t 优先级队列内的数据个数
 */
int32_t ut_pri_queue_get_size(const ut_pri_queue_t *pri_queue);

__END_DECLS
#endif

