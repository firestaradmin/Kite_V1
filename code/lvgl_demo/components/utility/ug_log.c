/*! @file ug_log.c
 *  @version 0.0.2
 *  @date May 12 2021
 *  @author firestaradmin
 *
 *  @brief
 *  Print and out log info.
 *  
 *  @log 
 * 2022-11-15: Version 0.0.4 port: for UEGUI
 * 2022-5-12: Version 0.0.3 feature: Modular modules make it more versatile  
 * 2022-3-29: Version 0.0.2
 */


/* C libs */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* my libs */
#include "ug_log.h"

/*************************************************** usage example ***************************************************/
// SemaphoreHandle_t ugLog_mutex;

// void ugLogLock()
// {
//     xSemaphoreTake(ugLog_mutex, portMAX_DELAY );
    
// }
// void ugLogUnlock()
// {
//     xSemaphoreGive(ugLog_mutex);
// }

// void ug_log_mutex_init( )
// {
//     ugLog_mutex = xSemaphoreCreateMutex();
//     if( ugLog_mutex == NULL )
//     {
//         printf("ug log mutex err!\r\n");
//     }
// }
// void app_main(void)
// {
// 	   printf("Kite_V1 core board demo!\n");
//     ug_log_mutex_init();
//     ug_log_init(ug_log_mutexWay_externel, printf, ugLogLock, ugLogUnlock);
//     UGINFO("hello world!\r\n");
// }
/************************************************* example - END ***********************************************/

// 使用等宽函数名称输出 看看能否帮助更好的观看log
// example: [time] [debug][@my_example_func      ]| hello world!
// example: [time] [debug][@my_example_a...s_func]| hello world!
// [12:07:25.467] [debug] [@my_main_task        ]| hello world! 
// [12:07:25.467] [debug] [@my_main_task        ]| hello world! 

// 上报云平台
#define LOG_REPORT_ENABLE 0

// TODO
// 在MobaXterm里串口如果直接输出\n 只是换行，每次要手动设置LF很麻烦， MobaXterm 回车换行支持
// #define MobaXterm_LF_MACRO 1 

// var 
static char echo_str[UG_LOG_MAX_LEN];
static char log_head_str[UG_LOG_HEAD_MAX_LEN];
static uint8_t tempArray[UG_LOG_ARRAY_MAX_SIZE];
static char log_func_name_temp_buf[26];

#if LOG_REPORT_ENABLE
#include "report_log.h"
static char log_report_str[UG_LOG_MAX_LEN];
#endif

static ug_log_module_t __module = {0};
// static char __internel_virtual_mutex = 0;



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
                void (*unlockMutex)(void) )
{
    // TODO: 判断大小端，以支持数组打印多余1byte的字段

    if(print_hal_cb == NULL)
        return -1;
    __module.print_hal_cb = print_hal_cb;
    __module.mutexWay = mutexWay;
    if(__module.mutexWay == ug_log_mutexWay_internel){
        // TODO internel mutex
        __module.lockMutex = NULL;
        __module.unlockMutex = NULL;
    }
    else if(__module.mutexWay == ug_log_mutexWay_externel){
        __module.lockMutex = lockMutex;
        __module.unlockMutex = unlockMutex;
    }
    else{
        __module.lockMutex = NULL;
        __module.unlockMutex = NULL;
    }
    __module.intialized = 1;

    return 0;
}




void printS(const char *fmt, ...)
{
    if(__module.intialized != 1)
        return;
    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UG_LOG_MAX_LEN, fmt, args);
    va_end(args);

    // ug_log_hal_print(echo_str);
    __module.print_hal_cb(echo_str);

    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.unlockMutex();
    }

}

void printLogS(const char* funcname,const char* type, const char *fmt, ...)
{
    if(__module.intialized != 1)
        return;
    // static char log_head_str[256];
    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UG_LOG_MAX_LEN, fmt, args);
    va_end(args);

#if UG_LOG_FUNCNAME_LEN_SPECIFY
    #if UG_LOG_FUNCNAME_LEN_SPECIFY_ALIGN_LEFT
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%-25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #else
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #endif

#else
    snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%s]| ", type, funcname);
#endif

    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);

#if LOG_REPORT_ENABLE
    if(strstr(type, "error")){
        memset(log_report_str, 0, sizeof(log_report_str));
        strncat(log_report_str, log_head_str, sizeof(log_report_str)-1);
        strncat(log_report_str, echo_str, sizeof(log_report_str)-1);
        my_log_report_insert_log_to_report(log_report_str);
    }
#endif

	if(__module.mutexWay != ug_log_mutexWay_none){
        __module.unlockMutex();
    }

}


void printLogS_noInsert(const char* funcname,const char* type, const char *fmt, ...)
{
    if(__module.intialized != 1)
        return;
    // static char log_head_str[256];
    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UG_LOG_MAX_LEN, fmt, args);
    va_end(args);

#if UG_LOG_FUNCNAME_LEN_SPECIFY
    #if UG_LOG_FUNCNAME_LEN_SPECIFY_ALIGN_LEFT
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%-25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #else
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #endif

#else
    snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%s]| ", type, funcname);
#endif

    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);


	if(__module.mutexWay != ug_log_mutexWay_none){
        __module.unlockMutex();
    }

}



void printLogFull(const char* file,int line,const char* funcname,const char* type, const char *fmt, ...)
{
    if(__module.intialized != 1)
        return;
    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UG_LOG_MAX_LEN, fmt, args);
    va_end(args);

    // "\n[error][main.c 221] @init():\n"
    snprintf(log_head_str, sizeof(log_head_str), "\n[** %s **][%s %d] @%s: ", type, file, line, funcname);
    
    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);
    
#if LOG_REPORT_ENABLE
    if(strstr(type, "error")){
        memset(log_report_str, 0, sizeof(log_report_str));
        strncat(log_report_str, log_head_str, sizeof(log_report_str)-1);
        strncat(log_report_str, echo_str, sizeof(log_report_str)-1);
        my_log_report_insert_log_to_report(log_report_str);
    }
#endif

	if(__module.mutexWay != ug_log_mutexWay_none){
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
void printS_array(uint8_t *array, uint32_t array_len, uint8_t sectionByteNum, char *space_character, char *prefix, uint8_t format, const char *fmt, ...)
{
    if(__module.intialized != 1)
        return;
    // static uint8_t tempArray[UG_LOG_ARRAY_MAX_SIZE];
    static char singleSection[UG_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    static char singleSection_temp[UG_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    uint8_t *p_array = NULL;
    uint32_t alignArrayLen = 0;
    uint8_t byte_alignment_cnt;
    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.lockMutex();
    }


    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UG_LOG_MAX_LEN, fmt, args);
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
        memcpy(tempArray + byte_alignment_cnt, array, array_len);   // alignment array
        p_array = tempArray;
        alignArrayLen = array_len + byte_alignment_cnt;
    }

    uint32_t sectionCnt = alignArrayLen / sectionByteNum;
    switch(format)
    {
        case UG_LOG_ARRAY_FORMAT_HEX:
            for (int i = 0; i < sectionCnt; i ++)
            {
                strncat(echo_str, prefix, sizeof(echo_str)-1);    // add prefix
                for (int j = 0; j < sectionByteNum; j++){
                    snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                    strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
                    // snprintf(singleSection, sizeof(singleSection), "%s%02X", singleSection, *(p_array + j + i * sectionByteNum));
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
        case UG_LOG_ARRAY_FORMAT_DEC:
            strncat(echo_str, "UG_LOG_ARRAY_FORMAT_DEC not implemente yet\r\n", sizeof(echo_str)-1);


            break;
        case UG_LOG_ARRAY_FORMAT_BIN:
            strncat(echo_str, "UG_LOG_ARRAY_FORMAT_BIN not implemente yet\r\n", sizeof(echo_str)-1);




            break;
        default:
            break;
    }
    __module.print_hal_cb(echo_str);

    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.unlockMutex();
    }
}


// TODO: 添加一个控制 是否输出 0占位符的 参数
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
                     const char *fmt, ...)
{
    if(__module.intialized != 1)
        return;
    // static uint8_t tempArray[UG_LOG_ARRAY_MAX_SIZE];
    static char singleSection[UG_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    static char singleSection_temp[UG_LOG_ARRAY_SINGLE_MAX_LEN + 3];
    uint8_t *p_array = NULL;
    uint32_t alignArrayLen = 0;
    uint8_t byte_alignment_cnt;
    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.lockMutex();
    }


    va_list args;
    va_start(args, fmt);
    vsnprintf(echo_str, UG_LOG_MAX_LEN, fmt, args);
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
        memcpy(tempArray + byte_alignment_cnt, array, array_len);   // alignment array
        p_array = tempArray;
        alignArrayLen = array_len + byte_alignment_cnt;
    }

    uint32_t sectionCnt = alignArrayLen / sectionByteNum;
    switch(format)
    {
        case UG_LOG_ARRAY_FORMAT_HEX:
            for (int i = 0; i < sectionCnt; i ++)
            {
                strncat(echo_str, prefix, sizeof(echo_str)-1);    // add prefix
                for (int j = 0; j < sectionByteNum; j++){
                    snprintf(singleSection_temp, sizeof(singleSection_temp), "%02X", *(p_array + j + i * sectionByteNum));
                    strncat(singleSection, singleSection_temp, sizeof(singleSection)-1);
                    // snprintf(singleSection, sizeof(singleSection), "%s%02X", singleSection, *(p_array + j + i * sectionByteNum));
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
        case UG_LOG_ARRAY_FORMAT_DEC:
            strncat(echo_str, "UG_LOG_ARRAY_FORMAT_DEC not implemente yet\n", sizeof(echo_str)-1);


            break;
        case UG_LOG_ARRAY_FORMAT_BIN:
            strncat(echo_str, "UG_LOG_ARRAY_FORMAT_BIN not implemente yet\n", sizeof(echo_str)-1);




            break;
        default:
            break;
    }


#if UG_LOG_FUNCNAME_LEN_SPECIFY
    #if UG_LOG_FUNCNAME_LEN_SPECIFY_ALIGN_LEFT
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%-25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #else
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #endif
#else
    snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%s]| ", type, funcname);
#endif

    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);

    if(__module.mutexWay != ug_log_mutexWay_none){
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
char *_get_specific_length_string(char *src, char *dest, int len)
{
    if(__module.intialized != 1)
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


#ifdef USE_UEGUI_LIB
/**
 * Add a log
 * @param level the level of log. (From `ug_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void _ug_log_add(char* type, const char * file, int line, const char * funcname, const char * format, ...)
{
    if(__module.intialized != 1)
        return;
    // static char log_head_str[256];
    if(__module.mutexWay != ug_log_mutexWay_none){
        __module.lockMutex();
    }	

    va_list args;
    va_start(args, format);
    vsnprintf(echo_str, UG_LOG_MAX_LEN, format, args);
    va_end(args);

#if UG_LOG_FUNCNAME_LEN_SPECIFY
    #if UG_LOG_FUNCNAME_LEN_SPECIFY_ALIGN_LEFT
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%-25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #else
        snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%25.25s]| ", type, _get_specific_length_string(funcname, log_func_name_temp_buf, 25));
    #endif

#else
    snprintf(log_head_str, sizeof(log_head_str), "[%s] [@%s]| ", type, funcname);
#endif

    __module.print_hal_cb(log_head_str);
    __module.print_hal_cb(echo_str);

#if LOG_REPORT_ENABLE
    if(strstr(type, "error")){
        memset(log_report_str, 0, sizeof(log_report_str));
        strncat(log_report_str, log_head_str, sizeof(log_report_str)-1);
        strncat(log_report_str, echo_str, sizeof(log_report_str)-1);
        my_log_report_insert_log_to_report(log_report_str);
    }
#endif

	if(__module.mutexWay != ug_log_mutexWay_none){
        __module.unlockMutex();
    }

}

#endif


#if 0
// __attribute__((weak)) inline void ug_log_hal_print(const char* str){
void ug_log_hal_print(char* str){
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







