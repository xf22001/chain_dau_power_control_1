

/*================================================================
 *
 *
 *   文件名称：channels_comm_proxy_remote.c
 *   创 建 者：肖飞
 *   创建日期：2021年09月16日 星期四 10时34分46秒
 *   修改日期：2022年06月02日 星期四 17时10分07秒
 *   描    述：
 *
 *================================================================*/
#include "channels_comm_proxy_remote.h"

#include "proxy.h"
#include "channels_comm_proxy.h"
#include "connect_state.h"
#include "command_status.h"
#include "can_command.h"
#include "can_data_task.h"
#include "can_data_task.h"
#include "power_manager.h"
#include "channel.h"

#define LOG_DISABLE
#include "log.h"

typedef struct {
	uint8_t require_state;//channel_require_work_state_t
	uint8_t module_assign_info;//module_assign_info_t
} channel_data_ctx_t;

typedef struct {
	connect_state_t connect_state;
	command_status_t *cmd_ctx;
	channel_data_ctx_t data_ctx;
	callback_item_t channel_power_module_assign_ready_cb;
} channels_comm_proxy_channel_ctx_t;

typedef struct {
	can_tx_msg_t can_tx_msg;
	can_rx_msg_t *can_rx_msg;
	uint32_t periodic_stamp;
	can_data_task_info_t *can_data_task_info;
	callback_item_t can_data_request_cb;
	callback_item_t can_data_response_cb;

	channels_comm_proxy_channel_ctx_t *channels_comm_proxy_channel_ctx;
	module_status_t *module_status;
} channels_comm_proxy_ctx_t;

typedef int (*request_callback_t)(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index);
typedef int (*response_callback_t)(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index);
typedef int (*timeout_callback_t)(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index);

typedef struct {
	uint8_t cmd;
	uint8_t cmd_code;
	uint8_t broadcast;
	uint32_t request_period;
	request_callback_t request_callback;
	response_callback_t response_callback;
	timeout_callback_t timeout_callback;
} command_item_t;

static int request_callback_proxy_null(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	return 0;
}

static int response_callback_proxy_null(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	return 0;
}

static int timeout_callback_proxy_null(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	return 0;
}

static void start_stop_channel(channels_info_t *channels_info, uint8_t channel_id, channel_event_type_t type)
{
	uint32_t timeout = 100;
	channel_info_t *channel_info = channels_info->channel_info + channel_id;
	channels_event_t *channels_event = (channels_event_t *)os_calloc(1, sizeof(channels_event_t));
	channel_event_t *channel_event = (channel_event_t *)os_calloc(1, sizeof(channel_event_t));
	int ret;

	OS_ASSERT(channels_event != NULL);
	OS_ASSERT(channel_event != NULL);

	channels_event->type = CHANNELS_EVENT_CHANNEL;
	channels_event->event = channel_event;

	channel_event->channel_id = channel_id;
	channel_event->type = type;
	channel_event->ctx = &channel_info->channel_event_start_display;

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

	ret = send_channels_event(channels_info, channels_event, timeout);

	if(ret != 0) {
		os_free(channels_event->event);
		os_free(channels_event);
		debug("send channel %d type %d failed!", channel_id, type);
	} else {
		debug("send channel %d type %d successful!", channel_id, type);
	}
}

static int request_channel_require(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;

	cmd_ctx->state = COMMAND_STATE_IDLE;

	ret = 0;

	return ret;
}

static int response_channel_require(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_rx_msg->Data;
	channels_config_t *channels_config = channels_info->channels_config;
	proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_proxy_channel_index(&channels_config->proxy_channel_info, proxy_channel_index);
	channel_info_t *channel_info = channels_info->channel_info + proxy_channel_item->channel_id;
	channel_require_t *channel_require = (channel_require_t *)channels_comm_proxy_ctx->can_rx_msg->Data;
	uint8_t channel_fault = 0;
	uint16_t voltage;
	uint16_t current;

	voltage = get_u16_from_u8_lh(
	              channel_require->require_voltage_l,
	              channel_require->require_voltage_h);
	current = get_u16_from_u8_lh(
	              channel_require->require_current_l,
	              channel_require->require_current_h);

	channel_require_update(channel_info, voltage, current, CHANNEL_REQUIRE_MODE_NORMAL);

	//debug("channel_id:%d require_voltage:%d, require_current:%d",
	//      channel_info->channel_id,
	//      channel_info->require_voltage,
	//      channel_info->require_current);

	if(data_ctx->require_state != channel_require->require_state) {
		debug("channel_id:%d require_state %s -> %s",
		      channel_info->channel_id,
		      get_channel_require_work_state_des(data_ctx->require_state),
		      get_channel_require_work_state_des(channel_require->require_state));

		switch(channel_require->require_state) {
			case CHANNEL_WORK_STATE_IDLE:
			case CHANNEL_WORK_STATE_STOP: {//停机
				command_status_t *cmd_ctx_module_ready = channel_ctx->cmd_ctx + channels_comm_proxy_command_enum(MODULES_READY);

				cmd_ctx_module_ready->state = COMMAND_STATE_IDLE;
				channel_request_stop(channel_info, CHANNEL_RECORD_ITEM_STOP_REASON_MANUAL);
			}
			break;

			default: {
			}
			break;
		}

		data_ctx->require_state = channel_require->require_state;
	}

	if(channel_require->fault_stop != 0) {
		channel_fault = 1;
	}

	if(get_fault(channel_info->faults, CHANNEL_FAULT_FAULT) != channel_fault) {
		set_fault(channel_info->faults, CHANNEL_FAULT_FAULT, channel_fault);

		debug("channel:%d update fault:%d", channel_info->channel_id, channel_fault);

		if(channel_fault != 0) {//停机
			command_status_t *cmd_ctx_module_ready = channel_ctx->cmd_ctx + channels_comm_proxy_command_enum(MODULES_READY);

			cmd_ctx_module_ready->state = COMMAND_STATE_IDLE;
			channel_request_stop(channel_info, channel_record_item_stop_reason_fault(CHANNEL_FAULT_FAULT));
		}
	}

	cmd_ctx->state = COMMAND_STATE_REQUEST;

	ret = 0;

	return ret;
}

static command_item_t command_item_channel_require = {
	.cmd = channels_comm_proxy_command_enum(CHANNEL_REQUIRE),
	.cmd_code = channels_comm_proxy_command_code_enum(CHANNEL_REQUIRE),
	.broadcast = 0,
	.request_period = 0,
	.request_callback = request_channel_require,
	.response_callback = response_channel_require,
	.timeout_callback = timeout_callback_proxy_null,
};

static int request_channel_start(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;

	cmd_ctx->state = COMMAND_STATE_IDLE;

	ret = 0;

	return ret;
}

static uint8_t pdu_channels_idle(channels_info_t *channels_info)
{
	int i;
	uint8_t ret = 1;

	for(i = 0; i < channels_info->channel_number; i++) {
		channel_info_t *channel_info = channels_info->channel_info + i;

		if(channel_info->state != CHANNEL_STATE_IDLE) {
			ret = 0;
			break;
		}
	}

	return ret;
}

static int response_channel_start(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_rx_msg->Data;
	channels_config_t *channels_config = channels_info->channels_config;
	proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_proxy_channel_index(&channels_config->proxy_channel_info, proxy_channel_index);

	if(pdu_channels_idle(channels_info) == 1) {
		start_stop_channel(channels_info, proxy_channel_item->channel_id, CHANNEL_EVENT_TYPE_START_CHANNEL);
	} else {
		command_status_t *_cmd_ctx = channel_ctx->cmd_ctx + channels_comm_proxy_command_enum(MODULES_READY);
		channel_data_ctx_t *_data_ctx = &channel_ctx->data_ctx;

		_data_ctx->module_assign_info = MODULE_ASSIGN_INFO_RETRY;
		_cmd_ctx->state = COMMAND_STATE_REQUEST;
	}

	cmd_ctx->state = COMMAND_STATE_REQUEST;

	ret = 0;

	return ret;
}

static command_item_t command_item_channel_start = {
	.cmd = channels_comm_proxy_command_enum(CHANNEL_START),
	.cmd_code = channels_comm_proxy_command_code_enum(CHANNEL_START),
	.broadcast = 0,
	.request_period = 0,
	.request_callback = request_channel_start,
	.response_callback = response_channel_start,
	.timeout_callback = timeout_callback_proxy_null,
};

static int request_channel_output(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;

	cmd_ctx->state = COMMAND_STATE_IDLE;

	ret = 0;

	return ret;
}

static int response_channel_output(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_rx_msg->Data;
	channels_config_t *channels_config = channels_info->channels_config;
	proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_proxy_channel_index(&channels_config->proxy_channel_info, proxy_channel_index);
	channel_info_t *channel_info = channels_info->channel_info + proxy_channel_item->channel_id;
	channel_output_t *channel_output = (channel_output_t *)channels_comm_proxy_ctx->can_rx_msg->Data;

	channel_info->voltage = get_u16_from_u8_lh(
	                            channel_output->output_voltage_l,
	                            channel_output->output_voltage_h);
	channel_info->current = get_u16_from_u8_lh(
	                            channel_output->output_current_l,
	                            channel_output->output_current_h);

	//debug("channel_id %d voltage:%d, current:%d",
	//      channel_info->channel_id,
	//      channel_info->voltage,
	//      channel_info->current);

	cmd_ctx->state = COMMAND_STATE_REQUEST;

	ret = 0;

	return ret;
}

static command_item_t command_item_channel_output = {
	.cmd = channels_comm_proxy_command_enum(CHANNEL_OUTPUT),
	.cmd_code = channels_comm_proxy_command_code_enum(CHANNEL_OUTPUT),
	.broadcast = 0,
	.request_period = 0,
	.request_callback = request_channel_output,
	.response_callback = response_channel_output,
	.timeout_callback = timeout_callback_proxy_null,
};

static uint8_t pdu_fault(channel_info_t *channel_info)
{
	uint8_t fault = 0;
	channels_info_t *channels_info = (channels_info_t *)channel_info->channels_info;

	if(get_first_fault(channels_info->faults) != -1) {
		debug("");
		fault = 1;
	}

	return fault;
}

static int request_pdu_status(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;
	channels_config_t *channels_config = channels_info->channels_config;
	proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_proxy_channel_index(&channels_config->proxy_channel_info, proxy_channel_index);
	channel_info_t *channel_info = channels_info->channel_info + proxy_channel_item->channel_id;
	cmd_request_pdu_status_t *cmd_request = (cmd_request_pdu_status_t *)channels_comm_proxy_ctx->can_tx_msg.Data;

	cmd_request->fault_stop = pdu_fault(channel_info);
	cmd_request->pdu_idle = pdu_channels_idle(channels_info);

	cmd_ctx->state = COMMAND_STATE_IDLE;

	ret = 0;

	return ret;
}

static command_item_t command_item_pdu_status = {
	.cmd = channels_comm_proxy_command_enum(DAU_STATUS),
	.cmd_code = channels_comm_proxy_command_code_enum(DAU_STATUS),
	.broadcast = 1,
	.request_period = 50,
	.request_callback = request_pdu_status,
	.response_callback = response_callback_proxy_null,
	.timeout_callback = timeout_callback_proxy_null,
};

static int request_modules_ready(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;
	channel_module_assign_info_t *channel_module_assign_info = (channel_module_assign_info_t *)channels_comm_proxy_ctx->can_tx_msg.Data;

	channel_module_assign_info->info = data_ctx->module_assign_info;

	cmd_ctx->state = COMMAND_STATE_RESPONSE;

	ret = 0;

	return ret;
}

static int response_modules_ready(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_rx_msg->Data;

	cmd_ctx->state = COMMAND_STATE_IDLE;

	ret = 0;

	return ret;
}

static command_item_t command_item_modules_ready = {
	.cmd = channels_comm_proxy_command_enum(MODULES_READY),
	.cmd_code = channels_comm_proxy_command_code_enum(MODULES_READY),
	.broadcast = 0,
	.request_period = 0,
	.request_callback = request_modules_ready,
	.response_callback = response_modules_ready,
	.timeout_callback = timeout_callback_proxy_null,
};

static int request_modules_status(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;

	if(cmd_ctx->index == 0) {
		int i;

		for(i = 0; i < power_manager_info->power_modules_info->power_module_number; i++) {
			power_module_item_info_t *power_module_item_info = power_manager_info->power_module_item_info + i;
			power_modules_info_t *power_modules_info = (power_modules_info_t *)power_module_item_info->power_modules_info;
			power_module_info_t *power_module_info = power_modules_info->power_module_info + power_module_item_info->id;
			module_status_t *module_status = channels_comm_proxy_ctx->module_status + i;
			power_module_item_status_t *status = &power_module_item_info->status;

			module_status->setting_voltage_l = get_u8_l_from_u16(power_module_info->setting_voltage / 100);
			module_status->setting_voltage_h = get_u8_h_from_u16(power_module_info->setting_voltage / 100);
			module_status->setting_current_l = get_u8_l_from_u16(power_module_info->setting_current / 100);
			module_status->setting_current_h = get_u8_h_from_u16(power_module_info->setting_current / 100);
			module_status->output_voltage_l = get_u8_l_from_u16(power_module_info->output_voltage);
			module_status->output_voltage_h = get_u8_h_from_u16(power_module_info->output_voltage);
			module_status->output_current_l = get_u8_l_from_u16(power_module_info->output_current);
			module_status->output_current_h = get_u8_h_from_u16(power_module_info->output_current);
			module_status->module_state_l = get_u8_l_from_u16(status->module_status);
			module_status->module_state_h = get_u8_h_from_u16(status->module_status);
			module_status->connect_state_l = get_u8_l_from_u16(status->connect_state);
			module_status->connect_state_h = get_u8_h_from_u16(status->connect_state);
		}
	}

	ret = can_com_prepare_tx_request_stateless(cmd_ctx,
	        can_com_cmd_common,
	        (uint8_t *)channels_comm_proxy_ctx->module_status,
	        sizeof(module_status_t) * power_manager_info->power_modules_info->power_module_number);

	return ret;
}

static command_item_t command_item_modules_status = {
	.cmd = channels_comm_proxy_command_enum(MODULES_STATUS),
	.cmd_code = channels_comm_proxy_command_code_enum(MODULES_STATUS),
	.broadcast = 1,
	.request_period = 2 * 1000,
	.request_callback = request_modules_status,
	.response_callback = response_callback_proxy_null,
	.timeout_callback = timeout_callback_proxy_null,
};

static power_module_info_t *get_alive_power_module_info(power_manager_info_t *power_manager_info)
{
	int i;
	power_module_info_t *power_module_info = NULL;

	for(i = 0; i < power_manager_info->power_modules_info->power_module_number; i++) {
		power_module_item_info_t *power_module_item_info = power_manager_info->power_module_item_info + i;
		power_module_item_status_t *status = &power_module_item_info->status;

		if(status->connect_state >= 6) {
			power_modules_info_t *power_modules_info = (power_modules_info_t *)power_module_item_info->power_modules_info;
			power_module_info = power_modules_info->power_module_info + power_module_item_info->id;
			break;
		}
	}

	return power_module_info;
}

static int request_input_voltage(channels_info_t *channels_info, void *_command_item, uint8_t proxy_channel_index)
{
	int ret = -1;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	command_item_t *item = (command_item_t *)_command_item;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
	//channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;
	//can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;
	input_voltage_info_t *input_voltage_info = (input_voltage_info_t *)channels_comm_proxy_ctx->can_tx_msg.Data;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;
	power_module_info_t *power_module_info = get_alive_power_module_info(power_manager_info);

	if(power_module_info == NULL) {
		return ret;
	}

	input_voltage_info->v_a_l = get_u8_l_from_u16(power_module_info->input_aline_voltage);
	input_voltage_info->v_a_h = get_u8_h_from_u16(power_module_info->input_aline_voltage);
	input_voltage_info->v_b_l = get_u8_l_from_u16(power_module_info->input_bline_voltage);
	input_voltage_info->v_b_h = get_u8_h_from_u16(power_module_info->input_bline_voltage);
	input_voltage_info->v_c_l = get_u8_l_from_u16(power_module_info->input_cline_voltage);
	input_voltage_info->v_c_h = get_u8_h_from_u16(power_module_info->input_cline_voltage);

	cmd_ctx->state = COMMAND_STATE_IDLE;

	ret = 0;

	return ret;
}

static command_item_t command_item_input_voltage = {
	.cmd = channels_comm_proxy_command_enum(INPUT_VOLTAGE),
	.cmd_code = channels_comm_proxy_command_code_enum(INPUT_VOLTAGE),
	.broadcast = 1,
	.request_period = 2 * 1000,
	.request_callback = request_input_voltage,
	.response_callback = response_callback_proxy_null,
	.timeout_callback = timeout_callback_proxy_null,
};

static command_item_t *channels_comm_proxy_command_table[] = {
	&command_item_channel_require,
	&command_item_channel_start,
	&command_item_channel_output,
	&command_item_pdu_status,
	&command_item_modules_ready,
	&command_item_modules_status,
	&command_item_input_voltage,
};

static void channels_comm_proxy_set_connect_state(channels_comm_proxy_ctx_t *channels_comm_proxy_ctx, uint8_t proxy_channel_index, uint8_t state)
{
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;

	update_connect_state(&channel_ctx->connect_state, state);
}

static uint8_t channels_comm_proxy_get_connect_state(channels_info_t *channels_info, uint8_t proxy_channel_index)
{
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;

	return get_connect_state(&channel_ctx->connect_state);
}

static uint32_t channels_comm_proxy_get_connect_stamp(channels_info_t *channels_info, uint8_t proxy_channel_index)
{
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_index;

	return get_connect_stamp(&channel_ctx->connect_state);
}

#define RESPONSE_TIMEOUT 200

static void handle_channel_connect_timeout(channels_info_t *channels_info)
{
	channels_config_t *channels_config = channels_info->channels_config;
	proxy_channel_item_t *proxy_channel_item;
	uint32_t ticks = osKernelSysTick();
	uint32_t fault = 0;
	int i;

	for(i = 0; i < channels_info->channel_number; i++) {
		channel_info_t *channel_info = channels_info->channel_info + i;
		proxy_channel_item = get_proxy_channel_item_by_channel_id(&channels_config->proxy_channel_info, channel_info->channel_id);

		if(proxy_channel_item == NULL) {
			continue;
		}

		channel_info->connect_state = channels_comm_proxy_get_connect_state(channels_info, proxy_channel_item->proxy_channel_index);

		if(channel_info->connect_state != 0) {
			channel_info->channel_connect_alive_stamps = ticks;
		}

		if(ticks_duration(ticks, channel_info->channel_connect_alive_stamps) >= 3000) {
			fault = 1;
		}

		if(get_fault(channel_info->faults, CHANNEL_FAULT_CONNECT_TIMEOUT) != fault) {
			set_fault(channel_info->faults, CHANNEL_FAULT_CONNECT_TIMEOUT, fault);
		}
	}
}

static void channels_comm_proxy_request_periodic(channels_info_t *channels_info)
{
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	channels_config_t *channels_config = channels_info->channels_config;
	uint8_t proxy_channel_number = channels_config->proxy_channel_info.proxy_channel_number;
	uint32_t ticks = osKernelSysTick();
	int i;
	int j;

	if(ticks_duration(ticks, channels_comm_proxy_ctx->periodic_stamp) < 50) {
		return;
	}

	channels_comm_proxy_ctx->periodic_stamp = ticks;

	handle_channel_connect_timeout(channels_info);

	for(j = 0; j < proxy_channel_number; j++) {
		proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_proxy_channel_index(&channels_config->proxy_channel_info, j);
		channels_comm_proxy_channel_ctx_t *channels_comm_proxy_channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + j;
		channel_info_t *channel_info = (channel_info_t *)channels_info->channel_info + proxy_channel_item->channel_id;

		uint8_t connect_timeout = 0;

		if(ticks_duration(ticks, channels_comm_proxy_get_connect_stamp(channels_info, j)) >= (3 * 1000)) {
			connect_timeout = 1;
		}

		if(get_fault(channel_info->faults, CHANNEL_FAULT_CONNECT_TIMEOUT) != connect_timeout) {
			set_fault(channel_info->faults, CHANNEL_FAULT_CONNECT_TIMEOUT, connect_timeout);

			if(connect_timeout != 0) {
				debug("channel %d(%d) timeout, ticks:%d, update stamp:%d",
				      proxy_channel_item->channel_id,
				      j,
				      ticks,
				      channels_comm_proxy_get_connect_stamp(channels_info, j));
			}
		}

		for(i = 0; i < ARRAY_SIZE(channels_comm_proxy_command_table); i++) {
			command_item_t *item = channels_comm_proxy_command_table[i];
			command_status_t *cmd_ctx = channels_comm_proxy_channel_ctx->cmd_ctx + item->cmd;

			if(cmd_ctx->state == COMMAND_STATE_RESPONSE) {//超时
				if(ticks_duration(ticks, cmd_ctx->send_stamp) >= RESPONSE_TIMEOUT) {
					channels_comm_proxy_set_connect_state(channels_comm_proxy_ctx, j, 0);
					debug("channel %d(%d), cmd %d(%s), index %d timeout, ticks:%d, send_stamp:%d, connect state:%d",
					      proxy_channel_item->channel_id,
					      j,
					      item->cmd,
					      get_channels_comm_proxy_command_des(item->cmd),
					      cmd_ctx->index,
					      ticks,
					      cmd_ctx->send_stamp,
					      channels_comm_proxy_get_connect_state(channels_info, j));

					cmd_ctx->state = COMMAND_STATE_REQUEST;

					if(item->timeout_callback != NULL) {
						item->timeout_callback(channels_info, item, j);
					}
				}
			}

			if((item->cmd == channels_comm_proxy_command_enum(CHANNEL_REQUIRE)) ||
			   (item->cmd == channels_comm_proxy_command_enum(CHANNEL_OUTPUT))) {
				if(ticks_duration(ticks, cmd_ctx->recv_stamp) >= 1000) {
					channels_comm_proxy_set_connect_state(channels_comm_proxy_ctx, j, 0);
					debug("channel %d(%d), cmd %d(%s), index %d timeout, ticks:%d, recv_stamp:%d, connect state:%d",
					      proxy_channel_item->channel_id,
					      j,
					      item->cmd,
					      get_channels_comm_proxy_command_des(item->cmd),
					      cmd_ctx->index,
					      ticks,
					      cmd_ctx->recv_stamp,
					      channels_comm_proxy_get_connect_state(channels_info, j));
				}
			}

			if(item->request_period == 0) {
				continue;
			}

			if(item->broadcast != 0) {
				if(j != 0) {
					continue;
				}
			}

			if(cmd_ctx->available == 0) {
				continue;
			}

			if(cmd_ctx->state != COMMAND_STATE_IDLE) {
				continue;
			}

			if(ticks_duration(ticks, cmd_ctx->stamp) >= item->request_period) {
				cmd_ctx->stamp = ticks;

				cmd_ctx->index = 0;
				cmd_ctx->state = COMMAND_STATE_REQUEST;

				//debug("channel %d(%d), cmd %d(%s), index %d start",
				//      proxy_channel_item->channel_id,
				//      j,
				//      item->cmd,
				//      get_channels_comm_proxy_command_des(item->cmd),
				//      cmd_ctx->index,
				//      channels_comm_proxy_get_connect_state(channels_info, j));
			}
		}
	}
}

static void channels_comm_proxy_request(channels_info_t *channels_info, can_info_t *can_info)
{
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	channels_config_t *channels_config = channels_info->channels_config;
	uint8_t proxy_channel_number = channels_config->proxy_channel_info.proxy_channel_number;
	int i;
	int j;
	int ret;

	for(j = 0; j < proxy_channel_number; j++) {
		proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_proxy_channel_index(&channels_config->proxy_channel_info, j);
		channels_comm_proxy_channel_ctx_t *channels_comm_proxy_channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + j;

		for(i = 0; i < ARRAY_SIZE(channels_comm_proxy_command_table); i++) {
			uint32_t ticks = osKernelSysTick();
			command_item_t *item = channels_comm_proxy_command_table[i];
			command_status_t *cmd_ctx = channels_comm_proxy_channel_ctx->cmd_ctx + item->cmd;
			can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_tx_msg.Data;
			u_com_can_id_t *u_com_can_id = (u_com_can_id_t *)&channels_comm_proxy_ctx->can_tx_msg.ExtId;

			channels_comm_proxy_request_periodic(channels_info);

			if(cmd_ctx->available == 0) {
				continue;
			}

			if(cmd_ctx->state != COMMAND_STATE_REQUEST) {
				continue;
			}

			u_com_can_id->v = 0;
			u_com_can_id->s.type = PROXY_TYPE_CHANNEL;
			u_com_can_id->s.flag = PROXY_FLAG_CHANNEL;
			u_com_can_id->s.src_id = PROXY_ADDR_REMOTE_CHANNEL;
			u_com_can_id->s.dst_id = proxy_channel_item->channel_id + 1;

			channels_comm_proxy_ctx->can_tx_msg.IDE = CAN_ID_EXT;
			channels_comm_proxy_ctx->can_tx_msg.RTR = CAN_RTR_DATA;
			channels_comm_proxy_ctx->can_tx_msg.DLC = 8;

			memset(channels_comm_proxy_ctx->can_tx_msg.Data, 0, 8);

			can_com_cmd_common->cmd = item->cmd_code;

			ret = item->request_callback(channels_info, item, j);

			//debug("request channel %d(%d), cmd %d(%s), index:%d",
			//      proxy_channel_item->channel_id,
			//      j,
			//      item->cmd,
			//      get_channels_comm_proxy_command_des(item->cmd),
			//      can_com_cmd_common->index);

			if(ret != 0) {
				debug("process channel %d(%d), cmd %d(%s), index %d error!",
				      proxy_channel_item->channel_id,
				      j,
				      item->cmd,
				      get_channels_comm_proxy_command_des(item->cmd),
				      can_com_cmd_common->index);
				continue;
			}

			if(item->broadcast != 0) {
				u_com_can_id->s.dst_id = 0x00;
			}

			cmd_ctx->send_stamp = ticks;
			ret = can_tx_data(can_info, &channels_comm_proxy_ctx->can_tx_msg, 10);

			if(ret != 0) {//发送失败
				channels_comm_proxy_set_connect_state(channels_comm_proxy_ctx, j, 0);

				debug("send channel %d(%d), cmd %d(%s), index:%d error!",
				      proxy_channel_item->channel_id,
				      j,
				      item->cmd,
				      get_channels_comm_proxy_command_des(item->cmd),
				      can_com_cmd_common->index);
			}

			can_info_signal(can_info);

			return;
		}
	}
}

static void channels_comm_proxy_response(channels_info_t *channels_info, can_rx_msg_t *can_rx_msg)
{
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	channels_config_t *channels_config = channels_info->channels_config;
	uint8_t proxy_channel_number = channels_config->proxy_channel_info.proxy_channel_number;
	int i;
	u_com_can_id_t *u_com_can_id;
	uint8_t channel_id;
	proxy_channel_item_t *proxy_channel_item;
	int ret;
	uint32_t ticks = osKernelSysTick();

	channels_comm_proxy_ctx->can_rx_msg = can_rx_msg;

	u_com_can_id = (u_com_can_id_t *)&can_rx_msg->ExtId;

	if(u_com_can_id->s.type != PROXY_TYPE_CHANNEL) {
		//debug("response type:%02x!", u_com_can_id->s.type);
		return;
	}

	if(u_com_can_id->s.flag != PROXY_FLAG_CHANNEL) {
		//debug("response flag:%02x!", u_com_can_id->s.flag);
		return;
	}

	if(u_com_can_id->s.dst_id != PROXY_ADDR_REMOTE_CHANNEL) {
		//debug("response dst_id:%02x!", u_com_can_id->s.dst_id);
		//debug("rx extid:0x%08x, data:%02x %02x %02x %02x %02x %02x %02x %02x\n",
		//      can_rx_msg->ExtId,
		//      can_rx_msg->Data[0],
		//      can_rx_msg->Data[1],
		//      can_rx_msg->Data[2],
		//      can_rx_msg->Data[3],
		//      can_rx_msg->Data[4],
		//      can_rx_msg->Data[5],
		//      can_rx_msg->Data[6],
		//      can_rx_msg->Data[7]);
		return;
	}

	channel_id = u_com_can_id->s.src_id - 1;

	if(channel_id >= channels_info->channel_number) {
		debug("channel_id:%d!", channel_id);
		//debug("rx extid:0x%08x, data:%02x %02x %02x %02x %02x %02x %02x %02x\n",
		//      can_rx_msg->ExtId,
		//      can_rx_msg->Data[0],
		//      can_rx_msg->Data[1],
		//      can_rx_msg->Data[2],
		//      can_rx_msg->Data[3],
		//      can_rx_msg->Data[4],
		//      can_rx_msg->Data[5],
		//      can_rx_msg->Data[6],
		//      can_rx_msg->Data[7]);
		return;
	}

	proxy_channel_item = get_proxy_channel_item_by_channel_id(&channels_config->proxy_channel_info, channel_id);

	if(proxy_channel_item == NULL) {
		return;
	}

	if(proxy_channel_item->proxy_channel_index >= proxy_channel_number) {
		debug("proxy_channel_index:%d!", proxy_channel_item->proxy_channel_index);
		//debug("rx extid:0x%08x, data:%02x %02x %02x %02x %02x %02x %02x %02x\n",
		//      can_rx_msg->ExtId,
		//      can_rx_msg->Data[0],
		//      can_rx_msg->Data[1],
		//      can_rx_msg->Data[2],
		//      can_rx_msg->Data[3],
		//      can_rx_msg->Data[4],
		//      can_rx_msg->Data[5],
		//      can_rx_msg->Data[6],
		//      can_rx_msg->Data[7]);
		return;
	}

	for(i = 0; i < ARRAY_SIZE(channels_comm_proxy_command_table); i++) {
		command_item_t *item = channels_comm_proxy_command_table[i];
		can_com_cmd_common_t *can_com_cmd_common = (can_com_cmd_common_t *)channels_comm_proxy_ctx->can_rx_msg->Data;

		if(can_com_cmd_common->cmd == item->cmd_code) {
			channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_item->proxy_channel_index;
			command_status_t *cmd_ctx = channel_ctx->cmd_ctx + item->cmd;
			//debug("response channel %d(%d), cmd %d(%s), index:%d",
			//      proxy_channel_item->channel_id,
			//      proxy_channel_item->proxy_channel_index,
			//      item->cmd,
			//      get_channels_comm_proxy_command_des(item->cmd),
			//      can_com_cmd_common->index);

			cmd_ctx->recv_stamp = ticks;

			channels_comm_proxy_set_connect_state(channels_comm_proxy_ctx, proxy_channel_item->proxy_channel_index, 1);

			ret = item->response_callback(channels_info, item, proxy_channel_item->proxy_channel_index);

			if(ret != 0) {//收到响应
				debug("process response channel %d(%d), cmd %d(%s), index:%d error!",
				      proxy_channel_item->channel_id,
				      proxy_channel_item->proxy_channel_index,
				      item->cmd,
				      get_channels_comm_proxy_command_des(item->cmd),
				      can_com_cmd_common->index);
				debug("rx extid:0x%08x, data:%02x %02x %02x %02x %02x %02x %02x %02x\n",
				      can_rx_msg->ExtId,
				      can_rx_msg->Data[0],
				      can_rx_msg->Data[1],
				      can_rx_msg->Data[2],
				      can_rx_msg->Data[3],
				      can_rx_msg->Data[4],
				      can_rx_msg->Data[5],
				      can_rx_msg->Data[6],
				      can_rx_msg->Data[7]);
			}

			break;
		}
	}

	return;
}

static void can_data_request(void *fn_ctx, void *chain_ctx)
{
	channels_info_t *channels_info = (channels_info_t *)fn_ctx;
	can_data_task_info_t *can_data_task_info = (can_data_task_info_t *)chain_ctx;

	channels_comm_proxy_request(channels_info, can_data_task_info->can_info);
}

static void can_data_response(void *fn_ctx, void *chain_ctx)
{
	channels_info_t *channels_info = (channels_info_t *)fn_ctx;
	can_data_task_info_t *can_data_task_info = (can_data_task_info_t *)chain_ctx;

	can_rx_msg_t *can_rx_msg = can_get_msg(can_data_task_info->can_info);

	channels_comm_proxy_response(channels_info, can_rx_msg);
}

static int init_channels_config_proxy_channel_info(channels_config_t *channels_config)
{
	int ret = 0;
	int i;
	int j;

	channels_config->proxy_channel_info.proxy_channel_number = 0;

	OS_ASSERT(channels_config->proxy_channel_info.items == NULL);

	for(i = 0; i < channels_config->channel_number; i++) {
		channel_config_t *channel_config = channels_config->channel_config[i];

		if(channel_config->channel_type == CHANNEL_TYPE_PROXY_REMOTE) {
			channels_config->proxy_channel_info.proxy_channel_number++;
		}
	}

	if(channels_config->proxy_channel_info.proxy_channel_number == 0) {
		return ret;
	}

	channels_config->proxy_channel_info.items = (proxy_channel_item_t *)os_calloc(channels_config->proxy_channel_info.proxy_channel_number, sizeof(proxy_channel_item_t));
	OS_ASSERT(channels_config->proxy_channel_info.items != NULL);

	j = 0;

	for(i = 0; i < channels_config->channel_number; i++) {
		channel_config_t *channel_config = channels_config->channel_config[i];

		if(channel_config->channel_type == CHANNEL_TYPE_PROXY_REMOTE) {
			proxy_channel_item_t *item = channels_config->proxy_channel_info.items + j;
			item->proxy_channel_index = j;
			item->channel_id = i;

			debug("proxy remote channel %d index %d", item->channel_id, item->proxy_channel_index);

			j++;
		}
	}

	return ret;
}

static void channel_power_module_assign_ready(void *_proxy_channel_item, void *_channel_info)
{
	proxy_channel_item_t *proxy_channel_item = (proxy_channel_item_t *)_proxy_channel_item;
	channel_info_t *channel_info = (channel_info_t *)_channel_info;
	channels_info_t *channels_info = (channels_info_t *)channel_info->channels_info;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)channels_info->channels_comm_proxy_ctx;
	channels_comm_proxy_channel_ctx_t *channel_ctx = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + proxy_channel_item->proxy_channel_index;
	command_status_t *cmd_ctx = channel_ctx->cmd_ctx + channels_comm_proxy_command_enum(MODULES_READY);
	channel_data_ctx_t *data_ctx = &channel_ctx->data_ctx;

	data_ctx->module_assign_info = MODULE_ASSIGN_INFO_READY;
	cmd_ctx->state = COMMAND_STATE_REQUEST;
}

int start_channels_comm_proxy_remote(channels_info_t *channels_info)
{
	int ret = 0;
	channels_comm_proxy_ctx_t *channels_comm_proxy_ctx;
	channels_config_t *channels_config = channels_info->channels_config;
	uint8_t proxy_channel_number;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;
	int i;

	OS_ASSERT(channels_info->channels_comm_proxy_ctx == NULL);

	init_channels_config_proxy_channel_info(channels_config);

	proxy_channel_number = channels_config->proxy_channel_info.proxy_channel_number;

	if(proxy_channel_number == 0) {
		return ret;
	}

	channels_comm_proxy_ctx = (channels_comm_proxy_ctx_t *)os_calloc(1, sizeof(channels_comm_proxy_ctx_t));
	OS_ASSERT(channels_comm_proxy_ctx != NULL);
	channels_comm_proxy_ctx->module_status = (module_status_t *)os_calloc(power_manager_info->power_modules_info->power_module_number, sizeof(module_status_t));
	OS_ASSERT(channels_comm_proxy_ctx->module_status != NULL);

	channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx = (channels_comm_proxy_channel_ctx_t *)os_calloc(proxy_channel_number, sizeof(channels_comm_proxy_channel_ctx_t));
	OS_ASSERT(channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx != NULL);

	for(i = 0; i < proxy_channel_number; i++) {
		int j;
		channels_comm_proxy_channel_ctx_t *item = channels_comm_proxy_ctx->channels_comm_proxy_channel_ctx + i;
		//channel_data_ctx_t *channel_data_ctx = &item->data_ctx;
		proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_proxy_channel_index(&channels_config->proxy_channel_info, i);

		item->cmd_ctx = (command_status_t *)os_calloc(CHANNELS_COMM_PROXY_COMMAND_SIZE, sizeof(command_status_t));
		OS_ASSERT(item->cmd_ctx != NULL);

		for(j = 0; j < CHANNELS_COMM_PROXY_COMMAND_SIZE; j++) {
			item->cmd_ctx[j].available = 1;
		}

		item->channel_power_module_assign_ready_cb.fn = channel_power_module_assign_ready;
		item->channel_power_module_assign_ready_cb.fn_ctx = proxy_channel_item;
		OS_ASSERT(register_callback(power_manager_info->power_manager_channel_module_assign_ready_chain, &item->channel_power_module_assign_ready_cb) == 0);
	}

	channels_comm_proxy_ctx->can_data_task_info = get_or_alloc_can_data_task_info(channels_config->proxy_channel_info.hcan);
	OS_ASSERT(channels_comm_proxy_ctx->can_data_task_info != NULL);

	channels_comm_proxy_ctx->can_data_request_cb.fn = can_data_request;
	channels_comm_proxy_ctx->can_data_request_cb.fn_ctx = channels_info;
	add_can_data_task_info_request_cb(channels_comm_proxy_ctx->can_data_task_info, &channels_comm_proxy_ctx->can_data_request_cb);
	channels_comm_proxy_ctx->can_data_response_cb.fn = can_data_response;
	channels_comm_proxy_ctx->can_data_response_cb.fn_ctx = channels_info;
	add_can_data_task_info_response_cb(channels_comm_proxy_ctx->can_data_task_info, &channels_comm_proxy_ctx->can_data_response_cb);

	channels_info->channels_comm_proxy_ctx = channels_comm_proxy_ctx;

	return ret;
}
