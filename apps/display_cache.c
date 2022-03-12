

/*================================================================
 *
 *
 *   文件名称：display_cache.c
 *   创 建 者：肖飞
 *   创建日期：2021年07月17日 星期六 09时42分40秒
 *   修改日期：2022年02月21日 星期一 11时27分02秒
 *   描    述：
 *
 *================================================================*/
#include "display_cache.h"

#include <stdio.h>

#include "app.h"
#include "channels.h"
#include "power_modules.h"

#include "log.h"

void load_app_display_cache(app_info_t *app_info)
{
}

void sync_app_display_cache(app_info_t *app_info)
{
}

void load_channels_display_cache(channels_info_t *channels_info)
{
	switch(channels_info->channels_settings.power_module_settings.power_module_type) {
		case POWER_MODULE_TYPE_PSEUDO: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_PSEUDO;
		}
		break;

		case POWER_MODULE_TYPE_HUAWEI: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_HUAWEI;
		}
		break;

		case POWER_MODULE_TYPE_INCREASE: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_INCREASE;
		}
		break;

		case POWER_MODULE_TYPE_INFY: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_INFY;
		}
		break;

		case POWER_MODULE_TYPE_STATEGRID: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_STATEGRID;
		}
		break;

		case POWER_MODULE_TYPE_YYLN: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_YYLN;
		}
		break;

		case POWER_MODULE_TYPE_WINLINE: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_WINLINE;
		}
		break;

		case POWER_MODULE_TYPE_ZTE: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_ZTE;
		}
		break;

		default: {
			channels_info->display_cache_channels.power_module_type = DISPLAY_POWER_MODULE_TYPE_PSEUDO;
		}
		break;
	}

}

void sync_channels_display_cache(channels_info_t *channels_info)
{
	if(channels_info->display_cache_channels.power_module_type_sync != 0) {
		channels_info->display_cache_channels.power_module_type_sync = 0;

		switch(channels_info->display_cache_channels.power_module_type) {
			case DISPLAY_POWER_MODULE_TYPE_PSEUDO: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_PSEUDO;
			}
			break;

			case DISPLAY_POWER_MODULE_TYPE_HUAWEI: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_HUAWEI;
			}
			break;

			case DISPLAY_POWER_MODULE_TYPE_INCREASE: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_INCREASE;
			}
			break;

			case DISPLAY_POWER_MODULE_TYPE_INFY: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_INFY;
			}
			break;

			case DISPLAY_POWER_MODULE_TYPE_STATEGRID: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_STATEGRID;
			}
			break;

			case DISPLAY_POWER_MODULE_TYPE_YYLN: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_YYLN;
			}
			break;

			case DISPLAY_POWER_MODULE_TYPE_WINLINE: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_WINLINE;
			}
			break;

			case DISPLAY_POWER_MODULE_TYPE_ZTE: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_ZTE;
			}
			break;

			default: {
				channels_info->channels_settings.power_module_settings.power_module_type = POWER_MODULE_TYPE_PSEUDO;
			}
			break;
		}

		channels_info->channels_settings_invalid = 1;
	}
}
