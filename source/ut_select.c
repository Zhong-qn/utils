/**
 * @file select_engine.c
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include "utils.h"
#include "ut_hash.h"
#include "ut_pri_queue.h"
#include "ut_select.h"


#define PRI_QUEUE_SIZE      50


typedef enum {
    ENGINE_EVENT_RESTART,
    ENGINE_EVENT_STOP,
} engine_manage_event_t;

typedef struct {
    ut_select_event_cb  cb;
    void*               context;
    struct timeval      time;
} engine_event_t;

typedef struct {
    ut_fd_t             fd;
    ut_select_fd_cb     cb;
    ut_bool_t           temporary;
    void*               context;
} engine_fd_t;

struct ut_select_engine_t {
    ut_pri_queue_t      *event_queue;
    ut_hash_t           *fd_poll;
    fd_set              read_fds;
    ut_fd_t             max_fd;
    ut_bool_t           need_continue;
    ut_bool_t           has_reset;
    pthread_mutex_t     running_flag;   /* 是否在运行的标志位 */
    int32_t             manage_pipe[2];
};


static void __engine_destroy(ut_select_engine_t* engine);
static uint32_t __fd_poll_hash_func(const char *key);
static ut_bool_t __event_queue_pri_comp(void* event1, void* event2);
static ut_bool_t __fd_set_foreach(const char *key, const void* value, void* context);
static ut_bool_t __fd_isset_foreach(const char *key, const void* value, void* context);
static void __manage_fd_callback(ut_fd_t manage_fd, void* context);
static void __engine_reload(ut_select_engine_t* engine);
static ut_errno_t __engine_fd_add(ut_select_engine_t* engine, ut_fd_t fd, ut_select_fd_cb callback, void* context, ut_bool_t temporary);
ut_errno_t __engine_fd_del(ut_select_engine_t* engine, ut_fd_t fd);


ut_errno_t ut_select_engine_create(ut_select_engine_t **engine)
{
    ut_errno_t          retval = UT_ERRNO_OK;
    ut_select_engine_t  *new_engine = NULL;

    if (engine == NULL) {
        retval = UT_ERRNO_INVALID;
        goto _out;
    }

    new_engine = zero_alloc(sizeof(ut_select_engine_t));
    if (new_engine == NULL) {
        retval = UT_ERRNO_OUTOFMEM;
        goto _out;
    }
    memset(new_engine, 0, sizeof(ut_select_engine_t));


    /* 创建优先级队列作为事件队列 */
    new_engine->event_queue = ut_pri_queue_create(PRI_QUEUE_SIZE, UT_TRUE, __event_queue_pri_comp);
    if (new_engine->event_queue == NULL) {
        retval = UT_ERRNO_UNKNOWN;
        goto _destroy;
    }

    /* 创建哈希表作为fd池 */
    retval = ut_hash_create(&new_engine->fd_poll, 100, __fd_poll_hash_func);
    if (retval != UT_ERRNO_OK) {
        goto _destroy;
    }

    new_engine->need_continue = UT_TRUE;
    pthread_mutex_init(&new_engine->running_flag, NULL);

    pipe(new_engine->manage_pipe);
    __engine_fd_add(new_engine, PIPE_RD_FD(new_engine->manage_pipe), __manage_fd_callback, new_engine, UT_FALSE);

    *engine = new_engine;
_out:
    return retval;

_destroy:
    __engine_destroy(new_engine);
    goto _out;
}

ut_errno_t ut_select_engine_fd_add_forever(ut_select_engine_t* engine, ut_fd_t fd, ut_select_fd_cb callback, void* context)
{
    ut_errno_t              retval = UT_ERRNO_OK;

    if (engine == NULL || callback == NULL || fd < 0) {
        retval = UT_ERRNO_INVALID;
        goto _out;
    }
    CR_LOG_DEBUG("select engine add a fd=%d\n", fd);
    retval = __engine_fd_add(engine, fd, callback, context, UT_FALSE);
    __engine_reload(engine);    /* 通知engine重新进行select */

_out:
    return retval;
}

ut_errno_t ut_select_engine_fd_add_once(ut_select_engine_t* engine, ut_fd_t fd, ut_select_fd_cb callback, void* context)
{
    ut_errno_t              retval = UT_ERRNO_OK;

    if (engine == NULL || callback == NULL || fd < 0) {
        retval = UT_ERRNO_INVALID;
        goto _out;
    }
    retval = __engine_fd_add(engine, fd, callback, context, UT_TRUE);
    __engine_reload(engine);    /* 通知engine重新进行select */

_out:
    return retval;
}

ut_errno_t ut_select_engine_fd_del(ut_select_engine_t* engine, ut_fd_t fd)
{
    ut_errno_t              retval = UT_ERRNO_OK;

    if (engine == NULL || fd < 0) {
        retval = UT_ERRNO_INVALID;
        goto _out;
    }

    __engine_fd_del(engine, fd);
    __engine_reload(engine);    /* 通知engine重新进行select */

_out:
    return retval;
}

ut_errno_t ut_select_engine_event_add(ut_select_engine_t* engine, ut_select_event_cb callback, void* context, int64_t timeout_us)
{
    ut_errno_t      retval = UT_ERRNO_OK;
    engine_event_t  *event = NULL;

    if (engine == NULL || callback == NULL || timeout_us < 0) {
        retval = UT_ERRNO_INVALID;
        goto _out;
    }

    event = zero_alloc(sizeof(engine_event_t));
    event->cb = callback;
    event->context = context;
    gettimeofday(&event->time, NULL);
    CR_LOG_DEBUG("current time:  %lds:%ldus\n", event->time.tv_sec, event->time.tv_usec);

    event->time.tv_sec += timeout_us / (1000 * 1000);
    event->time.tv_usec += timeout_us % (1000 * 1000);
    if (event->time.tv_usec < 0) {
        event->time.tv_usec += 1000 * 1000;
        event->time.tv_sec -= 1;
    }
    if (event->time.tv_sec < 0) {
        event->time.tv_sec = 0;
        event->time.tv_usec = 0;
    }

    CR_LOG_DEBUG("set timeout event %lds:%ldus\n", event->time.tv_sec, event->time.tv_usec);;
    ut_pri_queue_push(engine->event_queue, event);
    __engine_reload(engine);    /* 通知engine重新进行select */

_out:
    return retval;
}

ut_errno_t ut_select_engine_run(ut_select_engine_t* engine)
{
    struct timeval  tm_wait;
    struct timeval* select_tm = NULL;
    engine_event_t  *event = NULL;
    ut_errno_t      retval = UT_ERRNO_OK;
    int32_t         select_ret = 0;

    if (engine == NULL) {
        retval = UT_ERRNO_INVALID;
        goto _out;
    }

    while (engine->need_continue) {
        /*
            上锁的目的只是为了标志此时select引擎正在运行。此时任何调整select引擎的动作都
            必须进行引擎的reload，以保证该调整会立即生效
         */
        pthread_mutex_lock(&engine->running_flag);

        /* 初始化需要监听的文件描述符 */
        FD_ZERO(&engine->read_fds);
        ut_hash_foreach(engine->fd_poll, __fd_set_foreach, engine);

        /* 获取等待的时间 */
        retval = ut_pri_queue_peek(engine->event_queue, (void**)&event);
        if (retval == UT_ERRNO_RESOURCE) {   /* 说明此时没有事件需要处理 */
            select_tm = NULL;
            CR_LOG_DEBUG("no need to wait.\n");
        } else if (retval != UT_ERRNO_OK) {
            retval = UT_ERRNO_UNKNOWN;       /* 出错了 */
            CR_LOG_ERROR("error occured!\n");
            goto _out;
        } else {                            /* 说明此时有事件需要处理 */
            gettimeofday(&tm_wait, NULL);
            CR_LOG_DEBUG("current time:  %lds:%ldus\n", tm_wait.tv_sec, tm_wait.tv_usec);
            CR_LOG_DEBUG("event time:  %lds:%ldus\n", event->time.tv_sec, event->time.tv_usec);
            tm_wait.tv_sec = event->time.tv_sec - tm_wait.tv_sec;
            tm_wait.tv_usec = event->time.tv_usec - tm_wait.tv_usec;
            if (tm_wait.tv_usec < 0) {
                tm_wait.tv_usec += 1000 * 1000;
                tm_wait.tv_sec -= 1;
            }
            if (tm_wait.tv_sec < 0) {
                tm_wait.tv_sec = 0;
                tm_wait.tv_usec = 0;
            }
            CR_LOG_DEBUG("waiting for timeout...%lds:%ldus\n", tm_wait.tv_sec, tm_wait.tv_usec);
            select_tm = &tm_wait;
        }

        select_ret = select(engine->max_fd + 1, &engine->read_fds, NULL, NULL, select_tm);
        CR_LOG_DEBUG("select_ret=%d\n", select_ret);

        /* 解锁，此后将会执行回调函数。此时调整select引擎，则不需要进行reload */
        pthread_mutex_unlock(&engine->running_flag);

        /* 定时器事件处理 */
        if (!select_ret) {  
            event->cb(event->context);
            retval = ut_pri_queue_pop_trywait(engine->event_queue, (void**)&event);
            if (retval != UT_ERRNO_OK) {
                break;
            }
        /* fd可读 */
        } else if (select_ret > 0) {
            ut_hash_foreach(engine->fd_poll, __fd_isset_foreach, engine);
        /* 被中断程序打断 */
        } else {
            CR_LOG_INFO("select has been interrupted by system call.(%s)\n", strerror(errno));
        }
    }

    engine->need_continue = UT_TRUE;
_out:
    return retval;
}

ut_errno_t ut_select_engine_stop(ut_select_engine_t* engine)
{
    ut_errno_t              retval = UT_ERRNO_OK;
    engine_manage_event_t   event = ENGINE_EVENT_STOP;

    if (engine == NULL) {
        retval = UT_ERRNO_NULLPTR;
        goto _out;
    }

    write(PIPE_WR_FD(engine->manage_pipe), &event, sizeof(engine_manage_event_t));

_out:
    return retval;
}

ut_errno_t ut_select_engine_destroy(ut_select_engine_t* engine)
{
    ut_errno_t              retval = UT_ERRNO_OK;

    if (engine == NULL) {
        retval = UT_ERRNO_NULLPTR;
        goto _out;
    }

    if (engine->need_continue) {
        retval = UT_ERRNO_INVALID;
        goto _out;
    }

    __engine_destroy(engine);

_out:
    return retval;
}




static void __engine_reload(ut_select_engine_t* engine)
{
    engine_manage_event_t   mgt_event = ENGINE_EVENT_RESTART;

    if (!pthread_mutex_trylock(&engine->running_flag)) {
        /* 如果能拿到锁，这说明此时程序不在运行，解锁 */
        pthread_mutex_unlock(&engine->running_flag);
    } else {
        /* 如果拿不到锁，这说明此时程序正在运行，通知engine重新进行select */
        write(PIPE_WR_FD(engine->manage_pipe), &mgt_event, sizeof(engine_manage_event_t));
    }

    return ;
}

static ut_errno_t __engine_fd_add(ut_select_engine_t* engine, ut_fd_t fd, ut_select_fd_cb callback, void* context, ut_bool_t temporary)
{
    ut_errno_t      retval = UT_ERRNO_OK;
    char            buffer[32] = {0};
    engine_fd_t*    engine_fd = NULL;

    engine_fd = zero_alloc(sizeof(engine_fd_t));
    if (engine_fd == NULL) {
        retval = UT_ERRNO_OUTOFMEM;
        goto _out;
    }

    engine_fd->fd = fd;
    engine_fd->cb = callback;
    engine_fd->temporary = temporary;
    engine_fd->context = context;

    snprintf(buffer, 31, "%d", fd);
    ut_hash_push(engine->fd_poll, buffer, engine_fd);

_out:
    return retval;
}

ut_errno_t __engine_fd_del(ut_select_engine_t* engine, ut_fd_t fd)
{
    ut_errno_t              retval = UT_ERRNO_OK;
    char                    buffer[UT_LEN_32] = {0};

    snprintf(buffer, UT_LEN_32 - 1, "%d", fd);
    if (ut_hash_pop(engine->fd_poll, buffer) == NULL) {
        retval = UT_ERRNO_NOTEXSIT;
    }

    return retval;
}

static inline void __engine_destroy(ut_select_engine_t* engine)
{
    if (engine != NULL) {
        if (engine->event_queue != NULL) {
            ut_pri_queue_destroy(engine->event_queue);
            engine->event_queue = NULL;
        }
        if (engine->fd_poll != NULL) {
            ut_hash_destroy(engine->fd_poll);
        }
        pthread_mutex_destroy(&engine->running_flag);
        free(engine);
    }
}

static ut_bool_t __event_queue_pri_comp(void* event1, void* event2)
{
    int64_t    tmp_s = 0;
    int64_t    tmp_us = 0;

    tmp_s = PTR_CAST(event1, engine_event_t*)->time.tv_sec - PTR_CAST(event2, engine_event_t*)->time.tv_sec;
    tmp_us = PTR_CAST(event1, engine_event_t*)->time.tv_usec - PTR_CAST(event2, engine_event_t*)->time.tv_usec;

    if ((tmp_s < 0) || (!tmp_s && tmp_us <= 0)) {
        return UT_TRUE;
    } else {
        return UT_FALSE;
    }
}

static uint32_t __fd_poll_hash_func(const char *key)
{
    return PTR_CAST(atoi(key), uint32_t);
}

static ut_bool_t __fd_set_foreach(const char *key, const void* value, void* context)
{
    ut_bool_t               retval = UT_TRUE;
    ut_select_engine_t*     engine = (ut_select_engine_t*)context;
    engine_fd_t*            engine_fd = (engine_fd_t*)value;

    if (context == NULL || value == NULL) {
        retval = UT_FALSE;
        goto _out;
    }

    engine->max_fd = max(engine->max_fd, engine_fd->fd);
    FD_SET(engine_fd->fd, &engine->read_fds);

_out:
    return retval;
}

static void __manage_fd_callback(ut_fd_t manage_fd, void* context)
{
    engine_manage_event_t       event;
    ut_select_engine_t*         engine = (ut_select_engine_t*)context;

    while (fd_readable(manage_fd)) {
        read(manage_fd, &event, sizeof(engine_manage_event_t));
        CR_LOG_DEBUG("process manage message %d\n", event);
        switch (event) {
            case ENGINE_EVENT_STOP:
                engine->need_continue = UT_FALSE;
                break;
            case ENGINE_EVENT_RESTART:
            default:
                break;
        }
    }
    return ;
}

static ut_bool_t __fd_isset_foreach(const char *key, const void* value, void* context)
{
    ut_bool_t               retval = UT_TRUE;
    ut_select_engine_t*     engine = (ut_select_engine_t*)context;
    engine_fd_t*            engine_fd = (engine_fd_t*)value;

    if (context == NULL || value == NULL) {
        retval = UT_FALSE;
        goto _out;
    }

    if (FD_ISSET(engine_fd->fd, &engine->read_fds)) {
        if (engine_fd->temporary) {             /* 如果fd是只执行一次的，则从哈希表中移除 */
            char            buffer[32] = {0};

            snprintf(buffer, 31, "%d", engine_fd->fd);
            ut_hash_pop(engine->fd_poll, buffer);
        }

        engine_fd->cb(engine_fd->fd, engine_fd->context);   /* 如果fd可读，则执行回调 */
    }

_out:
    return retval;
}
