

/*================================================================
 *
 *
 *   文件名称：power_manager_group_policy_handler.c
 *   创 建 者：肖飞
 *   创建日期：2021年11月30日 星期二 15时07分16秒
 *   修改日期：2022年03月12日 星期六 14时26分48秒
 *   描    述：
 *
 *================================================================*/
#include "power_manager.h"

#include "relay_boards_comm_proxy_remote.h"
#include "channels_comm_proxy.h"

#include "log.h"

static int init_average(void *_power_manager_info)
{
	int ret = 0;
	return ret;
}

static int deinit_average(void *_power_manager_info)
{
	int ret = 0;
	return ret;
}

static int channel_start_average(void *_power_manager_channel_info)
{
	int ret = 0;
	power_manager_channel_info_t *power_manager_channel_info = (power_manager_channel_info_t *)_power_manager_channel_info;

	power_manager_channel_info->status.reassign = 0;
	power_manager_channel_info->status.reassign_module_group_number = 0;
	return ret;
}

static int channel_charging_average(void *_power_manager_channel_info)
{
	int ret = 0;
	power_manager_channel_info_t *power_manager_channel_info = (power_manager_channel_info_t *)_power_manager_channel_info;
	uint32_t ticks = osKernelSysTick();

	if(ticks_duration(ticks, power_manager_channel_info->output_current_alive_stamp) > 5000) {
		power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_manager_channel_info->power_manager_group_info;

		if(list_size(&power_manager_channel_info->power_module_group_list) > 1) {
			power_module_group_info_t *power_module_group_info = list_first_entry(&power_manager_channel_info->power_module_group_list, power_module_group_info_t, list);
			power_module_item_info_t *power_module_item_info;
			struct list_head *head1 = &power_module_group_info->power_module_item_list;
			list_for_each_entry(power_module_item_info, head1, power_module_item_info_t, list) {
				power_module_item_info->status.state = POWER_MODULE_ITEM_STATE_PREPARE_DEACTIVE;
			}
			list_move_tail(&power_module_group_info->list, &power_manager_group_info->power_module_group_deactive_list);
			debug("remove module group %d from channel %d", power_module_group_info->id, power_manager_channel_info->id);
			power_manager_channel_info->output_current_alive_stamp = ticks;
		}
	}

	return ret;
}

static void channel_info_deactive_power_module_group(power_manager_channel_info_t *power_manager_channel_info)
{
	struct list_head *pos;
	struct list_head *n;
	struct list_head *head;
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_manager_channel_info->power_manager_group_info;

	head = &power_manager_channel_info->power_module_group_list;

	list_for_each_safe(pos, n, head) {
		power_module_group_info_t *power_module_group_info = list_entry(pos, power_module_group_info_t, list);
		power_module_item_info_t *power_module_item_info;
		struct list_head *head1 = &power_module_group_info->power_module_item_list;
		list_for_each_entry(power_module_item_info, head1, power_module_item_info_t, list) {
			power_module_item_info->status.state = POWER_MODULE_ITEM_STATE_PREPARE_DEACTIVE;
		}
		list_move_tail(&power_module_group_info->list, &power_manager_group_info->power_module_group_deactive_list);
		debug("remove module group %d from channel %d", power_module_group_info->id, power_manager_channel_info->id);
	}
}

static void free_power_module_group_for_stop_channel(power_manager_group_info_t *power_manager_group_info)
{
	power_manager_channel_info_t *power_manager_channel_info;
	struct list_head *head;

	head = &power_manager_group_info->channel_deactive_list;

	list_for_each_entry(power_manager_channel_info, head, power_manager_channel_info_t, list) {
		channel_info_deactive_power_module_group(power_manager_channel_info);
	}
}

static int free_average(void *_power_manager_group_info)
{
	int ret = -1;
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;

	//释放要停机通道的模块
	free_power_module_group_for_stop_channel(power_manager_group_info);
	ret = 0;
	return ret;
}

static void channel_info_assign_one_power_module_group_agerage(power_manager_channel_info_t *power_manager_channel_info)
{
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_manager_channel_info->power_manager_group_info;
	struct list_head *head;
	power_module_group_info_t *power_module_group_info_item;
	power_module_item_info_t *power_module_item_info;

	if(list_empty(&power_manager_group_info->power_module_group_idle_list)) {
		return;
	}

	power_module_group_info_item = list_first_entry(&power_manager_group_info->power_module_group_idle_list, power_module_group_info_t, list);

	head = &power_module_group_info_item->power_module_item_list;
	list_for_each_entry(power_module_item_info, head, power_module_item_info_t, list) {
		if(power_module_item_info->status.state != POWER_MODULE_ITEM_STATE_IDLE) {
			debug("power module state is not idle:%s!!!", get_power_module_item_state_des(power_module_item_info->status.state));
		}

		power_module_item_info->status.state = POWER_MODULE_ITEM_STATE_PREPARE_ACTIVE;
	}
	power_module_group_info_item->power_manager_channel_info = power_manager_channel_info;
	list_move_tail(&power_module_group_info_item->list, &power_manager_channel_info->power_module_group_list);
	debug("assign module group %d to channel %d", power_module_group_info_item->id, power_manager_channel_info->id);
}

static void active_pdu_group_info_power_module_group_assign_average(power_manager_group_info_t *power_manager_group_info)
{
	power_manager_channel_info_t *power_manager_channel_info;
	struct list_head *head;

	while(list_size(&power_manager_group_info->power_module_group_idle_list) > 0) {//没有多余的模块需要分配了,退出
		head = &power_manager_group_info->channel_active_list;

		list_for_each_entry(power_manager_channel_info, head, power_manager_channel_info_t, list) {
			channel_info_assign_one_power_module_group_agerage(power_manager_channel_info);
		}
	}
}

static int assign_average(void *_power_manager_group_info)
{
	int ret = 0;
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;
	//充电中的枪数
	uint8_t active_channel_count;

	//获取需要充电的枪数
	active_channel_count = list_size(&power_manager_group_info->channel_active_list);
	debug("active_channel_count:%d", active_channel_count);

	if(active_channel_count == 0) {//如果没有枪需要充电,不分配
		return ret;
	}

	active_pdu_group_info_power_module_group_assign_average(power_manager_group_info);
	ret = 0;
	return ret;
}

static int _config(void *_power_manager_group_info)
{
	int ret = 0;
	//power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;
	//power_manager_info_t *power_manager_info = (power_manager_info_t *)power_manager_group_info->power_manager_info;
	//channels_info_t *channels_info = (channels_info_t *)power_manager_info->channels_info;

	return ret;
}

static int _sync(void *_power_manager_group_info)
{
	int ret = 0;
	//power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;
	//power_manager_info_t *power_manager_info = (power_manager_info_t *)power_manager_group_info->power_manager_info;
	//channels_info_t *channels_info = (channels_info_t *)power_manager_info->channels_info;
	return ret;
}

static power_manager_group_policy_handler_t power_manager_group_policy_handler_average = {
	.policy = POWER_MANAGER_GROUP_POLICY_AVERAGE,
	.init = init_average,
	.deinit = deinit_average,
	.channel_start = channel_start_average,
	.channel_charging = channel_charging_average,
	.free = free_average,
	.assign = assign_average,
	.config = _config,
	.sync = _sync,
};

static power_manager_group_policy_handler_t *power_manager_group_policy_handler_sz[] = {
	&power_manager_group_policy_handler_average,
};

power_manager_group_policy_handler_t *get_power_manager_group_policy_handler(uint8_t policy)
{
	int i;
	power_manager_group_policy_handler_t *power_manager_group_policy_handler = NULL;

	for(i = 0; i < ARRAY_SIZE(power_manager_group_policy_handler_sz); i++) {
		power_manager_group_policy_handler_t *power_manager_group_policy_handler_item = power_manager_group_policy_handler_sz[i];

		if(power_manager_group_policy_handler_item->policy == policy) {
			power_manager_group_policy_handler = power_manager_group_policy_handler_item;
		}
	}

	return power_manager_group_policy_handler;
}

static void modify_valid_time(void)
{
	struct tm tm = {0};
	time_t ts;

	tm.tm_year = 2021 - 1900;
	tm.tm_mon = 1 - 1;
	tm.tm_mon = 1 - 1;
	tm.tm_mday = 1;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	ts = mktime(&tm);
	set_time(ts);
}

void power_manager_restore_config(channels_info_t *channels_info)
{
	int i;
	int j;

	channels_settings_t *channels_settings = &channels_info->channels_settings;
	power_manager_settings_t *power_manager_settings = &channels_settings->power_manager_settings;

	power_manager_settings->power_manager_group_number = 1;

	debug("power_manager_group_number:%d", power_manager_settings->power_manager_group_number);

	for(i = 0; i < power_manager_settings->power_manager_group_number; i++) {
		power_manager_group_settings_t *power_manager_group_settings = &power_manager_settings->power_manager_group_settings[i];

		power_manager_group_settings->channel_number = 3;
		power_manager_group_settings->relay_board_number_per_channel = 0;

		for(j = 0; j < power_manager_group_settings->relay_board_number_per_channel; j++) {
			power_manager_group_settings->slot_per_relay_board[j] = 6;
		}

		power_manager_group_settings->power_module_group_number = 14;

		channels_info->channel_number += power_manager_group_settings->channel_number;

		for(j = 0; j < power_manager_group_settings->power_module_group_number; j++) {
			power_module_group_settings_t *power_module_group_settings = &power_manager_group_settings->power_module_group_settings[j];
			power_module_group_settings->power_module_number = 1;
		}
	}

	modify_valid_time();
}

