

/**
 * @file mlog.h
 * @author @firestaradmin
 * @brief What is rational is actual and what is actual is rational.
 * @version v0.0.1
 * @date 2025/05/14
 * 
 * @copyright Copyright (c) 2025 LXG 
 * 
 * @history:
 *  - 2025/05/14: v0.0.1 Initial Version
 *  - ----/--/--: --
 */


#ifndef __MLOG__
#define __MLOG__
#include "esp_log.h"


#define MLOGI(format,...) ESP_LOGI(__func__, format, ## __VA_ARGS__)
#define MLOGW(format,...) ESP_LOGW(__func__, format, ## __VA_ARGS__)
#define MLOGE(format,...) ESP_LOGE(__func__, format, ## __VA_ARGS__)

#endif // !__MLOG__
