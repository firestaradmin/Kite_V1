/**
 * @file mybeep.c
 * @author @firestaradmin
 * @brief 
 * @version v0.0.1
 * @date 2024/05/20
 * 
 * @copyright Copyright (c) 2024 LXG 
 * 
 * @history:
 *  - 2024/05/20: v0.0.1 Initial Version
 *  - ----/--/--: --
 */

#include "mybeep.h"



#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
static const char* TAG = "myBeep";

// #define _PIN_IO_BEEP 48


#define LEDC_TIMER                      LEDC_TIMER_0
#define LEDC_MODE                       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL                    LEDC_CHANNEL_0
#define LEDC_DUTY_RES                   LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DEFAULT_FREQUENCY          (500) // Frequency in Hertz.

#define LEDC_DUTY_50               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096

const uint16_t _tone_a_freq_map[] = {
    0,  // none
    221,248,278,294,330,371,416,    // down 降调
    441,495,556,589,661,742,833,    // normal 
    882,990,1112,1178,1322,1484,1665,    // up 升调
    };
const uint16_t _tone_b_freq_map[] = {
    0,  // none
    248,278,294,330,371,416,467,    // down 降调
    495,556,624,661,742,833,935,    // normal 
    990,1112,1178,1322,1484,1665,1869,    // up 升调
    };
const uint16_t _tone_c_freq_map[] = {
    0,  // none
    131,147,165,175,196,221,248,    // down 降调
    262,294,330,350,393,441,495,    // normal 
    525,589,661,700,786,882,990,    // up 升调
    };
const uint16_t _tone_d_freq_map[] = {
    0,  // none
    147,105,175,196,221,248,278,    // down 降调
    294,330,350,393,441,495,556,    // normal 
    589,661,700,786,882,990,1112,    // up 升调
    };
const uint16_t _tone_e_freq_map[] = {
    0,  // none
    165,175,196,221,248,278,312,    // down 降调
    330,350,393,441,495,556,624,    // normal 
    661,700,786,882,990,1112,1248,    // up 升调
    };
const uint16_t _tone_f_freq_map[] = {
    0,  // none
    175,196,221,234,262,294,330,    // down 降调
    350,393,441,495,556,624,661,    // normal 
    700,786,882,935,1049,1178,1322,    // up 升调
    };
const uint16_t _tone_g_freq_map[] = {
    0,  // none
    196,221,234,262,294,330,371,    // down 降调
    393,441,495,556,624,661,742,    // normal 
    786,882,990,1049,1178,1322,1434,    // up 升调
    };

void my_beep_init(int io_num)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_DEFAULT_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = io_num,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    my_beep_setduty0();
    ledc_timer_pause(LEDC_MODE, LEDC_TIMER );
}

void my_beep_setduty(uint32_t duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void my_beep_setduty0(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
void my_beep_setduty50(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_50));
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void my_beep_setfreq(uint32_t freq)
{
    ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq));
}



/**
 * @brief 演奏歌曲
 * @param tone_sel 选调
 * @param melody 乐曲音调
 * @param rhythm 乐曲节奏，数组中数据的最高位为1代表本节拍与下一拍连音，无间隔转变
 * @param beat_cnt 节奏数 
 * @param beat_per_min 每分钟节拍数 
*/
void my_beep_play_song( tone_selection_t tone_sel,
                        uint16_t *melody,
                        uint8_t *rhythm,
                        uint16_t beat_cnt,
                        uint16_t beat_per_min)
{
    uint16_t _time = 0;
    uint8_t _if_continues = 0;
    const uint16_t *_tone_sel = _tone_c_freq_map;
    uint16_t beat_time = (60 * 1000) / beat_per_min;
    // ESP_LOGI(TAG, "beat_per_min:%u, beat_time:%u", beat_per_min, beat_time);

    switch (tone_sel)
    {
    case k_tone_a:
        _tone_sel = _tone_a_freq_map;
        break;
    case k_tone_b:
        _tone_sel = _tone_b_freq_map;
        break;
    case k_tone_c:
        _tone_sel = _tone_c_freq_map;
        // ESP_LOGI(TAG, "choose c");
        break;
    case k_tone_d:
        _tone_sel = _tone_d_freq_map;
        break;
    case k_tone_e:
        _tone_sel = _tone_e_freq_map;
        break;
    case k_tone_f:
        _tone_sel = _tone_f_freq_map;
        break;
    case k_tone_g:
        _tone_sel = _tone_g_freq_map;
        break;
    
    default:
        _tone_sel = _tone_c_freq_map;
        break;
    }


    for (int i = 0; i < beat_cnt; i++){
        if (melody[i] == TONE_0){
            if ((uint8_t)(rhythm[i] & 0x7F) == (uint8_t)BEAT_FULL)
                _time = beat_time;
            else if((uint8_t)(rhythm[i] & 0x7F) == (uint8_t)BEAT_HALF)
                _time = beat_time/2;
            else if((uint8_t)(rhythm[i] & 0x7F) == (uint8_t)BEAT_QUARTER)
                _time = beat_time/4;

            my_beep_setduty0();
            vTaskDelay(_time / portTICK_PERIOD_MS);
        }
        else{
            my_beep_setfreq(_tone_sel[melody[i]]);

            // my_beep_setduty50();
            my_beep_setduty(10);
            if ((rhythm[i] & 0x80) > 0)
                _if_continues = 1;
            else
                _if_continues = 0;

            // ESP_LOGI(TAG, "rhythm[i]:%u", rhythm[i] & 0x7F);
            if ((uint8_t)(rhythm[i] & 0x7F) == (uint8_t)BEAT_FULL)
                _time = beat_time;
            else if((uint8_t)(rhythm[i] & 0x7F) == (uint8_t)BEAT_HALF)
                _time = beat_time/2;
            else if((uint8_t)(rhythm[i] & 0x7F) == (uint8_t)BEAT_QUARTER)
                _time = beat_time/4;

            // ESP_LOGI(TAG, "play freq:%u", _tone_sel[melody[i]]);
            // ESP_LOGI(TAG, "play time:%u", _time);

            if (_if_continues == 0){
                vTaskDelay((_time - 50) / portTICK_PERIOD_MS);
                my_beep_setduty0();
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }
            else{
                vTaskDelay(_time/ portTICK_PERIOD_MS);
            }
        }

    }
}


const uint16_t _song_happybirthday_melody[] = {
    TONE_0,TONE_0,TONE_5,TONE_5,
    TONE_6,TONE_5,TONE_1U,
    TONE_7,TONE_0,TONE_5,TONE_5,
    TONE_6,TONE_5,TONE_2U,
    TONE_1U,TONE_0,TONE_5,TONE_5,
    TONE_5U,TONE_3U,TONE_1U,
    TONE_7,TONE_6,TONE_0,
    TONE_0,TONE_0,TONE_4U,TONE_4U,
    TONE_3U,TONE_1U,TONE_2U,
    TONE_1U,TONE_0

};

/* 
    定义节奏型
    最高位为1代表本节拍与下一拍连音，无间隔转变
*/
const uint8_t _song_happybirthday_rhythm[] = {
    BEAT_FULL, BEAT_FULL, BEAT_HALF, BEAT_HALF, 
    BEAT_FULL, BEAT_FULL, BEAT_FULL,
    BEAT_FULL, BEAT_FULL, BEAT_HALF, BEAT_HALF,
    BEAT_FULL, BEAT_FULL, BEAT_FULL,
    BEAT_FULL, BEAT_FULL, BEAT_HALF, BEAT_HALF,
    BEAT_FULL, BEAT_FULL, BEAT_FULL,
    BEAT_FULL|0x80, BEAT_FULL, BEAT_FULL,
    BEAT_FULL, BEAT_FULL, BEAT_HALF, BEAT_HALF,
    BEAT_FULL, BEAT_FULL, BEAT_FULL,
    BEAT_FULL, BEAT_FULL

};


const uint16_t _song_qingtian_melody[] = {
    TONE_6D,TONE_1,TONE_5,TONE_1,TONE_4D,TONE_5D,TONE_6D,TONE_5,TONE_1,
    TONE_1D,TONE_5D,TONE_5,TONE_1,TONE_1D,TONE_5,TONE_7D,TONE_5,
    TONE_6D,TONE_1,TONE_5,TONE_6D,TONE_4D,TONE_5D,TONE_6D,TONE_5,TONE_1,
    TONE_1D,TONE_5D,TONE_5,TONE_1,TONE_1D,TONE_5,TONE_7D,TONE_1,TONE_5,
    TONE_6D,TONE_1,TONE_5,TONE_1,TONE_4D,TONE_5D,TONE_6D,TONE_5,TONE_1,
    TONE_1D,TONE_5D,TONE_5,TONE_1,TONE_1D,TONE_5,TONE_7D,TONE_5,
    TONE_6D,TONE_1,TONE_5,TONE_6D,TONE_4D,TONE_5D,TONE_6D,TONE_5,TONE_1,
    TONE_1D,TONE_5D,TONE_5,TONE_1,TONE_1D,TONE_5,TONE_7D,TONE_1,TONE_5,

};

/* 
    定义节奏型，时间
    最高位为1代表本节拍与下一拍连音，无间隔转变
*/
const uint8_t _song_qingtian_rhythm[] = {
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_QUARTER,BEAT_QUARTER,BEAT_HALF,BEAT_HALF,
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_QUARTER,BEAT_QUARTER,BEAT_HALF,BEAT_HALF,
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_QUARTER,BEAT_QUARTER,BEAT_HALF,
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_QUARTER,BEAT_QUARTER,BEAT_HALF,BEAT_HALF,
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_QUARTER,BEAT_QUARTER,BEAT_HALF,BEAT_HALF,
    BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_HALF,BEAT_QUARTER,BEAT_QUARTER,BEAT_HALF,
};

void my_beep_test_happybirthday(void)
{
    my_beep_play_song(k_tone_c,_song_happybirthday_melody,_song_happybirthday_rhythm,
                    sizeof(_song_happybirthday_melody)/sizeof(_song_happybirthday_melody[0]),90);

}

void my_beep_test_QingTian(void)
{

    my_beep_play_song(k_tone_g,_song_qingtian_melody,_song_qingtian_rhythm,
                    sizeof(_song_qingtian_melody)/sizeof(_song_qingtian_melody[0]),60);
}

