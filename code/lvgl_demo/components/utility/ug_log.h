/*! @file ug_log.h
 *  @version 0.0.1
 *  @date May 12 2021
 *  @author firestaradmin
 *
 *  @brief
 *  Print and out log info.
 *
 */


#ifndef __UG_LOG_H
#define __UG_LOG_H

// #include "nwy_osi_api.h"
#include "stdint.h"
// typedef unsigned char        uint8_t;
// typedef unsigned short int   uint16_t;
// typedef signed char          int8_t;
// typedef unsigned int          uint32_t;

/*************************************************** CONFIG ***************************************************/
/* controls whether to print debug type log */
#define UG_LOG_DEBUG_OUT    1


// 日志内容缓冲大小 
#define UG_LOG_MAX_LEN              1024
// 日志前缀缓冲大小
#define UG_LOG_HEAD_MAX_LEN         128
// 打印数组缓冲大小
#define UG_LOG_ARRAY_MAX_SIZE       256    
// 打印数组单个字节区最大长度
#define UG_LOG_ARRAY_SINGLE_MAX_LEN 16      


/* controls whether specify funcname length */
#define UG_LOG_FUNCNAME_LEN_SPECIFY     1
#if UG_LOG_FUNCNAME_LEN_SPECIFY
    #define UG_LOG_FUNCNAME_LEN_SPECIFY_ALIGN_LEFT 1
#endif

/************************************************* CONFIG - END ***********************************************/



typedef enum ug_log_mutexWay {
    ug_log_mutexWay_none = 0,
    ug_log_mutexWay_internel = 1,
    ug_log_mutexWay_externel = 2,
}ug_log_mutexWay_t;

typedef struct ug_log_module{
    ug_log_mutexWay_t mutexWay;
    void (*lockMutex)(void);
    void (*unlockMutex)(void);
    int (*print_hal_cb)(char *str);
    uint8_t intialized : 1;
} ug_log_module_t;




/************** out array format type define **************/
#define UG_LOG_ARRAY_FORMAT_HEX     0
#define UG_LOG_ARRAY_FORMAT_DEC     1
#define UG_LOG_ARRAY_FORMAT_BIN     2


#define UG_LOG_ARRAY_PREFIX_0x      1
#define UG_LOG_ARRAY_PREFIX_NONE    0





#define __FUNC__ __FUNCTION__
/**************  user use macro **************/

#define UGPRINT(...)        printS(__VA_ARGS__)

// this error will not insert to reportLog Module
#define UGERROR_S(...)        printLogS_noInsert(__FUNC__, "error", __VA_ARGS__)

// log out info
#define UGINFO(...)         printLogS(__FUNC__, " info", __VA_ARGS__)
#define UGERROR(...)        printLogS(__FUNC__, "error", __VA_ARGS__)
#define UGWARNING(...)      printLogS(__FUNC__, " warn", __VA_ARGS__)

// log out full info
#define UGINFO_F(...)       printLogFull(__FILE__, __LINE__, __FUNC__, " info", __VA_ARGS__)
#define UGERROR_F(...)      printLogFull(__FILE__, __LINE__, __FUNC__, "error", __VA_ARGS__)
#define UGWARNING_F(...)    printLogFull(__FILE__, __LINE__, __FUNC__, " warn", __VA_ARGS__)



#define UGPRINT_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\   
        printS_array(array, len, sectionByte, space, prefix, UG_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)
#define UGINFO_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\   
        printLogS_array(__FUNC__, " info", array, len, sectionByte, space, prefix, UG_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)   


#define UGBR()  UGPRINT("\n")
#define UGCR()  UGPRINT("\r")
#define UGCRBR()  UGPRINT("\r\n")



#if UG_LOG_DEBUG_OUT
#define UGDEBUG(...)        printLogS(__FUNC__, "debug", __VA_ARGS__)
#define UGDEBUG_F(...)      printLogFull(__FILE__, __LINE__, __FUNC__, "debug", __VA_ARGS__)
#define UGDEBUG_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\   
        printLogS_array(__FUNC__, "debug", array, len, sectionByte, space, prefix, UG_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)   
#else
#define UGDEBUG(...)        
#define UGDEBUG_F(...)      
#define UGDEBUG_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)
#endif



// #define USE_UEGUI_LIB
#ifdef USE_UEGUI_LIB
    #include "ug_user_config.h"
    /*Possible log level. For compatibility declare it independently from `UG_USE_LOG`*/
    #define UG_LOG_LEVEL_TRACE 0 /**< A lot of logs to give detailed information*/
    #define UG_LOG_LEVEL_INFO 1  /**< Log important events*/
    #define UG_LOG_LEVEL_WARN 2  /**< Log if something unwanted happened but didn't caused problem*/
    #define UG_LOG_LEVEL_ERROR 3 /**< Only critical issue, when the system may fail*/
    #define UG_LOG_LEVEL_USER 4 /**< Custom logs from the user*/
    #define UG_LOG_LEVEL_NONE 5 /**< Do not log anything*/
    #define _UG_LOG_LEVEL_NUM 6 /**< Number of log levels */

    UG_EXPORT_CONST_INT(UG_LOG_LEVEL_TRACE);
    UG_EXPORT_CONST_INT(UG_LOG_LEVEL_INFO);
    UG_EXPORT_CONST_INT(UG_LOG_LEVEL_WARN);
    UG_EXPORT_CONST_INT(UG_LOG_LEVEL_ERROR);
    UG_EXPORT_CONST_INT(UG_LOG_LEVEL_USER);
    UG_EXPORT_CONST_INT(UG_LOG_LEVEL_NONE);

    #define UG_LOG_LEVEL UG_LOG_LEVEL_TRACE

    typedef int8_t ug_log_level_t;
    /**
     * Add a log
     * @param level the level of log. (From `ug_log_level_t` enum)
     * @param file name of the file when the log added
     * @param line line number in the source code where the log added
     * @param func name of the function when the log added
     * @param format printf-like format string
     * @param ... parameters for `format`
     */
    void _ug_log_add(char* type, const char * file, int line, const char * funcname, const char * format, ...);


    #if UG_LOG_LEVEL <= UG_LOG_LEVEL_TRACE
    #define UG_LOG_TRACE(...) _ug_log_add("Trace", __FILE__, __LINE__, __func__, __VA_ARGS__);
    #else
    #define UG_LOG_TRACE(...)                                                                                              \
        {                                                                                                                  \
            ;                                                                                                              \
        }
    #endif

    #if UG_LOG_LEVEL <= UG_LOG_LEVEL_INFO
    #define UG_LOG_INFO(...) _ug_log_add(" Info", __FILE__, __LINE__, __func__, __VA_ARGS__);
    #else
    #define UG_LOG_INFO(...)                                                                                               \
        {                                                                                                                  \
            ;                                                                                                              \
        }
    #endif

    #if UG_LOG_LEVEL <= UG_LOG_LEVEL_WARN
    #define UG_LOG_WARN(...) _ug_log_add(" Warn", __FILE__, __LINE__, __func__, __VA_ARGS__);
    #else
    #define UG_LOG_WARN(...)                                                                                               \
        {                                                                                                                  \
            ;                                                                                                              \
        }
    #endif

    #if UG_LOG_LEVEL <= UG_LOG_LEVEL_ERROR
    #define UG_LOG_ERROR(...) _ug_log_add("Error", __FILE__, __LINE__, __func__, __VA_ARGS__);
    #else
    #define UG_LOG_ERROR(...)                                                                                              \
        {                                                                                                                  \
            ;                                                                                                              \
        }
    #endif

    #if UG_LOG_LEVEL <= UG_LOG_LEVEL_USER
    #define UG_LOG_USER(...) _ug_log_add(" User", __FILE__, __LINE__, __func__, __VA_ARGS__);
    #else
    #define UG_LOG_USER(...)                                                                                               \
        {                                                                                                                  \
            ;                                                                                                              \
        }
    #endif
#endif








/**
 * @brief init log module
 * 
 * @param mutexWay      ug_log_mutexWay_none = 0,
                        ug_log_mutexWay_internel = 1,
                        ug_log_mutexWay_externel = 2,
 * @param print_hal_cb  print log out hal callback
 * @param lockMutex 
 * @param unlockMutex 
 * @return int 
 */
int ug_log_init(ug_log_mutexWay_t mutexWay,
                int (*print_hal_cb)(char *str),
                void (*lockMutex)(void),
                void (*unlockMutex)(void));

void printS(const char *fmt, ...);
void printLogS(const char* funcname,const char* type, const char *fmt, ...);
void printLogS_noInsert(const char *funcname, const char *type, const char *fmt, ...);
void printLogFull(const char *file, int line, const char *funcname, const char *type, const char *fmt, ...);
void ug_log_hal_print(char *str);


/**
 * @brief 
 * 
 * @param array 
 * @param array_len 
 * @param sectionByteNum 
 * @param space_character 
 * @param prefix 
 * @param format 
 * @param fmt 
 * @param ... 
 */
void printS_array(uint8_t *array, uint32_t array_len, uint8_t sectionByteNum, char *space_character, char *prefix, uint8_t format, const char *fmt, ...);
/**
 * @brief print log array
 * 
 * @param funcname 
 * @param type 
 * @param array 
 * @param array_len 
 * @param sectionByteNum 一段数据的字节数
 * @param space_character 数据间隔符号
 * @param prefix 数据前缀
 * @param format 输出格式
 * @param fmt 
 * @param ... 
 */
void printLogS_array(const char *funcname,
                     const char *type,
                     uint8_t *array,
                     uint32_t array_len,
                     uint8_t sectionByteNum,
                     char *space_character,
                     char *prefix,
                     uint8_t format,
                     const char *fmt, ...);

/**
 * @brief 格式化指定长度的字符串，如果长度大于len，则返回的字符串中间将使用...来代替
 * 
 * @param src 
 * @param dest 
 * @param len string length, not sizeof
 * @return char* 
 */
char *_get_specific_length_string(char *src, char *dest, int len);

#endif // !__UG_LOG_H












/*************************************************** note ***************************************************/

/*


 1.原样输出字符串:
    printf("%s", str);


2. 输出指定长度的字符串, 超长时不截断, 不足时右对齐:
    printf("%Ns", str);             --N 为指定长度的10进制数值


3. 输出指定长度的字符串, 超长时不截断, 不足时左对齐:
    printf("%-Ns", str);            --N 为指定长度的10进制数值


4. 输出指定长度的字符串, 超长时截断, 不足时右对齐:
    printf("%N.Ms", str);           --N 为最终的字符串输出长度
                                    --M 为从参数字符串中取出的子串长度


5. 输出指定长度的字符串, 超长时截断, 不足时左对齐是:
    printf("%-N.Ms", str);          --N 为最终的字符串输出长度
                                    --M 为从参数字符串中取出的子串长度
*/
/************************************************* note - END ***********************************************/



