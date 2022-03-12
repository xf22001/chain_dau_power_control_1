

/*================================================================
 *   
 *   
 *   文件名称：modbus_addr_handler.h
 *   创 建 者：肖飞
 *   创建日期：2020年07月17日 星期五 10时11分10秒
 *   修改日期：2021年10月20日 星期三 16时59分02秒
 *   描    述：
 *
 *================================================================*/
#ifndef _MODBUS_ADDR_HANDLER_H
#define _MODBUS_ADDR_HANDLER_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "cmsis_os.h"
#include "app_platform.h"

#ifdef __cplusplus
}
#endif

#define add_enum_module_info(MODULE_ID) \
	MODBUS_ADDR_MODULE_##MODULE_ID##_PDU_GROUP_ID, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_CHANNEL_ID, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_RELAY_BOARD_ID, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_WORK_STATE, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_VOLTAGE, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_CURRENT, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_VOLTAGE, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_CURRENT, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_MODULE_STATE, \
	MODBUS_ADDR_MODULE_##MODULE_ID##_CONNECT_STATE

#define add_enum_channel_info(CHANNEL_ID) \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_PDU_GROUP_ID, \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_VOLTAGE, \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_CURRENT, \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_VOLTAGE, \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_CURRENT, \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_WORK_STATE, \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_CONNECT_STATE, \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_FAULT

#define add_enum_relay_board_info(RELAY_BOARD_ID) \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CHANNEL_ID, \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONFIG, \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_WORK_STATE, \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_OFFSET, \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_NUMBER, \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONNECT_STATE, \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_FAULT

typedef enum {
	MODBUS_ADDR_UNUSED = 0,
	add_enum_module_info(0),
	add_enum_module_info(1),
	add_enum_module_info(2),
	add_enum_module_info(3),
	add_enum_module_info(4),
	add_enum_module_info(5),
	add_enum_module_info(6),
	add_enum_module_info(7),
	add_enum_module_info(8),
	add_enum_module_info(9),
	add_enum_module_info(10),
	add_enum_module_info(11),

	addr_pad_module_info_start = 1120,
	add_enum_module_info(12),
	add_enum_module_info(13),
	add_enum_module_info(14),
	addr_current_max,
	addr_pad_module_info_end = 120,

	add_enum_channel_info(0),
	add_enum_channel_info(1),
	add_enum_channel_info(2),
	add_enum_channel_info(3),
	add_enum_channel_info(4),
	add_enum_channel_info(5),
	add_enum_channel_info(6),
	add_enum_channel_info(7),
	add_enum_channel_info(8),
	add_enum_channel_info(9),
	add_enum_channel_info(10),
	add_enum_channel_info(11),

	add_enum_relay_board_info(0),
	add_enum_relay_board_info(1),
	add_enum_relay_board_info(2),
	add_enum_relay_board_info(3),
	add_enum_relay_board_info(4),
	add_enum_relay_board_info(5),
	add_enum_relay_board_info(6),
	add_enum_relay_board_info(7),
	add_enum_relay_board_info(8),
	add_enum_relay_board_info(9),
	add_enum_relay_board_info(10),
	add_enum_relay_board_info(11),
	add_enum_relay_board_info(12),
	add_enum_relay_board_info(13),
	add_enum_relay_board_info(14),
	add_enum_relay_board_info(15),
	add_enum_relay_board_info(16),
	add_enum_relay_board_info(17),
	add_enum_relay_board_info(18),
	add_enum_relay_board_info(19),
	add_enum_relay_board_info(20),
	add_enum_relay_board_info(21),
	add_enum_relay_board_info(22),
	add_enum_relay_board_info(23),

	MODBUS_ADDR_PDU_GROUP_NUMBER,
	MODBUS_ADDR_PDU_GROUP1_CHANNEL_NUMBER,
	MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD_NUMBER_PER_CHANNEL,
	MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD1_MODULE_NUMBER_PER_CHANNEL,
	MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD2_MODULE_NUMBER_PER_CHANNEL,
	MODBUS_ADDR_PDU_GROUP2_CHANNEL_NUMBER,
	MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD_NUMBER_PER_CHANNEL,
	MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD1_MODULE_NUMBER_PER_CHANNEL,
	MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD2_MODULE_NUMBER_PER_CHANNEL,

	MODBUS_ADDR_MODULE_MAX_OUTPUT_VOLTAGE,
	MODBUS_ADDR_MODULE_MIN_OUTPUT_VOLTAGE,
	MODBUS_ADDR_MODULE_MAX_OUTPUT_CURRENT,
	MODBUS_ADDR_MODULE_MIN_OUTPUT_CURRENT,
	MODBUS_ADDR_MODULE_TYPE,

	MODBUS_ADDR_DAU_WORK_STATE,
	MODBUS_ADDR_DAU_FAULT,
	MODBUS_ADDR_START_RELAY_BOARD_CHECK,
	MODBUS_ADDR_RELAY_BOARD_CHECK_FAULT_CHANNEL_ID,
	MODBUS_ADDR_RELAY_BOARD_CHECK_FAULT_MODULE_ID,

	MODBUS_ADDR_PDU_POLICY,

	MODBUS_ADDR_MODULE_MAX_OUTPUT_POWER,

	MODBUS_ADDR_VER_MAJOR,
	MODBUS_ADDR_VER_MINOR,
	MODBUS_ADDR_VER_REV,

	MODBUS_ADDR_PDU_GROUP1_POWER_MODULE_NUMBER_PER_POWER_MODULE_GROUP,
	MODBUS_ADDR_PDU_GROUP2_POWER_MODULE_NUMBER_PER_POWER_MODULE_GROUP,
	MODBUS_ADDR_PDU_GROUP_FAULT,
	MODBUS_ADDR_RELAY_FAULT_ID,

	MODBUS_ADDR_SIZE = addr_current_max,
} modbus_slave_addr_t;

char *get_modbus_slave_addr_des(modbus_slave_addr_t addr);
void channels_modbus_data_action(void *fn_ctx, void *chain_ctx);

#endif //_MODBUS_ADDR_HANDLER_H
