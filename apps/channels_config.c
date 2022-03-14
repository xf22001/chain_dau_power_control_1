

/*================================================================
 *
 *
 *   文件名称：channels_config.c
 *   创 建 者：肖飞
 *   创建日期：2020年06月18日 星期四 09时17分57秒
 *   修改日期：2022年03月14日 星期一 15时14分47秒
 *   描    述：
 *
 *================================================================*/
#include "channels_config.h"

#include "main.h"
#include "os_utils.h"
#include "power_modules.h"

#include "log.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;

char *get_channel_config_channel_type_des(channel_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(CHANNEL_TYPE_NONE);
			add_des_case(CHANNEL_TYPE_NATIVE);
			add_des_case(CHANNEL_TYPE_PROXY_REMOTE);
			add_des_case(CHANNEL_TYPE_PROXY_LOCAL);

		default: {
		}
		break;
	}

	return des;
}

char *get_power_manager_type_des(power_manager_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(POWER_MANAGER_TYPE_NONE);
			add_des_case(POWER_MANAGER_TYPE_NATIVE);

		default: {
		}
		break;
	}

	return des;
}

static channels_config_t channels_config_0 = {
	.id = 0,
	.channel_number = 0,
	.channel_config = NULL,
	.power_module_config = {
		.power_module_default_type = POWER_MODULE_TYPE_WINLINE,
		.hcan = &hcan2,
	},
	.power_manager_config = {
		.power_manager_default_type = POWER_MANAGER_TYPE_NATIVE,
	},
	.display_config = {
		.huart = &huart2,
		.con_gpio = con485_GPIO_Port,
		.con_pin = con485_Pin,
		.led_gpio = led485_GPIO_Port,
		.led_pin = led485_Pin,
		.station = 1,
	},
	.proxy_channel_info = {
		.hcan = &hcan1,
	},

	//.force_stop_port = in2_GPIO_Port,
	//.force_stop_pin = in2_Pin,
	//.force_stop_normal_state = GPIO_PIN_RESET,

	//.fan_port = rey5_GPIO_Port,
	//.fan_pin = rey5_Pin,

	.door_port = in1_GPIO_Port,
	.door_pin = in1_Pin,
	.door_normal_state = GPIO_PIN_RESET,
};

static channels_config_t *channels_config_sz[] = {
	&channels_config_0,
};

channels_config_t *get_channels_config(uint8_t id)
{
	int i;
	channels_config_t *channels_config = NULL;
	channels_config_t *channels_config_item = NULL;

	for(i = 0; i < ARRAY_SIZE(channels_config_sz); i++) {
		channels_config_item = channels_config_sz[i];

		if(channels_config_item->id == id) {
			channels_config = channels_config_item;
			break;
		}
	}

	return channels_config;
}

proxy_channel_item_t *get_proxy_channel_item_by_proxy_channel_index(proxy_channel_info_t *proxy_channel_info, uint8_t proxy_channel_index)
{
	proxy_channel_item_t *item = NULL;
	int i;

	if(proxy_channel_info->proxy_channel_number == 0) {
		return item;
	}

	if(proxy_channel_info->items == NULL) {
		return item;
	}

	for(i = 0; i < proxy_channel_info->proxy_channel_number; i++) {
		proxy_channel_item_t *_item = proxy_channel_info->items + i;

		if(_item->proxy_channel_index == proxy_channel_index) {
			item = _item;
			break;
		}
	}

	return item;
}

proxy_channel_item_t *get_proxy_channel_item_by_channel_id(proxy_channel_info_t *proxy_channel_info, uint8_t channel_id)
{
	proxy_channel_item_t *item = NULL;
	int i;

	if(proxy_channel_info->proxy_channel_number == 0) {
		return item;
	}

	if(proxy_channel_info->items == NULL) {
		return item;
	}

	for(i = 0; i < proxy_channel_info->proxy_channel_number; i++) {
		proxy_channel_item_t *_item = proxy_channel_info->items + i;

		if(_item->channel_id == channel_id) {
			item = _item;
			break;
		}
	}

	return item;
}
