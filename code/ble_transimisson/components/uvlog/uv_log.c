/**
 * @file uv_log.c
 * @author @firestaradmin
 * @brief Print and out log info.  ||  Universal Log module ||
 * @version 0.1.0
 * @date 2023-03-03
 * 
 * @history: 
 * 
 *      2023/09/12: Version 0.3.0
 *          chore: 从特殊项目移植出通用版
 *          fix: 修复了全局日志等级配置生效问题
 * 
 * 
 *      2023/05/15: Version 0.2.0
 *          feat: new generation of log api(will write to file)
 * 
 *      2023/05/05: Version 0.1.1
 *          feat: add timestamp 
 *          feat: support fflush stdout for linux C
 * 
 * 
 *      2023/03/03: Version 0.1.0 
 *          refactor: modified & port & perf for TBOX-PJT
 *          feature: add support of func name length config
 *          feature: add log level feature
 *          feature: add Big/Little Endian format array print
 * 
 *      2022/11/15: Version 0.0.4 
 *          port: port for UEGUI
 * 
 *      2022/05/12: Version 0.0.3 
 *          feature: Modular modules make it more versatile  
 * 
 *      2022/03/29: Version 0.0.2 
 *          perf: modified some feature
 * 
 * @copyright Copyright (c) 2023 LXG 
 * 
 */


/* C libs */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* my libs */
#include "uv_log.h"



/******************** DEFINE ***************************************************************************/
#define __STR1(R) #R
#define __STR2(R) __STR1(R)
#define FUNCNAME_ALIGN_LEFT_FORMAT_STRING\
        "[%s] [@%-"__STR2(UV_LOG_MONOSPACE_FUNCNAME_LENGTH)"."__STR2(UV_LOG_MONOSPACE_FUNCNAME_LENGTH)"s]| "
#define FUNCNAME_ALIGN_RIGHT_FORMAT_STRING\
        "[%s] [@%"__STR2(UV_LOG_MONOSPACE_FUNCNAME_LENGTH)"."__STR2(UV_LOG_MONOSPACE_FUNCNAME_LENGTH)"s]| "


/******************** DEFINE - END *********************************************************************/


/******************** var  ***************************************************************************/

static uv_log_module_t __module = {0};

static char     echo_str[UV_LOG_MAX_LEN];
static char     log_head_str[UV_LOG_HEAD_MAX_LEN];
static uint8_t  tempArray[UV_LOG_ARRAY_MAX_SIZE];
static char     log_func_name_temp_buf[UV_LOG_MONOSPACE_FUNCNAME_LENGTH + 4];


/******************** var  - END *********************************************************************/

#if UV_LOG_API_ENABLE

static char     s_temp_buf[UV_LOG_MAX_LEN];

#endif 



#if UV_LOG_TIMESTAMP_ENABLE


char* _get_timestamp_string(void)
{
    static char time_str[25];
    time_t now = time(NULL);
    
    struct tm *local_time = localtime(&now);
    if((local_time->tm_year+1900) < 2023){
        // printf("failed tm_year:%d\n", local_time->tm_year);
        // strftime(time_str, sizeof(time_str), "[--invalid--%02H:%02M:%02S]", local_time);
        // strftime(time_str, sizeof(time_str), "[%Y.%m.%d-%02H:%02M:%02S]", local_time);
        snprintf(time_str, sizeof(time_str), "[--:--:--]");
    }
    else{
        // printf("tm_year:%d\n", local_time->tm_year);
        // strftime(time_str, sizeof(time_str), "[%m.%d-%02H:%02M:%02S]", local_time);
        // strftime(time_str, sizeof(time_str), "[%Y.%m.%d-%02H:%02M:%02S]", local_time);
        snprintf(time_str, sizeof(time_str), "[%02d:%02d:%02d]", local_time->tm_hour,local_time->tm_min,local_time->tm_sec);
    }

    return time_str;
}
#endif


#if UV_LOG_SAVE_LOG_CONFIG_FILE_ENBALE
    int _log_save_level = (int)UV_LOG_LEVEL_INFO;
#endif

    /******************** INTERNAL MUTEX ***************************************************************************/
#include <pthread.h>
    static pthread_mutex_t _internal_mutex;

    int _internalMutexInit(void)
    {
        int ret = pthread_mutex_init(&_internal_mutex, NULL);
        if (ret != 0)
        {
            UVERROR("pthread_mutex_init _internal_mutex err \n");
            return -1;
        }
        return 0;
}

void _internalLockMutex(void)
{
    pthread_mutex_lock(&_internal_mutex);
}
void _internalUnlockMutex(void)
{
    pthread_mutex_unlock(&_internal_mutex);
}

/******************** INTERNAL MUTEX - END *********************************************************************/





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
                void (*unlockMutex)(void) )
{
    if(print_hal_cb == NULL)
        return -1;
    memset(&__module, 0, sizeof(uv_log_module_t));
    __module.print_hal_cb = print_hal_cb;
    __module.mutexWay = mutexWay;
    if(__module.mutexWay == UV_LOG_MUTEXWAY_INTERNEL){
        if(_internalMutexInit() != 0){
            return -2;
        }
        __module.lockMutex = _internalLockMutex;
        __module.unlockMutex = _internalUnlockMutex;
    }
    else if(__module.mutexWay == UV_LOG_MUTEXWAY_EXTERNEL){
        __module.lockMutex = lockMutex;
        __module.unlockMutex = unlockMutex;
    }
    else{
        __module.lockMutex = NULL;
        __module.unlockMutex = NULL;
    }
    __module._intialized = 1;

    // 判断大小端，以支持数组打印多余1byte的字段
    union union_hex_format{
        uint32_t a;
        uint8_t b;
    } AA;
    AA.a = 1;
    if(AA.b != 0){
        // Little Endian
        __module._big_endian = 0;
        UVDEBUG("system data format: Little Endian\r\n");
    }
    else {
        // Big Endian
        __module._big_endian = 1;
        UVDEBUG("system data format: Big Endian\r\n");
    }

    
#if UV_LOG_SAVE_LOG_CONFIG_FILE_ENBALE
    int loadconfigok = 0;
    if (access(SAVE_LOG_CONFIG_FILE_PATH, F_OK) != -1) 
    {
        FILE *file = fopen(SAVE_LOG_CONFIG_FILE_PATH, "r");
        if (file != NULL) {
            if (fscanf(file, "%d", &_log_save_level) == 1) {
                UVINFO("Load log level: %d\n", _log_save_level);
                loadconfigok = 1;
            }
            else{
                UVWARNING("fscanf level failed.\n");
            }

            fclose(file);
        }
        else{
            UVWARNING("Open file %s fail.\n", SAVE_LOG_CONFIG_FILE_PATH);
        }
    };
    
    if(loadconfigok == 0){
        _log_save_level = (int)UV_LOG_LEVEL_INFO;
        UVINFO("Can't load log level config file, use default level: %d\n", _log_save_level);
    }
#endif


    return 0;
}




void printS(const char *fmt, ...)
{
    if(__module._intialized != 1)
        return;
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);

    // uv_log_hal_print(echo_str);
    __module.print_hal_cb(echo_str);


#if UV_LOG_SYS_AUTO_FFLUSH_CACHE
    fflush(stdout);
#endif

    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }

}

void printS_bare(uint8_t level, const char *fmt, ...)
{
    if(__module._intialized != 1)
        return;
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);

    // uv_log_hal_print(echo_str);
    __module.print_hal_cb(echo_str);
    
#if UV_LOG_SYS_AUTO_FFLUSH_CACHE
    fflush(stdout);
#endif

    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }



}

void printLogS(const char* funcname,const char* type, uint8_t level, const char *fmt, ...)
{
    if(__module._intialized != 1)
        return;
    // static char log_head_str[256];
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);

#if UV_LOG_MONOSPACE_FUNCNAME_ENABLE
    #if UV_LOG_MONOSPACE_FUNCNAME_ALIGN_LEFT
        snprintf(log_head_str, sizeof(log_head_str), FUNCNAME_ALIGN_LEFT_FORMAT_STRING, type, uv_log_get_specific_length_string(funcname, log_func_name_temp_buf, UV_LOG_MONOSPACE_FUNCNAME_LENGTH));
    #else
        snprintf(log_head_str, sizeof(log_head_str), FUNCNAME_ALIGN_RIGHT_FORMAT_STRING, type, uv_log_get_specific_length_string(funcname, log_func_name_temp_buf, UV_LOG_MONOSPACE_FUNCNAME_LENGTH));
    #endif

#else
    snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%s]| ", type, funcname);
#endif

#if UV_LOG_TIMESTAMP_ENABLE
    __module.print_hal_cb(_get_timestamp_string());
#endif 
    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);



#if UV_LOG_SYS_AUTO_FFLUSH_CACHE
    fflush(stdout);
#endif

	if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }

}



void printLogFull(const char* file,int line,const char* funcname,const char* type, uint8_t level, const char *fmt, ...)
{
    if(__module._intialized != 1)
        return;
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);

    // "\n[error][main.c 221] @init():\n"
    snprintf(log_head_str, sizeof(log_head_str), "[** %s **][%s %d] @%s: ", type, file, line, funcname);

#if UV_LOG_TIMESTAMP_ENABLE
    __module.print_hal_cb(_get_timestamp_string());
#endif 
    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);
    

#if UV_LOG_SYS_AUTO_FFLUSH_CACHE
    fflush(stdout);
#endif

	if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }
}


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
void printS_array(uint8_t *array, uint32_t array_len, uint8_t sectionByteNum,
                  const char *space_character, const char *prefix, uint8_t format, const char *fmt, ...)
{
    if(__module._intialized != 1)
        return;
    // static uint8_t tempArray[UV_LOG_ARRAY_MAX_SIZE];
    static char singleSection[UV_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    static char singleSection_temp[UV_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    uint8_t *p_array = NULL;
    uint32_t alignArrayLen = 0;
    uint8_t byte_alignment_cnt;
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }


    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);

    // align array
    memset(tempArray, 0, sizeof(tempArray));
    if(array_len % sectionByteNum == 0){
        p_array = array;
        alignArrayLen = array_len;
    }
    else{
        // 对齐需要的字节数
        byte_alignment_cnt = sectionByteNum - (array_len % sectionByteNum);
        if(array_len + byte_alignment_cnt > UV_LOG_ARRAY_MAX_SIZE){
            array_len = UV_LOG_ARRAY_MAX_SIZE - byte_alignment_cnt;
        }
        memcpy(tempArray + byte_alignment_cnt, array, array_len);   // alignment array
        p_array = tempArray;
        alignArrayLen = array_len + byte_alignment_cnt;
    }

    uint32_t sectionCnt = alignArrayLen / sectionByteNum;
    switch(format)
    {
        case UV_LOG_ARRAY_FORMAT_HEX:
            for (int i = 0; i < sectionCnt; i ++)
            {
                strncat(echo_str, prefix, sizeof(echo_str)-1);    // add prefix
                if((__module._big_endian && UV_LOG_ARRAY_PRINT_BIG_ENDIAN) || (!__module._big_endian && (!UV_LOG_ARRAY_PRINT_BIG_ENDIAN))){
                    for (int j = 0; j < sectionByteNum; j++){
                        snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                        strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
                    }
                }
                else{
                    for (int j = sectionByteNum-1; j >= 0; j--){
                        snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                        strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
                    }
                }

                strncat(echo_str, singleSection, sizeof(echo_str)-1);
                memset(singleSection, 0, sizeof(singleSection));
                if(i != sectionCnt - 1)
                    strncat(echo_str, space_character, sizeof(echo_str)-1);    // add space_character
                else{
                    strncat(echo_str, "\r\n", sizeof(echo_str)-1);
                }
            }

            break;
        case UV_LOG_ARRAY_FORMAT_DEC:
            strncat(echo_str, "UV_LOG_ARRAY_FORMAT_DEC not implemente yet ^_^\r\n", sizeof(echo_str)-1);


            break;
        case UV_LOG_ARRAY_FORMAT_BIN:
            strncat(echo_str, "UV_LOG_ARRAY_FORMAT_BIN not implemente yet ^_^\r\n", sizeof(echo_str)-1);




            break;
        default:
            break;
    }
#if UV_LOG_TIMESTAMP_ENABLE
    __module.print_hal_cb(_get_timestamp_string());
#endif 
    __module.print_hal_cb(echo_str);

#if UV_LOG_SYS_AUTO_FFLUSH_CACHE
    fflush(stdout);
#endif

    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }
}


// TODO: 添加一个控制 是否输出 0占位符的 参数
/**
 * @brief Log array data.
 * 
 * @param funcname the func name, which func called it.
 * @param type log type.
 * @param level log level.
 * @param array the array to be log out.
 * @param array_len the length of array data to be print.(byte)
 * @param sectionByteNum specify byteNum of one section.
 * @param space_character Spacing characters. maybe " "
 * @param prefix the prefix of a section, maybe "0x"
 * @param format Data base format, HEX/DEC/BIN
 * @param fmt c/c++ str format
 * @param ... 
 */
void printLogS_array(const char *funcname,
                     const char *type,
                     uint8_t level,
                     uint8_t *array,
                     uint32_t array_len,
                     uint8_t sectionByteNum,
                     const char *space_character,
                     const char *prefix,
                     uv_log_arrayDataFormat_t format,
                     const char *fmt, ...)
{
    if(__module._intialized != 1)
        return;
    // static uint8_t tempArray[UV_LOG_ARRAY_MAX_SIZE];
    static char singleSection[UV_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    static char singleSection_temp[UV_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    uint8_t *p_array = NULL;
    uint32_t alignArrayLen = 0;
    uint8_t byte_alignment_cnt;
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }


    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);

    // align array
    memset(tempArray, 0, sizeof(tempArray));
    if(array_len % sectionByteNum == 0){
        p_array = array;
        alignArrayLen = array_len;
    }
    else{
        // 对齐需要的字节数
        byte_alignment_cnt = sectionByteNum - (array_len % sectionByteNum);
        if(array_len + byte_alignment_cnt > UV_LOG_ARRAY_MAX_SIZE){
            array_len = UV_LOG_ARRAY_MAX_SIZE - byte_alignment_cnt;
        }
        memcpy(tempArray + byte_alignment_cnt, array, array_len);   // alignment array
        p_array = tempArray;
        alignArrayLen = array_len + byte_alignment_cnt;
    }

    uint32_t sectionCnt = alignArrayLen / sectionByteNum;
    switch(format)
    {
        case UV_LOG_ARRAY_FORMAT_HEX:
            for (int i = 0; i < sectionCnt; i ++)
            {
                strncat(echo_str, prefix, sizeof(echo_str)-1);    // add prefix
                if((__module._big_endian && UV_LOG_ARRAY_PRINT_BIG_ENDIAN) || (!__module._big_endian && (!UV_LOG_ARRAY_PRINT_BIG_ENDIAN))){
                    for (int j = 0; j < sectionByteNum; j++){
                        snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                        strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
                    }
                }
                else{
                    for (int j = sectionByteNum-1; j >= 0; j--){
                        snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                        strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
                    }
                }
                strncat(echo_str, singleSection, sizeof(echo_str)-1);
                memset(singleSection, 0, sizeof(singleSection));
                if(i != sectionCnt - 1)
                    strncat(echo_str, space_character, sizeof(echo_str)-1);    // add space_character
                else{
                    strncat(echo_str, "\n", sizeof(echo_str)-1);
                }
            }


            break;
        case UV_LOG_ARRAY_FORMAT_DEC:
            strncat(echo_str, "UV_LOG_ARRAY_FORMAT_DEC not implemente yet\n", sizeof(echo_str)-1);


            break;
        case UV_LOG_ARRAY_FORMAT_BIN:
            strncat(echo_str, "UV_LOG_ARRAY_FORMAT_BIN not implemente yet\n", sizeof(echo_str)-1);




            break;
        default:
            break;
    }


#if UV_LOG_MONOSPACE_FUNCNAME_ENABLE
    #if UV_LOG_MONOSPACE_FUNCNAME_ALIGN_LEFT
        snprintf(log_head_str, sizeof(log_head_str), FUNCNAME_ALIGN_LEFT_FORMAT_STRING, type, uv_log_get_specific_length_string(funcname, log_func_name_temp_buf, UV_LOG_MONOSPACE_FUNCNAME_LENGTH));
    #else
        snprintf(log_head_str, sizeof(log_head_str), FUNCNAME_ALIGN_RIGHT_FORMAT_STRING, type, uv_log_get_specific_length_string(funcname, log_func_name_temp_buf, UV_LOG_MONOSPACE_FUNCNAME_LENGTH));
    #endif
#else
    snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%s]| ", type, funcname);
#endif
#if UV_LOG_TIMESTAMP_ENABLE
    __module.print_hal_cb(_get_timestamp_string());
#endif 
    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);


#if UV_LOG_SYS_AUTO_FFLUSH_CACHE
    fflush(stdout);
#endif

    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }
}



/**
 * @brief 格式化指定长度的字符串，如果长度大于len，则返回的字符串中间将使用...来代替
 * 
 * @param src 
 * @param dest 
 * @param len string length, not sizeof
 * @return char* 
 */
char *uv_log_get_specific_length_string(char *src, char *dest, int len)
{
    if(__module._intialized != 1)
        return;
    // static char buf[100];
    memset(dest, 0, len + 1);
    int src_len = strlen(src);
    if(src_len <= len)
    {
        memcpy(dest, src, len);
        return dest;
    }
    else{
        int actual_str_len = len - 3;
        int front_str_len = (actual_str_len + 1) / 2;
        int tail_str_len = actual_str_len / 2;
        memcpy(dest, src, front_str_len);
        memcpy(dest + front_str_len, "...", 3);
        memcpy(dest + front_str_len + 3, src + src_len - tail_str_len, tail_str_len);
        return dest;
    }
}



#if UV_LOG_API_ENABLE


/**
 * @brief this log api will only write log to file
 * @param dpid
 * @param level  
    - UVL_DEBUG
    - UVL_INFO 
    - UVL_WARN 
    - UVL_ERROR
    - UVL_USER 
 * @param log
*/
void uv_log(int level, const char *fmt, ...)
{
    int ret = 0;
    if(__module._intialized != 1)
        return;
    // static char log_head_str[256];
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }	

    if (level < UV_LOG_GLOBAL_LEVEL){
        return;
    }

    
    const char *levelS;
    memset(s_temp_buf, 0, sizeof(s_temp_buf));
    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);
    switch (level)
    {
    case UVL_INFO:
        levelS = "Info ";
        break;
    case UVL_DEBUG:
        levelS = "Debug";
        break;
    case UVL_USER:
        levelS = "User ";
        break;
    case UVL_ERROR:
        levelS = "Error";
        break;
    case UVL_WARN:
        levelS = "Warn ";
        break;
    
    default:
        levelS = "-----";
        break;
    }
    // printf("log_head_str: %s\n", log_head_str);
    // printf("echo_str: %s\n", echo_str);

    snprintf(log_head_str, UV_LOG_HEAD_MAX_LEN, "[%s]| ", levelS);
    snprintf(s_temp_buf, UV_LOG_MAX_LEN, "%s%s%s", _get_timestamp_string(), log_head_str, echo_str);
    // printf("1\n");
    /* output to console */
#if UV_LOG_API_OUTTO_CONSOLE

    __module.print_hal_cb(s_temp_buf);
    
    #if UV_LOG_SYS_AUTO_FFLUSH_CACHE
        fflush(stdout);
    #endif


#endif


	if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }


}
void uv_log_forMacro(const char* funcname, int line, int level, const char *fmt, ...)
{
    if(__module._intialized != 1)
        return;
    // static char log_head_str[256];
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }	

    if (level < UV_LOG_GLOBAL_LEVEL){
        return;
    }



    const char *levelS;
    memset(s_temp_buf, 0, sizeof(s_temp_buf));
    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, fmt, args);
    va_end(args);
    switch (level)
    {
    case UVL_INFO:
        levelS = "Info ";
        break;
    case UVL_DEBUG:
        levelS = "Debug";
        break;
    case UVL_USER:
        levelS = "User ";
        break;
    case UVL_ERROR:
        levelS = "Error";
        break;
    case UVL_WARN:
        levelS = "Warn ";
        break;
    
    default:
        levelS = "-----";
        break;
    }
    snprintf(log_head_str, UV_LOG_HEAD_MAX_LEN, "[%s][@%-20.20s:%-4d]| ", 
                levelS, funcname, line);
    snprintf(s_temp_buf, UV_LOG_MAX_LEN, "%s%s%s", _get_timestamp_string(), log_head_str, echo_str);

    /* output to console */
#if UV_LOG_API_OUTTO_CONSOLE

    __module.print_hal_cb(s_temp_buf);
    
    #if UV_LOG_SYS_AUTO_FFLUSH_CACHE
        fflush(stdout);
    #endif


#endif

	if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }
}

#endif //！ UV_LOG_API_ENABLE



#ifdef USE_UEGUI_LIB
/**
 * Add a log
 * @param level the level of log. (From `uv_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void _uv_log_add(char* type, const char * file, int line, const char * funcname, const char * format, ...)
{
    if(__module._intialized != 1)
        return;
    // static char log_head_str[256];
    if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, format);
    vsnprintf(echo_str, UV_LOG_MAX_LEN, format, args);
    va_end(args);

#if UV_LOG_MONOSPACE_FUNCNAME_ENABLE
    #if UV_LOG_MONOSPACE_FUNCNAME_ALIGN_LEFT
        snprintf(log_head_str, sizeof(log_head_str), FUNCNAME_ALIGN_LEFT_FORMAT_STRING, type, uv_log_get_specific_length_string(funcname, log_func_name_temp_buf, UV_LOG_MONOSPACE_FUNCNAME_LENGTH));
    #else
        snprintf(log_head_str, sizeof(log_head_str), FUNCNAME_ALIGN_RIGHT_FORMAT_STRING, type, uv_log_get_specific_length_string(funcname, log_func_name_temp_buf, UV_LOG_MONOSPACE_FUNCNAME_LENGTH));
    #endif

#else
    snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%s]| ", type, funcname);
#endif

    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);


	if(__module.mutexWay != UV_LOG_MUTEXWAY_NONE){
        __module.unlockMutex();
    }

}

#endif


#if 0
// __attribute__((weak)) inline void uv_log_hal_print(const char* str){
void uv_log_hal_print(char* str){
    /* HAL Lib sample */
    // uint32_t cnt = 0;
    // while (str[cnt++] != '\0');
    // HAL_UART_Transmit(&UART1_Handler, str, cnt-1, 500);

    /* STD Lib sample */
    // UartSendStr(USART1, str);

    /* N716 Module */
    // nwy_ext_echo("%s", str);

	int size = strlen((char *)str);

	int i = 0;
	while(1)
    {
    	int tx_size;
		
    	tx_size = nwy_usb_serial_send((char *)str + i, size - i);
		if(tx_size <= 0)
			break;
		i += tx_size;
		if((i < size))
			nwy_sleep(10);
		else
			break;
	}

}

#endif


/**
 * @brief A simple log output test.
 * 
 */
void uv_log_test()
{
    /* init in main */
    // SystemClock_Config();
    // UART1_Interrupt_Init();
    // GPIO_LedInit();
    // GPIO_KeyInit();
    // uv_log_mutex_init();
    // uv_log_init(UV_LOG_MUTEXWAY_EXTERNEL, uv_log_hal_print, uvLogLock, uvLogUnlock);
    static uint8_t arrayA[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 55, 255};
    static uint32_t arrayB[] = {0xAA001233, 0xBB11FFEE};
    UVDEBUG_BARE("Debug bare implementation\r\n");
    UVINFO_BARE(" Info bare implementation\r\n");
    UVWARNING_BARE(" Warn bare implementation\r\n");
    UVERROR_BARE("Error bare implementation\r\n");
    UVUSER_BARE(" User bare implementation\r\n");

    UVCRBR();

    UVDEBUG("Log Debug implementation\r\n");
    UVINFO("Log  Info implementation\r\n");
    UVWARNING("Log Warning implementation\r\n");
    UVERROR("Log Error implementation\r\n");
    UVUSER("Log  User implementation\r\n");

    UVCRBR();

    UVDEBUG_F("Log full Debug implementation\r\n");
    UVINFO_F("Log full  Info implementation\r\n");
    UVWARNING_F("Log full  Warn implementation\r\n");
    UVERROR_F("Log full Error implementation\r\n");
    UVUSER_F("Log full  User implementation\r\n");
    
    UVCRBR();
    
    UVDEBUG_ARRAY_HEX(arrayA, 13, 1, " ", "0x", "Debug ArrayA: ");
    UVINFO_ARRAY_HEX(arrayA, 13, 1, " ", "0x", " Info ArrayA: ");
    UVWARNING_ARRAY_HEX(arrayA, 13, 1, " ", "0x", " Warn ArrayA: ");
    UVERROR_ARRAY_HEX(arrayA, 13, 1, " ", "0x", "Error ArrayA: ");
    UVUSER_ARRAY_HEX(arrayA, 13, 1, " ", "0x", " User ArrayA: ");
    UVINFO_ARRAY_HEX(arrayB, 8, 4, " ", "0x", " Info arrayB: ");
    UVWARNING_ARRAY_HEX(arrayB, 8, 4, " ", "0x", " Warn arrayB: ");
    UVDEBUG_ARRAY_HEX(arrayB, 8, 4, " ", "0x", "Debug arrayB: ");
    UVERROR_ARRAY_HEX(arrayB, 8, 4, " ", "0x", "Error arrayB: ");
    UVUSER_ARRAY_HEX(arrayB, 8, 4, " ", "0x", " User arrayB: ");

    UVCRBR();

    UVPRINT("\r\n######################### OVER #############################\r\n");

}








/************************
 *   Utility funcs
 ************************/

int uvGetArrayHexStr(char *dest, size_t size, 
                    uint8_t *array, uint32_t array_len, uint8_t sectionByteNum,
                    const char *space_character, const char *prefix)
{
    int tail = 0;
    if(dest == NULL)
        return -1;

    memset(dest, 0, size);

    char singleSection[16 + 3];
    char singleSection_temp[16 + 3];
    uint8_t *p_array = NULL;
    uint32_t alignArrayLen = 0;
    uint8_t byte_alignment_cnt;

    // align array
    if(array_len % sectionByteNum == 0){
        p_array = array;
        alignArrayLen = array_len;
    }
    else{
        uint8_t*  tempArray = NULL;

        // 对齐需要的字节数
        byte_alignment_cnt = sectionByteNum - (array_len % sectionByteNum);
        if(array_len + byte_alignment_cnt > UV_LOG_ARRAY_MAX_SIZE){
            array_len = UV_LOG_ARRAY_MAX_SIZE - byte_alignment_cnt;
        }

        tempArray = malloc(array_len);
        if(tempArray == NULL){
            return -2;
        }
        memset(tempArray, 0, array_len);

        memcpy(tempArray + byte_alignment_cnt, array, array_len);   // alignment array
        p_array = tempArray;
        alignArrayLen = array_len + byte_alignment_cnt;
    }

    uint32_t sectionCnt = alignArrayLen / sectionByteNum;

    memset(singleSection_temp, 0, sizeof(singleSection_temp));
    memset(singleSection, 0, sizeof(singleSection));
    for (int i = 0; i < sectionCnt; i ++)
    {
        // strncpy(dest + tail, prefix, size - 1);
        strncat(dest, prefix, size-1);    // add prefix
        if((__module._big_endian && UV_LOG_ARRAY_PRINT_BIG_ENDIAN) || (!__module._big_endian && (!UV_LOG_ARRAY_PRINT_BIG_ENDIAN))){
            for (int j = 0; j < sectionByteNum; j++){
                snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
            }
        }
        else{
            for (int j = sectionByteNum-1; j >= 0; j--){
                snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
            }
        }

        strncat(dest, singleSection, size-1);
        memset(singleSection, 0, sizeof(singleSection));
        if(i != sectionCnt - 1)
            strncat(dest, space_character, size-1);    // add space_character
        else{
            // strncat(dest, "\r\n", size-1);
        }
    }

    return 0;
}


/**
 * @return 返回实际转换的字节数
*/
int uvStrToHex(const char* str, uint8_t* dest, size_t size) 
{
    int i = 0;
    int len = strlen(str);
    int ret = 0;
    if(len % 2 != 0){
        // ret = 1;
        len--; // 舍弃最后一个奇数位字符
    }
    
    if (len / 2 > size){
        // ret = 2;
        len = size * 2; // 超出最大长度则截断
    }
    
    for (i = 0; i < len; i += 2) {
        sscanf(str + i, "%2hhx", &dest[i / 2]);
        ret++;
    }

    return ret;
}