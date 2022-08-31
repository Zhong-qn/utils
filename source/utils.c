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
#include "utils.h"



/*****************************************************/
/**********************内存申请************************/
/*****************************************************/

void* zero_alloc(size_t size)
{
    return calloc(1, size);
}


/*****************************************************/
/**************char型字符类型判断***********************/
/*****************************************************/

/*字符是ASCII码控制字符
是则返回1，否则返回0*/
ut_bool_t char_is_ctl(char ch)
{
    if ((ch<=0x1f) || (ch==0x7f)) {
        return UT_TRUE;
    }
    
    return UT_FALSE;
}

/*字符是ASCII码数字
是则返回1，否则返回0*/
ut_bool_t char_is_num(char ch)
{
    if (ch>=0x30 && ch<=0x39) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

/*字符是16进制数
是则返回1，否则返回0*/
ut_bool_t char_is_hex(char ch)
{
    if ((ch>='0') && (ch<='9')) {
        return UT_TRUE;
    }
    if ((ch>='a') && (ch<='f')) {
        return UT_TRUE;
    }
    if ((ch>='A') && (ch<='F')) {
        return UT_TRUE;
    }
    return UT_FALSE;
}

/*字符是ASCII码符号
是则返回1，否则返回0*/
ut_bool_t char_is_symbol(char ch)
{
    if ((ch>=0x20 && ch<=0x2f) 
         || (ch>=0x3a && ch<=0x40) 
         || (ch>=0x5b && ch<=0x60) 
         || (ch>=0x7b && ch<=0x7e)) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

/*字符是ASCII码字母
是则返回1，否则返回0*/
ut_bool_t char_is_letter(char ch)
{
    if ((ch>=0x41 && ch<=0x5a) 
         || (ch>=0x61 && ch<=0x7a)) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

/*字符是大写字母
是则返回1，否则返回0*/
ut_bool_t char_is_capital(char ch)
{
    if (ch >=0x41 && ch <=0x5a) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

/*字符是小写字母
是则返回1，否则返回0*/
ut_bool_t char_is_lowercase(char ch)
{
    if (ch >=0x61 && ch <=0x7a) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

/*字符是ASCII码拓展
是则返回1，否则返回0*/
ut_bool_t char_is_expand(char ch)
{
    if (ch>=0x80) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

/*字符是空格或tab分隔符
是则返回1，否则返回0*/
ut_bool_t char_is_separator(char ch)
{
    if (ch == ASCII_CODE_SP) {
        return UT_TRUE;
    }
    if (ch == ASCII_CODE_HT) {
        return UT_TRUE;
    }

    return UT_FALSE;
}

char char_lower(char ch)
{
    return (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : (ch);
}

char char_upper(char ch)
{
    return (ch >= 'a' && ch <= 'z') ? (ch + 'A' - 'a') : (ch);
}

/*****************************************************/
/*****************字符串处理函数************************/
/*****************************************************/

size_t str_find_specific(const char* str, const char find_char)
{
    const char* pos = str;
    size_t num = 0;
    
    while(*pos != ASCII_CODE_NUL) {
        if (*pos == '\\') {
            pos++;
        } else if (*pos == find_char) {
            num++;
        }
        pos++;
    }
    return num;
}

/*输入的字符串都是数字
是则返回1，否则返回0*/
ut_bool_t str_is_num(const char* str)
{
    int32_t pos = 0;
    int32_t point_flag = 0;

    if (*str == '-' || *str == '+') {
        pos++;
    }

    while (*(str+pos) != ASCII_CODE_NUL) {
        if (!char_is_num(*(str+pos))) {
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


/*字符串str是ip地址
是则返回0，返回其他数值标识不是ip地址*/
int32_t str_is_ipv4_addr(const char * str)
{
    int32_t count = 0;
    char ip_addr[4]= {0, 0, 0, 0};
    int32_t num_count = 0;
    int32_t dot_count = 0;
    char ch = ASCII_CODE_NUL;
    int32_t temp = 0;

    while (char_is_separator(*(str+count++))) {
        ;
    }
    count--;

    ch = *(str+count);
    if (ch == ASCII_CODE_NUL) {
        return count+1;
    }

    while (!char_is_separator(ch) && ch) {
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
        if (!char_is_separator(ch)) {
            return count+1;
        }
        count++;
        ch=*(str+count);
    }
    return 0;
}

/*字符串str是ip地址
是则返回0，返回其他数值标识不是ip地址*/
int32_t str_is_ipv6_addr(const char *string)
{
    int cnt = 0;
    char ch;
    int pos = 0;
    int num = 0;
    int colon_num = 0;
    int flag = 0; //标记::
    int ipv4_flag = 0;

    while (char_is_separator(*(string+cnt))) {
        cnt++;
    }

    ch=*(string+cnt);
    if (ch==0x00) {
        goto TAG_EXIT;
    }

    while(ch) {
        if (char_is_separator(ch)) {
            break;
        }

        if ((ch <= '9' && ch >= '0') || (char_lower(ch) >= 'a' && char_lower(ch) <= 'f')) {
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
        ch = *(string + cnt);
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
        if (str_is_ipv4_addr(string + pos + 1)) {
            goto TAG_EXIT;
        }
    }

    return 0;

TAG_EXIT:
    return -1;
}

/*字符串str是mac地址
是则返回0，返回其他数值标识不是mac地址*/
int32_t str_is_macaddr(const char * str)
{

    int32_t count;
    int32_t num_count;
    int32_t dot_count;
    char ch;

    count=0;
    num_count=0;
    dot_count=0;
    while (char_is_separator(*(str+count++))) {
        ;
    }
    count--;
    ch=*(str+count);
    if (ch==0x00) {
        return count+1;
    }
    while (!char_is_separator(ch) && ch) {
        if (char_is_hex(ch)) {
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
        if (!char_is_separator(ch)) {
            return count+1;
        }
        count++;
        ch=*(str+count);
    }
    return 0;
}

/*字符串str是16进制数
是则返回0，返回其他数值标识不是16进制数*/
int32_t str_is_hex(const char * str)
{
    int32_t count = 0;
    char ch;

    ch = *(str+count);
    while (ch != ASCII_CODE_NUL && !char_is_separator(ch)) {
        if ((ch <= '9' && ch >= '0')
                || (char_lower(ch) >= 'a' && char_lower(ch) <= 'f')) {
            ;
        } else {
            return count+1;
        }
        count++;
        ch = *(str+count);
    }

    return 0;
}

size_t strncpy_size(char* dest, const char* src, size_t size)
{
    char* dest_pos = dest;
    const char* src_pos = src;
    size_t copied_len = 0;

    if (NULL == dest || NULL == src) {
        return 0;
    }

    while (*src_pos != ASCII_CODE_NUL) {
        if (copied_len >= size) {
            break;
        }
        *(dest_pos++) = *(src_pos++);
        copied_len++;
    }

    return copied_len;
}

size_t strcpy_size(char* dest, const char* src)
{
    char* dest_pos = dest;
    const char* src_pos = src;
    size_t copied_len = 0;

    if (NULL == dest || NULL == src) {
        return 0;
    }

    while (*src_pos != ASCII_CODE_NUL) {
        *(dest_pos++) = *(src_pos++);
        copied_len++;
    }

    return copied_len;
}
/*将src复制到dest中，直到src到达行尾或遇到ch为止
返回复制的长度*/
size_t strcpy_until_char(char* dest, const char* src, char ch)
{
    char* dest_pos = dest;
    const char* src_pos = src;
    size_t copied_len = 0;

    if (NULL == dest || NULL == src) {
        return 0;
    }

    while ((*src_pos != ASCII_CODE_NUL) && (*src_pos != ch)) {
        *(dest_pos++) = *(src_pos++);
        copied_len++;
    }

    return copied_len;
}

size_t strcpy_until_char_specific(char* dest, const char* src, char ch)
{
    char* dest_pos = dest;
    const char* src_pos = src;
    size_t copied_len = 0;

    if (NULL == dest || NULL == src) {
        return 0;
    }

    while (*src_pos != ASCII_CODE_NUL) {
        if (*src_pos == '\\') {
            *(dest_pos++) = *(src_pos++);
            copied_len++;
        } else if (*src_pos == ch) {
            break;
        }
        *(dest_pos++) = *(src_pos++);
        copied_len++;
    }

    return copied_len;
}

ut_bool_t strcpy_delimited_by_space(char* save_buf, const char* analysis_str, uint32_t* pos)
{
    char ch = ASCII_CODE_NUL;
    int copy_pos = 0;
    int tmp_pos = *pos;
    ut_bool_t ret = UT_TRUE;

    /*清除命令前的所有空格*/
    do {
        ch = *(analysis_str + tmp_pos++);
        if (!char_is_separator(ch)) {
            tmp_pos--;
            break;
        }
    }while (ch != ASCII_CODE_NUL);

    /*获取命令字符串中的一组字符*/
    do {
        ch = *(analysis_str + tmp_pos++);
        if (!char_is_separator(ch)) {
            *(save_buf+copy_pos++) = ch;
        } else {
            break;
        }
    } while (ASCII_CODE_NUL != ch);

    *(save_buf+copy_pos++) = ASCII_CODE_NUL;
    
    if (ch == ASCII_CODE_NUL) {
        ret = UT_FALSE;
    }

    if (strlen(save_buf)) {
        *pos = tmp_pos;
    }

    return ret;
}

size_t strlen_until_char(const char* src, const char ch)
{
    size_t size = 0;
    const char* pos = src;

    while (((*pos) != ASCII_CODE_NUL) && ((*pos) != ch)) {
        size++;
        pos++;
    }

    return size;
}

size_t strlen_until_char_specific(const char* src, const char ch)
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

size_t strcpy_until_ptr(char* dest, const char* src, const char* end)
{
    char* dest_pos = dest;
    const char* src_pos = src;
    size_t copied_len = 0;

    if (NULL == dest || NULL == src) {
        return 0;
    }

    while (src_pos != end && *src != ASCII_CODE_NUL) {
        *(dest_pos++) = *(src_pos++);
        copied_len++;
    }

    return copied_len;
}

char* strchr_specific(const char* src, char ch)
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

char* strchr_count(const char* src, char ch, int count)
{
    char* ret = NULL;

    CHECK_PTR(src);

    while (*src != ASCII_CODE_NUL) {
        if (*src == ch) {
            if (!(count--)) {
                ret = (char*)(uintptr_t)src;
                break;
            }
        }

        src++;
    }

TAG_OUT:
    return ret;
}

char* strchr_case_brackets(const char* src, char ch)
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
                    return (char*)(uintptr_t)(src+pos);
                }
                break;
                
        }/*switch*/
        pos++;
    }/*while*/

TAG_OUT:
    return NULL;
}


/*****************************************************/
/*******************文件处理函数************************/
/*****************************************************/

int32_t fd_readable(ut_fd_t fd)
{
    int32_t read_len = 0;

    /* 获取输入缓冲区大小 */
    ioctl(fd, FIONREAD, &read_len);

    return read_len;
}

int32_t fd_fflush(ut_fd_t fd)
{
    int32_t     cur_len = 0;
    int32_t     read_len = 0;
    char        buf[MAX_RECV_BUF_LEN] = {0};

    /* 获取输入缓冲区内容 */
    cur_len = fd_readable(fd);
    read_len += cur_len;

    /* 如果缓冲区中还有内容，则清空。 */
    while (cur_len != 0) {
        if (read(fd, &buf, MAX_RECV_BUF_LEN) <= 0) {
            break;
        }
        cur_len = fd_readable(fd);
        read_len += cur_len;
    }

    return read_len;
}

void fd_block(ut_fd_t fd, ut_bool_t block)
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

int32_t terminal_get(struct winsize* window_size)
{
    return ioctl(STDIN_FILENO, TIOCGWINSZ, window_size);
}