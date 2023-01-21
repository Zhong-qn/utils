/**
 * @file ut.h
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
#include <stdint.h>
#include <sys/ioctl.h>

#include "ut/ut_def.h"

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
            UT_LOG("system error\n");\
        } else if (!WIFEXITED(status)) {\
            UT_LOG("system call shell failed\n");\
        } else if (WEXITSTATUS(status)) {\
            UT_LOG("system run shell return failed\n");\
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
void* ut_zero_alloc(size_t size);


/*****************************************************/
/********************字符类型判断***********************/
/*****************************************************/

/**
 * @brief 判断字符是ASCII控制字符
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_ctl(char ch);

/**
 * @brief 判断字符是数字
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_num(char ch);

/**
 * @brief 判断字符是十六进制数字
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_hex(char ch);

/**
 * @brief 判断字符是符号
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_symbol(char ch);

/**
 * @brief 判断字符是字母
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_letter(char ch);

/**
 * @brief 判断字符是ASCII扩展字符
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_expand(char ch);

/**
 * @brief 判断字符是空格或制表符
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_separator(char ch);

/**
 * @brief 判断字符是大写字母
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_capital(char ch);

/**
 * @brief 判断字符是小写字母
 * 
 * @param [in] ch 待判断的字符
 * @return 真或假
 */
ut_bool_t ut_char_is_lowercase(char ch);




/**
 * @brief 将字母转换成小写
 * 
 * @param [in] ch 进行判断的字符
 * @retval 转换的结果字符
 */
char ut_char_lower(char ch);

/**
 * @brief 将字母转换成大写
 * 
 * @param [in] ch 进行判断的字符
 * @retval 转换的结果字符
 */
char ut_char_upper(char ch);


/******************************************************/
/********************字符串类型判断**********************/
/*****************************************************/

/**
 * @brief 判断字符串是不是数字，可判断负数和小数
 * 
 * @param [in] str 待判断的字符串
 * @return 真或假
 */
ut_bool_t ut_str_is_num(const char* str);

/**
 * @brief 判断字符串是不是十六进制整数
 * 
 * @param [in] str 待判断的字符串
 * @return 返回0表示是十六进制数，非0值表示从第几位开始不是十六进制数
 */
int32_t ut_str_is_hex(const char * str);

/**
 * @brief 判断字符串是不是IPv4地址
 * 
 * @param [in] str 待判断的字符串
 * @return 返回0表示是IPv4地址，非0值表示从第几位开始不是IPv4地址
 */
int32_t ut_str_is_ipv4addr(const char * str);

/**
 * @brief 判断字符串是不是IPv6地址
 * 
 * @param [in] str 待判断的字符串
 * @return 返回0表示是IPv6地址，非0值表示从第几位开始不是IPv6地址
 */
int32_t ut_str_is_ipv6addr(const char * str);

/**
 * @brief 判断字符串是不是MAC地址
 * 
 * @param [in] str 待判断的字符串
 * @return 返回0表示是MAC地址，非0值表示从第几位开始不是MAC地址
 */
int32_t ut_str_is_macaddr(const char * str);




/******************************************************/
/********************字符串处理函数**********************/
/*****************************************************/

/**
 * @brief 查找字符串str中字符ch出现的次数
 * 
 * @param [in] str 待查找的字符串
 * @param [in] ch 查找的字符
 * @retval 字符出现的次数
 */
size_t ut_strcnt(const char* str, char ch);

/**
 * @brief 查找字符串str中字符ch出现的次数，若字符出现前有'\'符号则跳过该字符
 * 
 * @param [in] str 待查找的字符串
 * @param [in] ch 查找的字符
 * @retval 字符出现的次数
 */
size_t ut_strcnt_sp(const char* str, char ch);




/**
 * @brief 将src复制到dst中
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @retval 返回复制的长度
 */
size_t ut_strcpy(char* dst, const char* src);

/**
 * @brief 将src复制到dst中，直到遇到字符ch或已经到达src字符串的末尾停止。
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] ch 停止的字符
 * @retval 返回复制的长度
 */
size_t ut_strcpy_char(char* dst, const char* src, char ch);

/**
 * @brief 将src复制到dst中，直到遇到字符ch或已经到达src字符串的末尾停止。若ch字符前
 *        有'\'字符，则会忽视该字符
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] ch 停止的字符
 * @retval 返回复制的长度
 */
size_t ut_strcpy_char_sp(char* dst, const char* src, char ch);

/**
 * @brief 将src复制到dst中，直到到达end或者已经到达src字符串的末尾为止
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] end 停止的位置指针
 * @retval 返回复制的长度
 */
size_t ut_strcpy_ptr(char* dst, const char* src, const char* end);

/**
 * @brief 将src中的size个字符复制到dst里
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] size 需要复制的长度
 * @retval 返回实际复制的长度
 */
size_t ut_strncpy(char* dst, const char* src, size_t size);

/**
 * @brief 将src中的size个字符复制到dst里，直到遇到字符ch或已经到达src字符串的末尾
 *        时停止。
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] size 需要复制的长度
 * @param [in] ch 停止的字符
 * @return 返回实际复制的长度
 */
size_t ut_strncpy_char(char* dst, const char* src, size_t size, char ch);

/**
 * @brief 将src中的size个字符复制到dst里，直到遇到字符ch或已经到达src字符串的末尾时
 *        停止。若ch字符前有'\'字符，则会忽视该字符
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] size 需要复制的长度
 * @param [in] ch 停止的字符
 * @retval 返回实际复制的长度
 */
size_t ut_strncpy_char_sp(char* dst, const char* src, size_t size, char ch);

/**
 * @brief 将src中的size个字符复制到dst里，直到到达end或者已经到达src字符串的末尾为止
 * 
 * @param [inout] dst 复制到的目的地址
 * @param [in] src 源字符串位置
 * @param [in] size 需要复制的长度
 * @param [in] end 停止的位置指针
 * @retval 返回实际复制的长度
 */
size_t ut_strncpy_ptr(char* dst, const char* src, size_t size, const char* end);




/**
 * @brief 计算src字符串的长度，遇到ch停止
 * 
 * @param [in] src 计算长度的字符串
 * @param [in] ch 停止的字符
 * @retval src到第一次遇到ch字符时的长度
 */
size_t ut_strlen_char(const char* src, char ch);

/**
 * @brief 计算src字符串的长度，遇到ch停止,若ch字符出现前有'\'符号则跳过该字符
 * 
 * @param [in] src 计算长度的字符串
 * @param [in] ch 停止的字符
 * @retval src到第一次遇到ch字符时的长度
 */
size_t ut_strlen_char_sp(const char* src, char ch);




/**
 * @brief 在src字符串中查找ch字符第一次出现的位置，若ch字符出现前有'\'符号则跳过该字符
 * 
 * @param [in] src 待查找的字符串
 * @param [in] ch 待查找的字符
 * @retval 成功返回ch第1次出现的位置，失败返回NULL
 */
char* ut_strchr_sp(const char* src, char ch);

/**
 * @brief 在src字符串内查找第seq个出现的字符ch
 * 
 * @param [in] src 待查找的字符串
 * @param [in] ch 待查找的字符
 * @param [in] seq 需要查找字符出现的次数
 * @retval 成功返回ch第seq次出现的位置，失败返回NULL
 */
char* ut_strchr_seq(const char* src, char ch, int seq);

/**
 * @brief 在src字符串中查找ch字符第一次出现的位置，跳过出现在()、{}、[]内的ch字符
 * 
 * @param [in] src 待查找的字符串
 * @param [in] ch 待查找的字符
 * @retval 成功返回ch第一次出现的位置，失败返回NULL
 */
char* ut_strchr_brackets(const char* src, char ch);




/**
 * @brief 查看文件可读的长度
 * 
 * @param [in] fd 文件描述符
 * @retval 可读的长度
 */
int32_t ut_fd_readable(ut_fd_t fd);

/**
 * @brief 清空fd内的所有数据
 * 
 * @param [in] fd 文件描述符
 * @retval 清除的长度
 */
int32_t ut_fd_fflush(ut_fd_t fd);

/**
 * @brief 设置fd的阻塞属性
 * 
 * @param [in] fd 文件描述符
 * @param [in] block 是否阻塞，true阻塞false非阻塞
 */
void ut_fd_block(ut_fd_t fd, ut_bool_t block);

/**
 * @brief 获取当前终端属性
 * 
 * @param [out] window_size 传出终端属性
 * @return 
 */
int32_t ut_terminal_get(struct winsize* window_size);

__END_DECLS
#endif 
