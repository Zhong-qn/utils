/**
 * @file ut_string.c
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string.h>
#include "ut/ut.h"




/*****************************************************/
/**************char型字符类型判断***********************/
/*****************************************************/

ut_bool_t ut_char_is_ctl(char ch)
{
    if ((ch <= 0x1f) || (ch==0x7f)) {
        return UT_TRUE;
    }
    
    return UT_FALSE;
}

ut_bool_t ut_char_is_num(char ch)
{
    if (ch >= 0x30 && ch <= 0x39) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

ut_bool_t ut_char_is_hex(char ch)
{
    if ((ch >= '0') && (ch <= '9')) {
        return UT_TRUE;
    }
    if ((ch >= 'a') && (ch <= 'f')) {
        return UT_TRUE;
    }
    if ((ch >= 'A') && (ch <= 'F')) {
        return UT_TRUE;
    }
    return UT_FALSE;
}

ut_bool_t ut_char_is_symbol(char ch)
{
    if ((ch >= 0x20 && ch <= 0x2f) 
         || (ch >= 0x3a && ch <= 0x40) 
         || (ch >= 0x5b && ch <= 0x60) 
         || (ch >= 0x7b && ch <= 0x7e)) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

ut_bool_t ut_char_is_letter(char ch)
{
    if ((ch >= 0x41 && ch <= 0x5a) 
         || (ch >= 0x61 && ch <= 0x7a)) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

ut_bool_t ut_char_is_capital(char ch)
{
    if (ch >= 0x41 && ch <= 0x5a) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

ut_bool_t ut_char_is_lowercase(char ch)
{
    if (ch >= 0x61 && ch <= 0x7a) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

ut_bool_t ut_char_is_expand(char ch)
{
    if (((u_int8_t)ch) >= 0x80 && ((u_int8_t)ch) <= 0xff) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

ut_bool_t ut_char_is_separator(char ch)
{
    if (ch == ASCII_CODE_SP) {
        return UT_TRUE;
    }
    if (ch == ASCII_CODE_HT) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

char ut_char_lower(char ch)
{
    return (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : (ch);
}

char ut_char_upper(char ch)
{
    return (ch >= 'a' && ch <= 'z') ? (ch + 'A' - 'a') : (ch);
}




/*****************************************************/
/*****************字符串处理函数************************/
/*****************************************************/

ut_bool_t ut_str_is_num(const char* str)
{
    int32_t pos = 0;
    int32_t point_flag = 0;

    if (*str == '-' || *str == '+') {
        pos++;
    }

    while (*(str+pos) != ASCII_CODE_NUL) {
        if (!ut_char_is_num(*(str+pos))) {
            if (!point_flag && *(str+pos) == '.') {
                point_flag = 1;
            } else {
                return UT_FALSE;
            }
        }
        pos++;
    }

    return UT_TRUE;
}

int32_t ut_str_is_hex(const char * str)
{
    char    ch;
    int32_t count = 0;

    ch = *(str+count);
    while (ch != ASCII_CODE_NUL && !ut_char_is_separator(ch)) {
        if ((ch <= '9' && ch >= '0')
                || (ut_char_lower(ch) >= 'a' && ut_char_lower(ch) <= 'f')) {
            ;
        } else {
            return count+1;
        }
        count++;
        ch = *(str+count);
    }

    return 0;
}

int32_t ut_str_is_ipv4addr(const char * str)
{
    int32_t count = 0;
    char ip_addr[4]= {0, 0, 0, 0};
    int32_t num_count = 0;
    int32_t dot_count = 0;
    char ch = ASCII_CODE_NUL;
    int32_t temp = 0;

    while (ut_char_is_separator(*(str+count++)));
    count--;

    ch = *(str+count);
    if (ch == ASCII_CODE_NUL) {
        return count+1;
    }

    while (!ut_char_is_separator(ch) && ch) {
        if (ch >= '0' && ch <= '9') {
            if (num_count>=3) {
                return count+1;
            } else {
                ip_addr[num_count++]=ch;
            }
        } else {
            if (dot_count >= 3) {
                return count+1;
            }
            if (ch=='.') {
                if (!num_count) {
                    return count+1;
                } else {
                    temp=atoi(ip_addr);
                    if (temp>255) {
                        return count;
                    } else {
                        dot_count++;
                        memset(ip_addr,0x00,4);
                        num_count=0;
                    }
                }
            } else {
                return count+1;
            }
        }
        count++;
        ch=*(str+count);
    }
    if (dot_count<3) {
        return count+1;
    } else {
        if (!num_count) {
            return count+1;
        } else {
            temp=atoi(ip_addr);
            if (temp>255) {
                return count;
            } else {
                dot_count++;
                memset(ip_addr,0x00,4);
                num_count=0;
            }
        }
    }
    while (ch) {
        if (!ut_char_is_separator(ch)) {
            return count+1;
        }
        count++;
        ch=*(str+count);
    }
    return 0;
}

int32_t ut_str_is_ipv6addr(const char *str)
{
    int     cnt = 0;
    char    ch;
    int     pos = 0;
    int     num = 0;
    int     colon_num = 0;
    int     flag = 0; //标记::
    int     ipv4_flag = 0;

    while (ut_char_is_separator(*(str+cnt))) {
        cnt++;
    }

    ch=*(str+cnt);
    if (ch == 0x00) {
        goto TAG_EXIT;
    }

    while(ch) {
        if (ut_char_is_separator(ch)) {
            break;
        }

        if ((ch <= '9' && ch >= '0') || (ut_char_lower(ch) >= 'a' && ut_char_lower(ch) <= 'f')) {
            if (num == 4) {
                goto TAG_EXIT;
            }
            num++;
            
        } else if (ch == ':') {
            num = 0;
            if (cnt - pos == 1) {
                if (flag) {
                    goto TAG_EXIT;  //存在两个::
                } else {
                    flag = 1;
                }
                colon_num--;
            } else {
                colon_num++;
            }

            pos = cnt;
        } else if (ch == '.') { //ipv6内嵌ipv4
            ipv4_flag = 1;
            break;
        } else {
            goto TAG_EXIT;
        }

        cnt++;
        ch = *(str + cnt);
    }

    if (!flag) { //ipv6使用冒分16进制表示,未缩写
        if (colon_num != 7) {
            goto TAG_EXIT;
        }
    } else { //ipv6采用0位压缩表示
        if (colon_num > 5) {
            goto TAG_EXIT;
        }
    }

    //ipv6内嵌ipv4地址
    if (ipv4_flag == 1) {
        if (ut_str_is_ipv4addr(str + pos + 1)) {
            goto TAG_EXIT;
        }
    }

    return 0;

TAG_EXIT:
    return -1;
}

int32_t ut_str_is_macaddr(const char * str)
{

    int32_t count;
    int32_t num_count;
    int32_t dot_count;
    char    ch;

    count=0;
    num_count=0;
    dot_count=0;
    while (ut_char_is_separator(*(str+count++))) {
        ;
    }
    count--;
    ch=*(str+count);
    if (ch==0x00) {
        return count+1;
    }
    while (!ut_char_is_separator(ch) && ch) {
        if (ut_char_is_hex(ch)) {
            if (num_count>=2) {
                return count+1;
            } else {
                num_count++;
            }
        } else {
            if (dot_count>=5) {
                return count+1;
            }
            if (ch==':') {

                if (num_count!=2) {
                    return count+1;
                }
                if (!num_count) {
                    return count+1;
                } else {
                    dot_count++;
                    num_count=0;
                }
            } else {
                return count+1;
            }
        }
        count++;
        ch=*(str+count);
    }
    if (dot_count<5) {
        return count+1;
    } else {
        if (num_count!=2) {
            return count+1;
        }

        if (!num_count) {
            return count+1;
        } else {
            dot_count++;
            num_count=0;
        }
    }

    while (ch) {
        if (!ut_char_is_separator(ch)) {
            return count+1;
        }
        count++;
        ch=*(str+count);
    }
    return 0;
}




size_t ut_strcnt(const char* str, char ch)
{
    size_t num = 0;
    
    while (*str != ASCII_CODE_NUL) {
        if (*str == ch) {
            num++;
        }
        str++;
    }
    return num;
}

size_t ut_strcnt_sp(const char* str, char ch)
{
    size_t num = 0;
    
    while(*str != ASCII_CODE_NUL) {
        if (*str == '\\') {
            str++;
            if (*str == ASCII_CODE_NUL) {
                break;
            }
        } else if (*str == ch) {
            num++;
        }
        str++;
    }
    return num;
}




size_t ut_strcpy(char* dst, const char* src)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while (*src != ASCII_CODE_NUL) {
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}

size_t ut_strcpy_char(char* dst, const char* src, char ch)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while ((*src != ASCII_CODE_NUL) && (*src != ch)) {
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}

size_t ut_strcpy_char_sp(char* dst, const char* src, char ch)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while (*src != ASCII_CODE_NUL) {
        if (*src == '\\') {
            *(dst++) = *(src++);
            copied_len++;
            if (*src == ASCII_CODE_NUL) {
                break;
            }
        } else if (*src == ch) {
            break;
        }
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}

size_t ut_strcpy_ptr(char* dst, const char* src, const char* end)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while (src != end && *src != ASCII_CODE_NUL) {
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}

size_t ut_strncpy(char* dst, const char* src, size_t size)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while (*src != ASCII_CODE_NUL) {
        if (copied_len >= size) {
            break;
        }
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}

size_t ut_strncpy_char(char* dst, const char* src, size_t size, char ch)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while ((*src != ASCII_CODE_NUL) && (*src != ch)) {
        if (copied_len >= size) {
            break;
        }
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}

size_t ut_strncpy_char_sp(char* dst, const char* src, size_t size, char ch)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while (*src != ASCII_CODE_NUL) {
        if (copied_len >= size) {
            break;
        }
        if (*src == '\\') {
            *(dst++) = *(src++);
            copied_len++;
            if (*src == ASCII_CODE_NUL || copied_len >= size) {
                break;
            }
        } else if (*src == ch) {
            break;
        }
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}

size_t ut_strncpy_ptr(char* dst, const char* src, size_t size, const char* end)
{
    size_t copied_len = 0;

    if (NULL == dst || NULL == src) {
        return 0;
    }

    while (src != end && *src != ASCII_CODE_NUL) {
        if (copied_len >= size) {
            break;
        }
        *(dst++) = *(src++);
        copied_len++;
    }

    return copied_len;
}





size_t ut_strlen_char(const char* src, char ch)
{
    size_t size = 0;
    const char* pos = src;

    while (((*pos) != ASCII_CODE_NUL) && ((*pos) != ch)) {
        size++;
        pos++;
    }

    return size;
}

size_t ut_strlen_char_sp(const char* src, char ch)
{
    size_t size = 0;

    while (*src != ASCII_CODE_NUL) {
        if (*src == '\\') {
            size++;
            src++;
        } else if (*src == ch) {
            break;
        }
        size++;
        src++;
    }

    return size;
}




char* ut_strchr_sp(const char* src, char ch)
{
    char* ret = NULL;

    CHECK_PTR(src);

    while (*src != ASCII_CODE_NUL) {
        if (*src == '\\') {
            src++;
        } else if (*src == ch) {
            ret = (char*)(uintptr_t)src;
            break;
        }
        src++;
    }

TAG_OUT:
    return ret;
}

char* ut_strchr_seq(const char* src, char ch, int seq)
{
    char* ret = NULL;

    CHECK_PTR(src);

    while (*src != ASCII_CODE_NUL) {
        if (*src == ch) {
            if (!(seq--)) {
                ret = (char*)src;
                break;
            }
        }

        src++;
    }

TAG_OUT:
    return ret;
}

char* ut_strchr_brackets(const char* src, char ch)
{
    int pos = 0;
    int32_t little_flag = 0;
    int32_t middle_flag = 0;
    int32_t large_flag = 0;

    CHECK_PTR(src);

    while (*(src+pos) != ASCII_CODE_NUL) {
        switch (*(src+pos)) {
            case '(':
                little_flag++;
                break;

            case ')':
                little_flag = little_flag > 0 ? little_flag-1 : 0;
                break;

            case '[':
                middle_flag++;
                break;
            
            case ']':
                middle_flag = middle_flag > 0 ? middle_flag-1 : 0;
                break;

            case '{':
                large_flag++;
                break;

            case '}':
                large_flag = large_flag > 0 ? large_flag-1 : 0;
                break;
            
            default:
                if ((*(src+pos) == ch) && !little_flag && !middle_flag && !large_flag) {
                    return (char*)(src+pos);
                }
                break;
                
        }/*switch*/
        pos++;
    }/*while*/

TAG_OUT:
    return NULL;
}

