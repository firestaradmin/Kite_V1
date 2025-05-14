/**
 * @file mybeep.h
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


#ifndef __MYBEEP_H__
#define __MYBEEP_H__

#include "stdint.h"

#define TONE_N      0
#define TONE_DO_D   1
#define TONE_RI_D   2
#define TONE_MI_D   3
#define TONE_FA_D   4
#define TONE_SO_D   5
#define TONE_LA_D   6
#define TONE_XI_D   7
#define TONE_DO     (TONE_XI_D+1)
#define TONE_RI     (TONE_XI_D+2)
#define TONE_MI     (TONE_XI_D+3)
#define TONE_FA     (TONE_XI_D+4)
#define TONE_SO     (TONE_XI_D+5)
#define TONE_LA     (TONE_XI_D+6)
#define TONE_XI     (TONE_XI_D+7)
#define TONE_DO_U   (TONE_XI+1)
#define TONE_RI_U   (TONE_XI+2)
#define TONE_MI_U   (TONE_XI+3)
#define TONE_FA_U   (TONE_XI+4)
#define TONE_SO_U   (TONE_XI+5)
#define TONE_LA_U   (TONE_XI+6)
#define TONE_XI_U   (TONE_XI+7)

#define TONE_0   0
#define TONE_1D 1
#define TONE_2D 2
#define TONE_3D 3
#define TONE_4D 4
#define TONE_5D 5
#define TONE_6D 6
#define TONE_7D 7
#define TONE_1 (TONE_XI_D+1)
#define TONE_2 (TONE_XI_D+2)
#define TONE_3 (TONE_XI_D+3)
#define TONE_4 (TONE_XI_D+4)
#define TONE_5 (TONE_XI_D+5)
#define TONE_6 (TONE_XI_D+6)
#define TONE_7 (TONE_XI_D+7)
#define TONE_1U (TONE_XI+1)
#define TONE_2U (TONE_XI+2)
#define TONE_3U (TONE_XI+3)
#define TONE_4U (TONE_XI+4)
#define TONE_5U (TONE_XI+5)
#define TONE_6U (TONE_XI+6)
#define TONE_7U (TONE_XI+7)

typedef enum tone_selection {
    k_tone_a = 0,
    k_tone_b,
    k_tone_c,
    k_tone_d,
    k_tone_e,
    k_tone_f,
    k_tone_g,

} tone_selection_t;

#define BEAT_FULL       1
#define BEAT_HALF       2
#define BEAT_QUARTER    3

void my_beep_init(int io_num);
void my_beep_setfreq(uint32_t freq);
void my_beep_setduty(uint32_t duty);    // 影响音量
void my_beep_setduty50(void);
void my_beep_setduty0(void);

void my_beep_play_song(tone_selection_t tone_sel,
                       uint16_t *melody,
                       uint8_t *rhythm,
                       uint16_t beat_cnt,
                       uint16_t beat_per_min);

void my_beep_test_happybirthday(void);
void my_beep_test_QingTian(void);

#endif // !__MYBEEP_H__
