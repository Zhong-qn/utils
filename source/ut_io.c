/**
 * @file utils.c
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ut.h"



/*****************************************************/
/**********************内存申请************************/
/*****************************************************/

void* ut_zero_alloc(size_t size)
{
    return calloc(1, size);
}


/*****************************************************/
/*******************文件处理函数************************/
/*****************************************************/

int32_t ut_fd_readable(ut_fd_t fd)
{
    int32_t read_len = 0;

    /* 获取输入缓冲区大小 */
    ioctl(fd, FIONREAD, &read_len);

    return read_len;
}

int32_t ut_fd_fflush(ut_fd_t fd)
{
    int32_t     cur_len = 0;
    int32_t     read_len = 0;
    char        buf[MAX_RECV_BUF_LEN] = {0};

    /* 获取输入缓冲区内容 */
    cur_len = ut_fd_readable(fd);
    read_len += cur_len;

    /* 如果缓冲区中还有内容，则清空。 */
    while (cur_len != 0) {
        if (read(fd, &buf, MAX_RECV_BUF_LEN) <= 0) {
            break;
        }
        cur_len = ut_fd_readable(fd);
        read_len += cur_len;
    }

    return read_len;
}

void ut_fd_block(ut_fd_t fd, ut_bool_t block)
{
    int32_t     flag = 0;

    flag = fcntl(fd, F_GETFL);
    if (block) {
        flag &= ~O_NONBLOCK;
    } else {
        flag |= O_NONBLOCK;
    }

    return ;
}

int32_t ut_terminal_get(struct winsize* window_size)
{
    return (ioctl(STDIN_FILENO, TIOCGWINSZ, window_size) == -1) ? UT_ERRNO_UNKNOWN : UT_ERRNO_OK;
}