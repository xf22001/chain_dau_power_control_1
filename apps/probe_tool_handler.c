

/*================================================================
 *
 *
 *   文件名称：probe_tool_handler.c
 *   创 建 者：肖飞
 *   创建日期：2020年03月20日 星期五 12时48分07秒
 *   修改日期：2022年03月11日 星期五 13时14分07秒
 *   描    述：
 *
 *================================================================*/
#include "probe_tool_handler.h"

#include <string.h>

#include "lwip/netdb.h"
#include "lwip/inet.h"

#include "flash.h"
#include "iap.h"
#include "channels.h"

#include "sal_hook.h"

#define LOG_UDP
#include "log.h"

static void fn1(request_t *request)
{
	probe_server_chunk_sendto(request->payload.fn, (void *)0x8000000, 512);
}

static void fn2(request_t *request)
{
	int ret;
	char *content = (char *)(request + 1);
	channels_config_t *channels_config = get_channels_config(0);
	channels_info_t *channels_info;
	int fn;
	int channel_id;
	int type;
	int catched;

	if(channels_config == NULL) {
		return;
	}

	channels_info = get_channels();

	if(channels_info == NULL) {
		return;
	}

	ret = sscanf(content, "%d %d %d%n", &fn, &channel_id, &type, &catched);

	if(ret == 3) {
		channel_info_t *channel_info = channels_info->channel_info + channel_id;
		channel_event_t *channel_event = os_calloc(1, sizeof(channel_event_t));
		channels_event_t *channels_event = os_calloc(1, sizeof(channels_event_t));

		debug("fn:%d, channel_id:%d, type:%d!", fn, channel_id, type);

		OS_ASSERT(channel_info != NULL);
		OS_ASSERT(channel_event != NULL);
		OS_ASSERT(channels_event != NULL);

		switch(type) {
			case CHANNEL_EVENT_TYPE_START_CHANNEL: {
			}
			break;

			case CHANNEL_EVENT_TYPE_STOP_CHANNEL: {
				channel_info->channel_event_stop.stop_reason = CHANNEL_RECORD_ITEM_STOP_REASON_MANUAL;
			}
			break;

			default: {
			}
			break;
		}

		channel_event->channel_id = channel_id;
		channel_event->type = type;
		channel_event->ctx = &channel_info->channel_event_start_display;

		channels_event->type = CHANNELS_EVENT_CHANNEL;
		channels_event->event = channel_event;

		if(send_channels_event(channels_info, channels_event, 100) != 0) {
			os_free(channels_event->event);
			os_free(channels_event);
			debug("send channel %d type %d failed!", channel_id, type);
		} else {
			debug("send channel %d type %d successful!", channel_id, type);
		}
	} else {
		_hexdump("fn2 content", content, request->header.data_size);
	}
}

static void fn3(request_t *request)
{
	static uint32_t file_crc32 = 0;

	uint32_t data_size = request->header.data_size;
	uint32_t data_offset = request->header.data_offset;
	uint32_t total_size = request->header.total_size;
	uint32_t stage = request->payload.stage;
	uint8_t *data = (uint8_t *)(request + 1);
	uint8_t start_upgrade_app = 0;

	if(is_app() == 1) {
		if(set_app_valid(0) != 0) {
			debug("");
			return;
		}

		_printf("reset to bootloader!\n");

		HAL_NVIC_SystemReset();
		return;
	}

	if(stage == 0) {
		flash_erase_sector(IAP_CONST_FW_ADDRESS_START_SECTOR, IAP_CONST_FW_ADDRESS_SECTOR_NUMBER);
	} else if(stage == 1) {
		if(data_size == 4) {
			uint32_t *p = (uint32_t *)data;
			file_crc32 = *p;
		}
	} else if(stage == 2) {
		flash_write(IAP_CONST_FW_ADDRESS + data_offset, data, data_size);

		if(data_offset + data_size == total_size) {
			uint32_t read_offset = 0;
			uint32_t crc32 = 0;

			while(read_offset < total_size) {
				uint32_t i;
				uint32_t left = total_size - read_offset;
				uint32_t read_size = (left > 32) ? 32 : left;
				uint8_t *read_buffer = (uint8_t *)(IAP_CONST_FW_ADDRESS + read_offset);

				for(i = 0; i < read_size; i++) {
					crc32 += read_buffer[i];
				}

				read_offset += read_size;
			}

			_printf("crc32:%x, file_crc32:%x\n", crc32, file_crc32);

			if(crc32 == file_crc32) {
				start_upgrade_app = 1;
			}
		}
	}

	loopback(request);

	if(start_upgrade_app != 0) {
		_printf("start upgrade app!\n");

		if(set_firmware_size(total_size) != 0) {
			debug("");
		}

		if(set_firmware_valid(1) != 0) {
			debug("");
		}

		if(set_firmware_valid(0) != 0) {
			debug("");
		}

		if(set_app_valid(1) != 0) {
			debug("");
		}

		HAL_NVIC_SystemReset();
	}
}

static int p_host(struct hostent *ent)
{
	int ret = 0;
	char **cp;

	if(ent == NULL) {
		ret = -1;
		return ret;
	}

	_printf("\n");

	_printf("h_name:%s\n", ent->h_name);
	_printf("h_aliases:\n");
	cp = ent->h_aliases;

	while(*cp != NULL) {
		_printf("%s\n", *cp);
		cp += 1;

		if(*cp != NULL) {
			//_printf(", ");
		}
	}

	_printf("h_addrtype:%d\n", ent->h_addrtype);

	_printf("h_length:%d\n", ent->h_length);

	_printf("h_addr_list:\n");
	cp = ent->h_addr_list;

	while(*cp != NULL) {
		_printf("%s\n", inet_ntoa(**cp));
		cp += 1;

		if(*cp != NULL) {
			//_printf(", ");
		}
	}

	return ret;
}

static void get_host_by_name(char *content, uint32_t size)
{
	struct hostent *ent;
	char *hostname = (char *)os_alloc(RECV_BUFFER_SIZE);
	int ret;
	int fn;
	int catched;

	//_hexdump("content", (const char *)content, size);

	if(hostname == NULL) {
		return;
	}

	hostname[0] = 0;

	ret = sscanf(content, "%d %s%n", &fn, hostname, &catched);

	if(ret == 2) {
		_printf("hostname:%s!\n", hostname);
		ent = gethostbyname(hostname);
		p_host(ent);
	} else {
		_printf("no hostname!\n");
	}

	os_free(hostname);
}

static void fn4(request_t *request)
{
	const ip_addr_t *local_ip = get_default_ipaddr();
	_printf("local host ip:%s\n", inet_ntoa(*local_ip));

	get_host_by_name((char *)(request + 1), request->header.data_size);
	memset(request, 0, RECV_BUFFER_SIZE);
}

uint16_t osGetCPUUsage(void);
static void fn5(request_t *request)
{
	int size = xPortGetFreeHeapSize();
	uint8_t *os_thread_info;
	uint32_t ticks = osKernelSysTick();
	uint16_t cpu_usage = osGetCPUUsage();
	size_t total_heap_size = get_total_heap_size();
	size_t heap_size;
	size_t heap_count;
	size_t heap_max_size;

	get_mem_info(&heap_size, &heap_count,  &heap_max_size);

	_printf("cpu usage:%d\n", cpu_usage);
	_printf("free os heap size:%d\n", size);
	_printf("total heap size:%d, free heap size:%d, used:%d, heap count:%d, max heap size:%d\n",
	        total_heap_size,
	        total_heap_size - heap_size,
	        heap_size,
	        heap_count,
	        heap_max_size);
	_printf("current ticks:%lu\n", ticks);
	_printf("%lu day %lu hour %lu min %lu sec\n",
	        ticks / (1000 * 60 * 60 * 24),//day
	        (ticks % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60),//hour
	        (ticks % (1000 * 60 * 60)) / (1000 * 60),//min
	        (ticks % (1000 * 60)) / (1000)//sec
	       );

	if(size < 4 * 1024) {
		return;
	}

	size = 1024;

	os_thread_info = (uint8_t *)os_alloc(size);

	if(os_thread_info == NULL) {
		return;
	}

	osThreadList(os_thread_info);

	_puts((const char *)os_thread_info);

	os_free(os_thread_info);

	if(is_app() == 1) {
		_printf("in app!\n");
	} else {
		_printf("in bootloader!\n");
	}
}

static void fn6(request_t *request)
{
	start_dump_channels_stats();
}

static void fn7(request_t *request)
{
	int ret;
	char *content = (char *)(request + 1);
	channels_config_t *channels_config = get_channels_config(0);
	channels_info_t *channels_info;
	channel_info_t *channel_info;
	int fn;
	int channel;
	int voltage;
	int current;
	int state;
	int catched;
	uint32_t ticks = osKernelSysTick();

	if(channels_config == NULL) {
		return;
	}

	channels_info = get_channels();

	if(channels_info == NULL) {
		return;
	}

	ret = sscanf(content, "%d %d %d %d %d%n", &fn, &channel, &voltage, &current, &state, &catched);

	if(ret == 5) {
		debug("fn:%d, channel:%d, voltage:%d, current:%d, state:%d!", fn, channel, voltage, current, state);
	} else {
		_hexdump("fn7 content", content, request->header.data_size);
	}

	if(channel >= channels_info->channel_number) {
		return;
	}

	channel_info = channels_info->channel_info + channel;

	channel_info->require_voltage = voltage;
	channel_info->require_current = current;
	channel_info->remote_require_state = state;
}

static void fn8(request_t *request)
{
}

static void fn9(request_t *request)
{
}

void set_connect_enable(uint8_t enable);
uint8_t get_connect_enable(void);
static void fn10(request_t *request)
{
}

#include "modbus_addr_handler.h"
#include "modbus_slave_txrx.h"
static void fn11(request_t *request)
{
	int ret;
	char *content = (char *)(request + 1);
	int fn;
	int addr;
	int value;
	int op;
	int catched;
	channels_config_t *channels_config = get_channels_config(0);
	channels_info_t *channels_info;
	modbus_data_ctx_t modbus_data_ctx;

	if(channels_config == NULL) {
		return;
	}

	channels_info = get_channels();

	if(channels_info == NULL) {
		return;
	}

	ret = sscanf(content, "%d %d %d %d%n", &fn, &addr, &value, &op, &catched);

	if(ret == 4) {
		debug("fn:%d, addr:%d, value:%d, op:%d!", fn, addr, value, op);
	} else {
		_hexdump("fn11 content", content, request->header.data_size);
		return;
	}

	modbus_data_ctx.ctx = NULL;
	modbus_data_ctx.action = op;
	modbus_data_ctx.addr = addr;
	modbus_data_ctx.value = value;
	channels_modbus_data_action(channels_info, &modbus_data_ctx);

	debug("op:%s, addr:%s(%d), value:%d",
	      (modbus_data_ctx.action == MODBUS_DATA_ACTION_GET) ? "get" :
	      (modbus_data_ctx.action == MODBUS_DATA_ACTION_SET) ? "set" :
	      "unknow",
	      get_modbus_slave_addr_des(modbus_data_ctx.addr),
	      modbus_data_ctx.addr,
	      modbus_data_ctx.value);
}

static void fn17(request_t *request)
{
	char *content = (char *)(request + 1);
	int fn;
	int catched;
	int ret;

	ret = sscanf(content, "%d %n", &fn, &catched);

	if(ret == 1) {
		app_set_reset_config();
		app_save_config();
		debug("reset config ...");
		HAL_NVIC_SystemReset();
	}
}

static server_item_t server_map[] = {
	{1, fn1},
	{2, fn2},
	{3, fn3},
	{4, fn4},
	{5, fn5},
	{6, fn6},
	{7, fn7},
	{8, fn8},
	{9, fn9},
	{10, fn10},
	{11, fn11},
	{17, fn17},
};

server_map_info_t server_map_info = {
	.server_map = server_map,
	.server_map_size = sizeof(server_map) / sizeof(server_item_t),
};
