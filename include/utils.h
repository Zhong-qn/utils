/**
 * @file utils.h
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief tools
 * @version 1.0
 * @date 2021-07-11
 * 
 * @copyright chatroom Copyright (C) 2022 QiaoningZhong
 * 
 */
#ifndef __CR_UTILS_H__
#define __CR_UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/ioctl.h>


#define ASCII_CODE_NUL      0x0000
#define ASCII_CODE_SOH      0x0001
#define ASCII_CODE_STX      0x0002
#define ASCII_CODE_ETX      0x0003
#define ASCII_CODE_EOT      0x0004
#define ASCII_CODE_ENQ      0x0005
#define ASCII_CODE_ACK      0x0006
#define ASCII_CODE_BEL      0x0007
#define ASCII_CODE_BS       0x0008
#define ASCII_CODE_HT       0x0009
#define ASCII_CODE_LF       0x000a
#define ASCII_CODE_VT       0x000b
#define ASCII_CODE_FF       0x000c
#define ASCII_CODE_CR       0x000d
#define ASCII_CODE_SO       0x000e
#define ASCII_CODE_SI       0x000f
#define ASCII_CODE_DEL      0x007f
#define ASCII_CODE_DC1      0x0011
#define ASCII_CODE_SUB      0x001a
#define ASCII_CODE_ESC      0x001b
#define ASCII_CODE_SP       0x0020
#define ASCII_CODE_ESC2     0x00e0
#define ASCII_CODE_MAX      0x00ff

#define UT_LEN_16           16
#define UT_LEN_32           32
#define UT_LEN_64           64
#define UT_LEN_128          128
#define UT_LEN_256          256
#define UT_LEN_512          512
#define UT_LEN_1024         1024

#define MAX_CMD_MATCH_NUM               128
#define CMD_MAX_STR_LEN                 512
#define MAX_RECV_BUF_LEN                255
#define PIPE_BUF_SIZE                   64

#define MAX_RESPONSE_LEN                10240

#define MAX_TERMINAL_HEIGHT             22
#define MAX_TERMINAL_WIDTH              78
#define DEFAULT_TERMINAL_WIDTH          80
#define DEFAULT_TERMINAL_HEIGHT         24

#define PIPE_RD_FD(pipe_arr)            (pipe_arr[0])
#define PIPE_WR_FD(pipe_arr)            (pipe_arr[1])
#define FOREVER                         while(1)

#ifndef max
#define max(x, y)   (((x) < (y)) ? (y) : (x))
#endif
#ifndef min
#define min(x, y)   (((x) < (y)) ? (x) : (y))
#endif
#ifndef to_upper
#define to_upper(x)  ((('a' <= (x)) && ('z' >= (x))) ? ((x) - 'a' + 'A') : (x))
#endif

#define list_entry(ptr, type, member) \
            ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define container_of(container_ptr, container_type, container_member) \
            list_entry(container_ptr, container_type, container_member)

#define CHECK_VAL_EQ(val1, val2, cmd, tag)  do {\
        if ((val1) == (val2)) {\
            cmd;\
            goto tag;\
        }\
    } while (0)

#define CHECK_VAL_NEQ(val1, val2, cmd, tag)  do {\
        if ((val1) != (val2)) {\
            cmd;\
            goto tag;\
        }\
    } while (0)

#define CHECK_PTR(ptr)                  CHECK_VAL_EQ((void*)ptr, NULL, NULL, TAG_OUT)
#define CHECK_PTR_RET(ptr, ret, val)    CHECK_VAL_EQ((void*)ptr, NULL, ret = val, TAG_OUT)
#define CHECK_PTR_TAG(ptr, tag)         CHECK_VAL_EQ((void*)ptr, NULL, NULL, tag)

#define CHECK_FREE(ptr) \
    do {\
        if (ptr != NULL) {\
            free(ptr);\
            ptr = NULL;\
        }\
    } while (0)

#define PTR_CAST(val, to_type)   ((to_type)(uintptr_t)(val))

#define NO_WARN(command)    do {\
        if (command);\
    } while(0)

#define ESC_BEGIN                       "\033["
#define ESC_END                         "\033[0m"
#define ESC_STR(attr, str)              ESC_BEGIN attr "m" str ESC_END
#define ESC_STR2(attr1, attr2, str)     ESC_BEGIN attr1 ";" attr2 "m" str ESC_END
#define COLOR_BG_BLACK                  "40"
#define COLOR_BG_WHITE                  "47"
#define COLOR_BG_READ                   "41"
#define COLOR_BG_GREEN                  "42"
#define COLOR_BG_YELLOW                 "43"
#define COLOR_BG_BLUE                   "44"
#define COLOR_BG_PINK                   "45"
#define COLOR_CH_BLACK                  "30"
#define COLOR_CH_WHITE                  "37"
#define COLOR_CH_RED                    "31"
#define COLOR_CH_GREEN                  "32"
#define COLOR_CH_YELLOW                 "33"
#define COLOR_CH_BLUE                   "34"
#define COLOR_CH_PINK                   "35"

#define CR_LOG(format, ...)
#define CR_LOG_DEBUG(format, ...)
#define CR_LOG_INFO(format, ...)
#define CR_LOG_ERROR(format, ...)

#ifdef ENABLE_LOG
#if (LOG_LEVEL >= 3)
#undef CR_LOG
#define CR_LOG(format, ...)             printf(ESC_STR(COLOR_CH_BLACK,  "[%25s :%5d] "      format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#if (LOG_LEVEL >= 2)
#undef CR_LOG_DEBUG
#define CR_LOG_DEBUG(format, ...)       printf(ESC_STR(COLOR_CH_GREEN,  "[%25s :%5d] [D] "  format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#if (LOG_LEVEL >= 1)
#undef CR_LOG_INFO
#define CR_LOG_INFO(format, ...)        printf(ESC_STR(COLOR_CH_BLUE,   "[%25s :%5d] [I] "  format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#if (LOG_LEVEL >= 0)
#undef CR_LOG_ERROR
#define CR_LOG_ERROR(format, ...)       printf(ESC_STR(COLOR_CH_RED,    "[%25s :%5d] [E] "  format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#define FUNC_IN()                       CR_LOG_DEBUG("%s in\n", __FUNCTION__)
#define FUNC_OUT()                      CR_LOG_DEBUG("%s out\n", __FUNCTION__)


/**
 * @brief 对system函数的封装，进行了system返回值的判定封装
 * 
 * @param [in] command 输入参数，类型为const char*，表示system函数的传入参数
 * @retval 返回0成功，返回-1失败
 */
#define SYSTEM(command)    ({\
        int retval = -1;\
        int status = system(command);\
        if (status == -1) {\
            CR_LOG("system error\n");\
        } else if (!WIFEXITED(status)) {\
            CR_LOG("system call shell failed\n");\
        } else if (WEXITSTATUS(status)) {\
            CR_LOG("system run shell return failed\n");\
        } else {\
            retval = 0;\
        }\
        retval;\
    })


typedef enum boolean_type_e {
    UT_FALSE = 0,
#define UT_FALSE UT_FALSE
    UT_TRUE = 1
#define UT_TRUE UT_TRUE
} ut_bool_t;

typedef enum ut_standard_errno_e {
    UT_ERRNO_NOTEXSIT = -6,   /* 所请求的资源不存在 */
    UT_ERRNO_RESOURCE = -5,   /* 资源不足，如队列满、数组满等 */
    UT_ERRNO_OUTOFMEM = -4,   /* 内存不足 */
    UT_ERRNO_INVALID = -3,    /* 非法的参数或操作 */
    UT_ERRNO_NULLPTR = -2,    /* 空指针错误 */
    UT_ERRNO_UNKNOWN = -1,    /* 未知的错误 */
    UT_ERRNO_OK = 0,          /* 无错误发生 */
} ut_errno_t;

typedef signed int ut_fd_t;
typedef signed int ut_socket_fd_t;
typedef unsigned int ut_virtual_key_t;


__BEGIN_DECLS

/**
 * @brief zero malloc，申请一片全为0的空间。
 * 
 * @param [in] size 申请的空间大小
 * @retval 返回申请到的空间
 */
void* zero_alloc(size_t size);


/*****************************************************/
/********************字符类型判断***********************/
/*****************************************************/

ut_bool_t char_is_ctl(char ch);
ut_bool_t char_is_num(char ch);
ut_bool_t char_is_hex(char ch);
ut_bool_t char_is_symbol(char ch);
ut_bool_t char_is_letter(char ch);
ut_bool_t char_is_expand(char ch);
ut_bool_t char_is_separator(char ch);
ut_bool_t char_is_capital(char ch);
ut_bool_t char_is_lowercase(char ch);

/**
 * @brief 若字符是大写字母，将该字母变成对应的小写字母
 * 
 * @param [in] ch 进行判断的字符
 * @retval 转换的结果字符
 */
char char_lower(char ch);

/**
 * @brief 若字符是小写字母，将该字母变成对应的大写字母
 * 
 * @param [in] ch 进行判断的字符
 * @retval 转换的结果字符
 */
char char_upper(char ch);


/******************************************************/
/********************字符串类型判断**********************/
/*****************************************************/

ut_bool_t str_is_num(const char* str);
int32_t str_is_ipv4_addr(const char * str);
int32_t str_is_ipv6_addr(const char * str);
int32_t str_is_macaddr(const char * str);
int32_t str_is_hex(const char * str);




/******************************************************/
/********************字符串处理函数**********************/
/*****************************************************/

/**
 * @brief 将src复制到dest中,返回复制的长度
 * 
 * @param [inout] dest 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] ch 停止的字符
 * @retval 返回复制的长度
 */
size_t strcpy_size(char* dest, const char* src);

/**
 * @brief 将src中的size个字符复制到dest里,返回复制的长度
 * 
 * @param [inout] dest 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] ch 停止的字符
 * @retval 返回复制的长度
 */
size_t strncpy_size(char* dest, const char* src, size_t size);

/**
 * @brief 将src复制到dest中，直到遇到字符ch或已经到达src字符串的末尾停止。
 * 
 * @param [inout] dest 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] ch 停止的字符
 * @retval 返回复制的长度
 */
size_t strcpy_until_char(char* dest, const char* src, char ch);

/**
 * @brief 将src复制到dest中，直到遇到字符ch或已经到达src字符串的末尾停止。若ch字符前
 *        有\\字符，则会忽视该字符
 * 
 * @param [inout] dest 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] ch 停止的字符
 * @retval 返回复制的长度
 */
size_t strcpy_until_char_specific(char* dest, const char* src, char ch);

/**
 * @brief 从源字符串的指定位置往后获取一段输入，遇到分隔符停止
 * 
 * @param [in] save_buf 获取输入的存放位置
 * @param [in] analysis_str 获取一段输入的源字符串
 * @param [inout] pos 当前已获取到的位置
 * @retval 获取完成后还有没有可获取的内容，有返回true，没有返回false
 */
ut_bool_t strcpy_delimited_by_space(char* save_buf, const char* analysis_str, uint32_t* pos);

/**
 * @brief 将src复制到dest中，直到到达end或者已经到达src字符串的末尾为止
 * 
 * @param [inout] dest 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] end 停止的位置指针
 * @retval 返回复制的长度
 */
size_t strcpy_until_ptr(char* dest, const char* src, const char* end);

/**
 * @brief 计算src字符串的长度，遇到ch停止
 * 
 * @param [in] src 计算长度的字符串
 * @param [in] ch 停止的字符
 * @retval src到第一次遇到ch字符时的长度
 */
size_t strlen_until_char(const char* src, const char ch);

/**
 * @brief 计算src字符串的长度，遇到ch停止,若ch字符出现前有\\符号则跳过该字符
 * 
 * @param [in] src 计算长度的字符串
 * @param [in] ch 停止的字符
 * @retval src到第一次遇到ch字符时的长度
 */
size_t strlen_until_char_specific(const char* src, const char ch);

/**
 * @brief 在src字符串中查找ch字符第一次出现的位置，若ch字符出现前有\\符号则跳过该字符
 * 
 * @param [in] src 待查找的字符串
 * @param [in] ch 待查找的字符
 * @retval 成功返回ch第count次出现的位置，失败返回NULL
 */
char* strchr_specific(const char* src, char ch);

/**
 * @brief 在src字符串内查找第count个出现的字符ch
 * 
 * @param [in] src 待查找的字符串
 * @param [in] ch 待查找的字符
 * @param [in] count 需要查找字符出现的次数
 * @retval 成功返回ch第count次出现的位置，失败返回NULL
 */
char* strchr_count(const char* src, char ch, int count);

/**
 * @brief 在src字符串中查找ch字符第一次出现的位置，若ch字符出现在()、{}、[]内，则跳过
 * 
 * @param [in] src 待查找的字符串
 * @param [in] ch 待查找的字符
 * @retval 成功返回ch第一次出现的位置，失败返回NULL
 */
char* strchr_case_brackets(const char* src, char ch);

/**
 * @brief 查找字符串str中字符find_char出现的次数，若字符出现前有\\符号则跳过该字符
 * 
 * @param [in] str 待查找的字符串
 * @param [in] find_char 查找的字符
 * @retval 字符出现的次数
 */
size_t str_find_specific(const char* str, const char find_char);

/**
 * @brief 查看文件可读的长度
 * 
 * @param [in] fd 文件描述符
 * @retval int32_t 可读的长度
 */
int32_t fd_readable(ut_fd_t fd);

/**
 * @brief 清空fd内的所有数据
 * 
 * @param [in] fd 文件描述符
 * @retval int32_t 清除的长度
 */
int32_t fd_fflush(ut_fd_t fd);

/**
 * @brief 设置fd的阻塞属性
 * 
 * @param [in] fd 文件描述符
 * @param [in] block 是否阻塞，true阻塞false非阻塞
 */
void fd_block(ut_fd_t fd, ut_bool_t block);

/**
 * @brief 获取当前终端属性
 * 
 * @param [out] window_size 传出终端属性
 * @return int32_t 
 */
int32_t terminal_get(struct winsize* window_size);

__END_DECLS
#endif 
