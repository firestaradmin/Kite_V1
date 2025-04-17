/**
 * @file uv_log.h
 * @author @firestaradmin
 * @brief Print and out log info.  ||  Universal Log module ||
 * @version 0.1.0
 * @date 2023-03-03
 *
 * @history: In the source file.
 *
 * @copyright Copyright (c) 2023 LXG
 *
 */

#ifndef __UV_LOG_H__
#define __UV_LOG_H__
#define __FUNC__ __FUNCTION__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif



/******************** Usage 1 ***************************************************************************/
/*
// 1. difine a mutex variable
SemaphoreHandle_t uvLog_mutex;

// 2. provide lock/unlock func
void uvLogLock()
{
    xSemaphoreTake(uvLog_mutex, portMAX_DELAY );

}
void uvLogUnlock()
{
    xSemaphoreGive(uvLog_mutex);
}

// 3. init mutex
void uv_log_mutex_init( )
{
    uvLog_mutex = xSemaphoreCreateMutex();
    if( uvLog_mutex == NULL )
    {
        printf("uv log mutex err!\r\n");
    }
}

// 4. init the log module
void app_main(void)
{
    uv_log_mutex_init();
    uv_log_init(UV_LOG_MUTEXWAY_EXTERNEL, printf, uvLogLock, uvLogUnlock);
    UVINFO("hello world!\r\n");
}

// 5. output:
[12:07:25.467] [ info] [@app_main             ]| hello world!

*/
/******************** Usage 1 - END *********************************************************************/



/******************** TYPEDEF ***************************************************************************/
/**
 * The log print mask level
*/
#define  UV_LOG_LEVEL_DEBUG     0       /* Log everything */
#define  UV_LOG_LEVEL_INFO      1       /* Log some info which programer want to output */
#define  UV_LOG_LEVEL_WARN      2       /* Log if something unwanted happened but didn't caused problem */
#define  UV_LOG_LEVEL_ERROR     3       /* Only critical issue, when the system may fail */
#define  UV_LOG_LEVEL_USER      4       /* Only print user output */
#define  UV_LOG_LEVEL_NONE      5       /* Do not log anything */



/* Output array data format type define */
typedef enum uv_log_arrayDataFormat
{
    UV_LOG_ARRAY_FORMAT_HEX = 0,
    UV_LOG_ARRAY_FORMAT_DEC,
    UV_LOG_ARRAY_FORMAT_BIN
} uv_log_arrayDataFormat_t;


typedef enum uv_log_mutexWay {
    UV_LOG_MUTEXWAY_NONE = 0,
    UV_LOG_MUTEXWAY_INTERNEL,
    UV_LOG_MUTEXWAY_EXTERNEL
}uv_log_mutexWay_t;


typedef struct uv_log_module{
    void (*lockMutex)(void);
    void (*unlockMutex)(void);
    int (*print_hal_cb)(char *str);
    uv_log_mutexWay_t   mutexWay;
    uint8_t _intialized : 1;
    uint8_t _big_endian : 1;
} uv_log_module_t;




/******************** TYPEDEF - END *********************************************************************/






/******************** User config ***************************************************************************/


/**
 * Config the log of all program output level
*/
#define UV_LOG_GLOBAL_LEVEL    UV_LOG_LEVEL_DEBUG

/**
 * Config the log to save levle
*/
#define UV_LOG_SAVE_FILE_LEVEL    UV_LOG_LEVEL_INFO





/*
等宽输出样式使能：
使用等宽函数名称输出 能帮助更好的观看log
example: [time] [debug][@my_example_func      ]| hello world!
example: [time] [debug][@my_example_a...s_func]| hello world!
[12:07:25.467] [debug] [@app_main             ]| hello world!
[12:07:25.467] [debug] [@app_main             ]| hello world!
*/
/* Config whether use monospace func print */
#define UV_LOG_MONOSPACE_FUNCNAME_ENABLE     1

#if UV_LOG_MONOSPACE_FUNCNAME_ENABLE
    /* Config the func name whether align left */
    #define UV_LOG_MONOSPACE_FUNCNAME_ALIGN_LEFT    1

    /* Config the func name character length */
    #define UV_LOG_MONOSPACE_FUNCNAME_LENGTH    20
#endif

/* Config whether add timestamp in head 
 * example: [5.5-14:52:22][Debug] [@main   ]| hello world!
 */
#define UV_LOG_TIMESTAMP_ENABLE     1
#if UV_LOG_TIMESTAMP_ENABLE
    #include <time.h>

    // #define UV_LOG_TIMESTAMP_CUSTOM_FUN 1
#endif


/* Config array print endian format, Big or Little Endian */
#define UV_LOG_ARRAY_PRINT_BIG_ENDIAN   1


/* Mask PRINT API outputs */
#define UV_LOG_PRINT_SHIELD             0

/* log cache size, byte */
#define UV_LOG_MAX_LEN                  2048
/* log-head cache size, byte */
#define UV_LOG_HEAD_MAX_LEN             256
/* array-log cache size, byte */
#define UV_LOG_ARRAY_MAX_SIZE           256
/* array-log single unit cache size, byte */
#define UV_LOG_ARRAY_SINGLE_MAX_LEN     16

/* Config whether auto flush sys io cache - stdout */
#define UV_LOG_SYS_AUTO_FFLUSH_CACHE    1

/** 
 * Config whether enable uv_api interface to write log 
 * This API is new generation of log
*/
#define UV_LOG_API_ENABLE   1

#if UV_LOG_API_ENABLE

#define  UV_LOG_API_OUTTO_CONSOLE    1
/* define for uv_log() */
#define  UVL_DEBUG  UV_LOG_LEVEL_DEBUG   /* Log everything */
#define  UVL_INFO   UV_LOG_LEVEL_INFO    /* Log some info which programer want to output */
#define  UVL_WARN   UV_LOG_LEVEL_WARN    /* Log if something unwanted happened but didn't caused problem */
#define  UVL_ERROR  UV_LOG_LEVEL_ERROR   /* Only critical issue, when the system may fail */
#define  UVL_USER   UV_LOG_LEVEL_USER    /* Only print user output */
#define  UVL_NONE   UV_LOG_LEVEL_NONE    /* Do not log anything */

#endif 

/* 配置是否支持日志储存到文件的配置功能 */
#define     UV_LOG_SAVE_LOG_CONFIG_FILE_ENBALE      0
#if UV_LOG_SAVE_LOG_CONFIG_FILE_ENBALE
    #define SAVE_LOG_CONFIG_FILE_PATH "./config_log_level"
#endif
/******************** User config - END *********************************************************************/




/******************** USER API ***************************************************************************/

#if (UV_LOG_GLOBAL_LEVEL <= UV_LOG_LEVEL_DEBUG)
    #define UVDEBUG(...)        printLogS(__FUNC__, "Debug", UV_LOG_LEVEL_DEBUG, __VA_ARGS__)
    #define UVDEBUG_F(...)      printLogFull(__FILE__, __LINE__, __FUNC__, "Debug", UV_LOG_LEVEL_DEBUG, __VA_ARGS__)
    #define UVDEBUG_BARE(...)   printS_bare(UV_LOG_LEVEL_DEBUG, __VA_ARGS__)
    #define UVDEBUG_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\
            printLogS_array(__FUNC__, "Debug", UV_LOG_LEVEL_DEBUG, array, len, sectionByte, space, prefix, UV_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)
#else
    #define UVDEBUG(...)
    #define UVDEBUG_F(...)
    #define UVDEBUG_BARE(...)
    #define UVDEBUG_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)
#endif


#if (UV_LOG_GLOBAL_LEVEL <= UV_LOG_LEVEL_INFO)
    #define UVINFO(...)         printLogS(__FUNC__, " Info", UV_LOG_LEVEL_INFO, __VA_ARGS__)
    #define UVINFO_F(...)       printLogFull(__FILE__, __LINE__, __FUNC__, " Info", UV_LOG_LEVEL_INFO, __VA_ARGS__)
    #define UVINFO_BARE(...)    printS_bare(UV_LOG_LEVEL_INFO, __VA_ARGS__)
    #define UVINFO_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\
            printLogS_array(__FUNC__, " Info", UV_LOG_LEVEL_INFO, array, len, sectionByte, space, prefix, UV_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)
#else
    #define UVINFO(...)
    #define UVINFO_F(...)
    #define UVINFO_BARE(...)
    #define UVINFO_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)
#endif

#if (UV_LOG_GLOBAL_LEVEL <= UV_LOG_LEVEL_WARN)
    #define UVWARNING(...)      printLogS(__FUNC__, " Warn", UV_LOG_LEVEL_WARN, __VA_ARGS__)
    #define UVWARNING_F(...)    printLogFull(__FILE__, __LINE__, __FUNC__, " Warn", UV_LOG_LEVEL_WARN, __VA_ARGS__)
    #define UVWARNING_BARE(...) printS_bare(UV_LOG_LEVEL_WARN, __VA_ARGS__)
    #define UVWARNING_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\
            printLogS_array(__FUNC__, " Warn", UV_LOG_LEVEL_WARN, array, len, sectionByte, space, prefix, UV_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)
#else
    #define UVWARNING(...)
    #define UVWARNING_F(...)
    #define UVWARNING_BARE(...)
    #define UVWARNING_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)
#endif

#if (UV_LOG_GLOBAL_LEVEL <= UV_LOG_LEVEL_ERROR)
    #define UVERROR(...)        printLogS(__FUNC__, "Error", UV_LOG_LEVEL_ERROR, __VA_ARGS__)
    #define UVERROR_F(...)      printLogFull(__FILE__, __LINE__, __FUNC__, "Error", UV_LOG_LEVEL_ERROR, __VA_ARGS__)
    #define UVERROR_BARE(...)   printS_bare(UV_LOG_LEVEL_ERROR, __VA_ARGS__)
    #define UVERROR_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\
            printLogS_array(__FUNC__, "Error", UV_LOG_LEVEL_ERROR, array, len, sectionByte, space, prefix, UV_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)
#else
    #define UVERROR(...)
    #define UVERROR_F(...)
    #define UVERROR_BARE(...)
    #define UVERROR_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)
#endif


#if (UV_LOG_GLOBAL_LEVEL <= UV_LOG_LEVEL_USER)
    #define UVUSER(...)        printLogS(__FUNC__, " User", UV_LOG_LEVEL_USER, __VA_ARGS__)
    #define UVUSER_F(...)      printLogFull(__FILE__, __LINE__, __FUNC__, " User", UV_LOG_LEVEL_USER, __VA_ARGS__)
    #define UVUSER_BARE(...)   printS_bare(UV_LOG_LEVEL_USER, __VA_ARGS__)
    #define UVUSER_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\
            printLogS_array(__FUNC__, " User", UV_LOG_LEVEL_USER, array, len, sectionByte, space, prefix, UV_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)
#else
    #define UVUSER(...)
    #define UVUSER_F(...)
    #define UVUSER_BARE(...)
    #define UVUSER_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)
#endif


#if (UV_LOG_PRINT_SHIELD == 0)
    #define UVBR()          UVPRINT("\n")
    #define UVCR()          UVPRINT("\r")
    #define UVCRBR()        UVPRINT("\r\n")
    #define UVPRINT(...)    printS(__VA_ARGS__)
    #define UVPRINT_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)\
            printS_array(array, len, sectionByte, space, prefix, UV_LOG_ARRAY_FORMAT_HEX, __VA_ARGS__)
#else
    #define UVBR()    
    #define UVCR()    
    #define UVCRBR()   
    #define UVPRINT(...)
    #define UVPRINT_ARRAY_HEX(array, len, sectionByte, space, prefix, ...)
#endif
/******************** USER API - END *********************************************************************/




/******************** FUNC ***************************************************************************/

/**
 * @brief init log module
 *
 * @param mutexWay  UV_LOG_MUTEXWAY_NONE,
                    UV_LOG_MUTEXWAY_INTERNEL,
                    UV_LOG_MUTEXWAY_EXTERNEL
 * @param print_hal_cb  print log out hal callback
 * @param lockMutex lockMutex callback
 * @param unlockMutex unlockMutex callback
 * @return int 0:Success
 */
int uv_log_init(uv_log_mutexWay_t mutexWay,
                int (*print_hal_cb)(char *str),
                void (*lockMutex)(void),
                void (*unlockMutex)(void));

/**
 * @brief Log print callback, User must implemente it 
 * 
 * @param str 
 */
void uv_log_hal_print(char *str);


#if UV_LOG_API_ENABLE

void uv_log(int level, const char *fmt, ...);

void uv_log_forMacro(const char *funcname, int line, int level, const char *fmt, ...);

#define UV_LOG(level, ...)  uv_log_forMacro(__FUNC__, __LINE__, level, __VA_ARGS__)
#endif



/************************
 *   utl
 ************************/

int uvGetArrayHexStr(char *dest, size_t size,
                     uint8_t *array, uint32_t array_len, uint8_t sectionByteNum,
                     const char *space_character, const char *prefix);

int uvStrToHex(const char *str, uint8_t *dest, size_t size);
/******************** FUNC - END *********************************************************************/


/******************** Not recommand to use ***************************************************************************/

void printS(const char *fmt, ...);

void printS_bare(uint8_t level, const char *fmt, ...);

void printLogS(const char *funcname, const char *type, uint8_t level, const char *fmt, ...);

void printLogFull(const char *file, int line, const char *funcname, const char *type,
                  uint8_t level, const char *fmt, ...);

void printS_array(uint8_t *array, uint32_t array_len, uint8_t sectionByteNum,
                  const char *space_character, const char *prefix, uint8_t format, const char *fmt, ...);

void printLogS_array(const char *funcname,
                     const char *type,
                     uint8_t level,
                     uint8_t *array,
                     uint32_t array_len,
                     uint8_t sectionByteNum,
                     const char *space_character,
                     const char *prefix,
                     uv_log_arrayDataFormat_t format,
                     const char *fmt, ...);

/**
 * @brief 格式化指定长度的字符串，如果长度大于len，则返回的字符串中间将使用...来代替
 *
 * @param src
 * @param dest
 * @param len string length, not sizeof
 * @return char*
 */
char *uv_log_get_specific_length_string(char *src, char *dest, int len);

/******************** Not recommand to use - END *********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // !__UV_LOG_H__













