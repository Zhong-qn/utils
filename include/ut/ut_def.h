/**
 * @file ut_def.h
 * @author zhongqiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-21
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef __UT_DEF_H__
#define __UT_DEF_H__

#define ASCII_CODE_NUL      0x00U
#define ASCII_CODE_SOH      0x01U
#define ASCII_CODE_STX      0x02U
#define ASCII_CODE_ETX      0x03U
#define ASCII_CODE_EOT      0x04U
#define ASCII_CODE_ENQ      0x05U
#define ASCII_CODE_ACK      0x06U
#define ASCII_CODE_BEL      0x07U
#define ASCII_CODE_BS       0x08U
#define ASCII_CODE_HT       0x09U
#define ASCII_CODE_LF       0x0aU
#define ASCII_CODE_VT       0x0bU
#define ASCII_CODE_FF       0x0cU
#define ASCII_CODE_CR       0x0dU
#define ASCII_CODE_SO       0x0eU
#define ASCII_CODE_SI       0x0fU
#define ASCII_CODE_DEL      0x7fU
#define ASCII_CODE_DC1      0x11U
#define ASCII_CODE_SUB      0x1aU
#define ASCII_CODE_ESC      0x1bU
#define ASCII_CODE_SP       0x20U
#define ASCII_CODE_ESC2     0xe0U
#define ASCII_CODE_MAX      0xffU

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

#define UT_LOG(format, ...)
#define UT_LOG_DEBUG(format, ...)
#define UT_LOG_INFO(format, ...)
#define UT_LOG_ERROR(format, ...)

#ifdef ENABLE_LOG
#if (LOG_LEVEL >= 3)
#undef UT_LOG
#define UT_LOG(format, ...)             printf(ESC_STR(COLOR_CH_BLACK,  "[%25s :%5d] "      format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#if (LOG_LEVEL >= 2)
#undef UT_LOG_DEBUG
#define UT_LOG_DEBUG(format, ...)       printf(ESC_STR(COLOR_CH_GREEN,  "[%25s :%5d] [D] "  format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#if (LOG_LEVEL >= 1)
#undef UT_LOG_INFO
#define UT_LOG_INFO(format, ...)        printf(ESC_STR(COLOR_CH_BLUE,   "[%25s :%5d] [I] "  format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#if (LOG_LEVEL >= 0)
#undef UT_LOG_ERROR
#define UT_LOG_ERROR(format, ...)       printf(ESC_STR(COLOR_CH_RED,    "[%25s :%5d] [E] "  format), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#define FUNC_IN()                       UT_LOG_DEBUG("%s in\n", __FUNCTION__)
#define FUNC_OUT()                      UT_LOG_DEBUG("%s out\n", __FUNCTION__)

#endif