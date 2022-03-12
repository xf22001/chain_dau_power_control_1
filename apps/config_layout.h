

/*================================================================
 *   
 *   
 *   文件名称：config_layout.h
 *   创 建 者：肖飞
 *   创建日期：2021年08月25日 星期三 08时24分46秒
 *   修改日期：2021年08月25日 星期三 11时38分24秒
 *   描    述：
 *
 *================================================================*/
#ifndef _CONFIG_LAYOUT_H
#define _CONFIG_LAYOUT_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#include "app_platform.h"
#include "cmsis_os.h"

#include "app.h"
#include "config_utils.h"
#include "channels.h"

#pragma pack(push, 1)

typedef struct {
	config_item_head_t head;
	mechine_info_t mechine_info;
} storage_mechine_info_t;

typedef struct {
	config_item_head_t head;
	channels_settings_t channels_settings;
} storage_channels_settings_t;

typedef struct {
	union {
		storage_mechine_info_t storage_mechine_info;
		uint8_t seg[512];
	} mechine_info_seg;
	union {
			storage_channels_settings_t storage_channels_settings;
		uint8_t seg[1024];
	} channels_settings_seg;
} config_layout_t;

#pragma pack(pop)

static inline config_layout_t *get_config_layout(void)
{
	return (config_layout_t *)0;
}

#endif //_CONFIG_LAYOUT_H
