

/*================================================================
 *
 *
 *   文件名称：modbus_addr_handler.c
 *   创 建 者：肖飞
 *   创建日期：2020年07月17日 星期五 10时13分49秒
 *   修改日期：2022年03月15日 星期二 11时50分01秒
 *   描    述：
 *
 *================================================================*/
#include "modbus_addr_handler.h"
#include "app.h"
#include "channels.h"
#include "power_manager.h"
#include "modbus_data_value.h"
#include "modbus_slave_txrx.h"
#include "channels_comm_proxy_remote.h"

#include "log.h"

#define add_enum_module_info_des_case(MODULE_ID) \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_PDU_GROUP_ID); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_CHANNEL_ID); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_RELAY_BOARD_ID); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_WORK_STATE); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_VOLTAGE); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_CURRENT); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_VOLTAGE); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_CURRENT); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_MODULE_STATE); \
	add_des_case(MODBUS_ADDR_MODULE_##MODULE_ID##_CONNECT_STATE)

#define add_enum_channel_info_des_case(CHANNEL_ID) \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_PDU_GROUP_ID); \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_VOLTAGE); \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_CURRENT); \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_VOLTAGE); \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_CURRENT); \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_WORK_STATE); \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_CONNECT_STATE); \
	add_des_case(MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_FAULT)

#define add_enum_relay_board_info_des_case(RELAY_BOARD_ID) \
	add_des_case(MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CHANNEL_ID); \
	add_des_case(MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONFIG); \
	add_des_case(MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_WORK_STATE); \
	add_des_case(MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_OFFSET); \
	add_des_case(MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_NUMBER); \
	add_des_case(MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONNECT_STATE); \
	add_des_case(MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_FAULT)

char *get_modbus_slave_addr_des(modbus_slave_addr_t addr)
{
	char *des = "unknow";

	switch(addr) {
			add_enum_module_info_des_case(0);
			add_enum_module_info_des_case(1);
			add_enum_module_info_des_case(2);
			add_enum_module_info_des_case(3);
			add_enum_module_info_des_case(4);
			add_enum_module_info_des_case(5);
			add_enum_module_info_des_case(6);
			add_enum_module_info_des_case(7);
			add_enum_module_info_des_case(8);
			add_enum_module_info_des_case(9);
			add_enum_module_info_des_case(10);
			add_enum_module_info_des_case(11);
			add_enum_module_info_des_case(12);
			add_enum_module_info_des_case(13);
			add_enum_module_info_des_case(14);

			add_enum_channel_info_des_case(0);
			add_enum_channel_info_des_case(1);
			add_enum_channel_info_des_case(2);
			add_enum_channel_info_des_case(3);
			add_enum_channel_info_des_case(4);
			add_enum_channel_info_des_case(5);
			add_enum_channel_info_des_case(6);
			add_enum_channel_info_des_case(7);
			add_enum_channel_info_des_case(8);
			add_enum_channel_info_des_case(9);
			add_enum_channel_info_des_case(10);
			add_enum_channel_info_des_case(11);

			add_enum_relay_board_info_des_case(0);
			add_enum_relay_board_info_des_case(1);
			add_enum_relay_board_info_des_case(2);
			add_enum_relay_board_info_des_case(3);
			add_enum_relay_board_info_des_case(4);
			add_enum_relay_board_info_des_case(5);
			add_enum_relay_board_info_des_case(6);
			add_enum_relay_board_info_des_case(7);
			add_enum_relay_board_info_des_case(8);
			add_enum_relay_board_info_des_case(9);
			add_enum_relay_board_info_des_case(10);
			add_enum_relay_board_info_des_case(11);
			add_enum_relay_board_info_des_case(12);
			add_enum_relay_board_info_des_case(13);
			add_enum_relay_board_info_des_case(14);
			add_enum_relay_board_info_des_case(15);
			add_enum_relay_board_info_des_case(16);
			add_enum_relay_board_info_des_case(17);
			add_enum_relay_board_info_des_case(18);
			add_enum_relay_board_info_des_case(19);
			add_enum_relay_board_info_des_case(20);
			add_enum_relay_board_info_des_case(21);
			add_enum_relay_board_info_des_case(22);
			add_enum_relay_board_info_des_case(23);

			add_des_case(MODBUS_ADDR_PDU_GROUP_NUMBER);
			add_des_case(MODBUS_ADDR_PDU_GROUP1_CHANNEL_NUMBER);
			add_des_case(MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD_NUMBER_PER_CHANNEL);
			add_des_case(MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD1_MODULE_NUMBER_PER_CHANNEL);
			add_des_case(MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD2_MODULE_NUMBER_PER_CHANNEL);
			add_des_case(MODBUS_ADDR_PDU_GROUP2_CHANNEL_NUMBER);
			add_des_case(MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD_NUMBER_PER_CHANNEL);
			add_des_case(MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD1_MODULE_NUMBER_PER_CHANNEL);
			add_des_case(MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD2_MODULE_NUMBER_PER_CHANNEL);

			add_des_case(MODBUS_ADDR_MODULE_MAX_OUTPUT_VOLTAGE);
			add_des_case(MODBUS_ADDR_MODULE_MIN_OUTPUT_VOLTAGE);
			add_des_case(MODBUS_ADDR_MODULE_MAX_OUTPUT_CURRENT);
			add_des_case(MODBUS_ADDR_MODULE_MIN_OUTPUT_CURRENT);
			add_des_case(MODBUS_ADDR_MODULE_TYPE);

			add_des_case(MODBUS_ADDR_DAU_WORK_STATE);
			add_des_case(MODBUS_ADDR_DAU_FAULT);
			add_des_case(MODBUS_ADDR_START_RELAY_BOARD_CHECK);
			add_des_case(MODBUS_ADDR_RELAY_BOARD_CHECK_FAULT_CHANNEL_ID);
			add_des_case(MODBUS_ADDR_RELAY_BOARD_CHECK_FAULT_MODULE_ID);
			add_des_case(MODBUS_ADDR_PDU_POLICY);
			add_des_case(MODBUS_ADDR_MODULE_MAX_OUTPUT_POWER);
			add_des_case(MODBUS_ADDR_VER_MAJOR);
			add_des_case(MODBUS_ADDR_VER_MINOR);
			add_des_case(MODBUS_ADDR_VER_REV);

		default: {
		}
		break;
	}

	return des;
}

typedef struct {
	uint8_t id;
	uint8_t field;
} enum_info_t;

typedef enum {
	MODULE_FIELD_TYPE_PDU_GROUP_ID = 0,
	MODULE_FIELD_TYPE_CHANNEL_ID,
	MODULE_FIELD_TYPE_RELAY_BOARD_ID,
	MODULE_FIELD_TYPE_WORK_STATE,
	MODULE_FIELD_TYPE_SETTING_VOLTAGE,
	MODULE_FIELD_TYPE_SETTING_CURRENT,
	MODULE_FIELD_TYPE_OUTPUT_VOLTAGE,
	MODULE_FIELD_TYPE_OUTPUT_CURRENT,
	MODULE_FIELD_TYPE_MODULE_STATE,
	MODULE_FIELD_TYPE_CONNECT_STATE,
} module_field_type_t;

static const char *get_module_field_type_des(module_field_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(MODULE_FIELD_TYPE_PDU_GROUP_ID);
			add_des_case(MODULE_FIELD_TYPE_CHANNEL_ID);
			add_des_case(MODULE_FIELD_TYPE_RELAY_BOARD_ID);
			add_des_case(MODULE_FIELD_TYPE_WORK_STATE);
			add_des_case(MODULE_FIELD_TYPE_SETTING_VOLTAGE);
			add_des_case(MODULE_FIELD_TYPE_SETTING_CURRENT);
			add_des_case(MODULE_FIELD_TYPE_OUTPUT_VOLTAGE);
			add_des_case(MODULE_FIELD_TYPE_OUTPUT_CURRENT);
			add_des_case(MODULE_FIELD_TYPE_MODULE_STATE);
			add_des_case(MODULE_FIELD_TYPE_CONNECT_STATE);

		default: {
		}
		break;
	}

	return des;
}

#define add_get_module_enum_info_case(MODULE_ID, enum_info) \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_PDU_GROUP_ID: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_PDU_GROUP_ID; \
	} \
	break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_CHANNEL_ID: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_CHANNEL_ID; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_RELAY_BOARD_ID: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_RELAY_BOARD_ID; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_WORK_STATE: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_WORK_STATE; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_VOLTAGE: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_SETTING_VOLTAGE; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_CURRENT: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_SETTING_CURRENT; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_VOLTAGE: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_OUTPUT_VOLTAGE; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_CURRENT: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_OUTPUT_CURRENT; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_MODULE_STATE: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_MODULE_STATE; \
	} break; \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_CONNECT_STATE: { \
		enum_info->id = MODULE_ID; \
		enum_info->field = MODULE_FIELD_TYPE_CONNECT_STATE; \
	} break

static void get_module_enum_info(modbus_slave_addr_t addr, enum_info_t *enum_info)
{
	switch(addr) {
			add_get_module_enum_info_case(0, enum_info);
			add_get_module_enum_info_case(1, enum_info);
			add_get_module_enum_info_case(2, enum_info);
			add_get_module_enum_info_case(3, enum_info);
			add_get_module_enum_info_case(4, enum_info);
			add_get_module_enum_info_case(5, enum_info);
			add_get_module_enum_info_case(6, enum_info);
			add_get_module_enum_info_case(7, enum_info);
			add_get_module_enum_info_case(8, enum_info);
			add_get_module_enum_info_case(9, enum_info);
			add_get_module_enum_info_case(10, enum_info);
			add_get_module_enum_info_case(11, enum_info);
			add_get_module_enum_info_case(12, enum_info);
			add_get_module_enum_info_case(13, enum_info);
			add_get_module_enum_info_case(14, enum_info);

		default: {
			debug("not handle addr %s", get_modbus_slave_addr_des(addr));
			enum_info->id = 0xff;
			enum_info->field = 0xff;
		}
		break;
	}
}

typedef enum {
	CHANNEL_FIELD_TYPE_PDU_GROUP_ID = 0,
	CHANNEL_FIELD_TYPE_REQUIRE_VOLTAGE,
	CHANNEL_FIELD_TYPE_REQUIRE_CURRENT,
	CHANNEL_FIELD_TYPE_OUTPUT_VOLTAGE,
	CHANNEL_FIELD_TYPE_OUTPUT_CURRENT,
	CHANNEL_FIELD_TYPE_WORK_STATE,
	CHANNEL_FIELD_TYPE_CONNECT_STATE,
	CHANNEL_FIELD_TYPE_FAULT,
} channel_field_type_t;

static const char *get_channel_field_type_des(module_field_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(CHANNEL_FIELD_TYPE_PDU_GROUP_ID);
			add_des_case(CHANNEL_FIELD_TYPE_REQUIRE_VOLTAGE);
			add_des_case(CHANNEL_FIELD_TYPE_REQUIRE_CURRENT);
			add_des_case(CHANNEL_FIELD_TYPE_OUTPUT_VOLTAGE);
			add_des_case(CHANNEL_FIELD_TYPE_OUTPUT_CURRENT);
			add_des_case(CHANNEL_FIELD_TYPE_WORK_STATE);
			add_des_case(CHANNEL_FIELD_TYPE_CONNECT_STATE);
			add_des_case(CHANNEL_FIELD_TYPE_FAULT);

		default: {
		}
		break;
	}

	return des;
}

#define add_get_channel_enum_info_case(CHANNEL_ID, enum_info) \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_PDU_GROUP_ID: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_PDU_GROUP_ID; \
	} \
	break; \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_VOLTAGE: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_REQUIRE_VOLTAGE; \
	} break; \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_CURRENT: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_REQUIRE_CURRENT; \
	} break; \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_VOLTAGE: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_OUTPUT_VOLTAGE; \
	} break; \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_CURRENT: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_OUTPUT_CURRENT; \
	} break; \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_WORK_STATE: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_WORK_STATE; \
	} break; \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_CONNECT_STATE: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_CONNECT_STATE; \
	} break; \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_FAULT: { \
		enum_info->id = CHANNEL_ID; \
		enum_info->field = CHANNEL_FIELD_TYPE_FAULT; \
	} break

static void get_channel_enum_info(modbus_slave_addr_t addr, enum_info_t *enum_info)
{
	switch(addr) {
			add_get_channel_enum_info_case(0, enum_info);
			add_get_channel_enum_info_case(1, enum_info);
			add_get_channel_enum_info_case(2, enum_info);
			add_get_channel_enum_info_case(3, enum_info);
			add_get_channel_enum_info_case(4, enum_info);
			add_get_channel_enum_info_case(5, enum_info);
			add_get_channel_enum_info_case(6, enum_info);
			add_get_channel_enum_info_case(7, enum_info);
			add_get_channel_enum_info_case(8, enum_info);
			add_get_channel_enum_info_case(9, enum_info);
			add_get_channel_enum_info_case(10, enum_info);
			add_get_channel_enum_info_case(11, enum_info);

		default: {
			debug("not handle addr %s", get_modbus_slave_addr_des(addr));
			enum_info->id = 0xff;
			enum_info->field = 0xff;
		}
		break;
	}
}

typedef enum {
	RELAY_BOARD_FIELD_TYPE_CHANNEL_ID = 0,
	RELAY_BOARD_FIELD_TYPE_CONFIG,
	RELAY_BOARD_FIELD_TYPE_WORK_STATE,
	RELAY_BOARD_FIELD_TYPE_OFFSET,
	RELAY_BOARD_FIELD_TYPE_NUMBER,
	RELAY_BOARD_FIELD_TYPE_CONNECT_STATE,
	RELAY_BOARD_FIELD_TYPE_FAULT,
} relay_board_field_type_t;

static const char *get_relay_board_field_type_des(module_field_type_t type)
{
	char *des = "unknow";

	switch(type) {
			add_des_case(RELAY_BOARD_FIELD_TYPE_CHANNEL_ID);
			add_des_case(RELAY_BOARD_FIELD_TYPE_CONFIG);
			add_des_case(RELAY_BOARD_FIELD_TYPE_WORK_STATE);
			add_des_case(RELAY_BOARD_FIELD_TYPE_OFFSET);
			add_des_case(RELAY_BOARD_FIELD_TYPE_NUMBER);
			add_des_case(RELAY_BOARD_FIELD_TYPE_CONNECT_STATE);
			add_des_case(RELAY_BOARD_FIELD_TYPE_FAULT);

		default: {
		}
		break;
	}

	return des;
}

#define add_get_relay_board_enum_info_case(RELAY_BOARD_ID, enum_info) \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CHANNEL_ID: { \
		enum_info->id = RELAY_BOARD_ID; \
		enum_info->field = RELAY_BOARD_FIELD_TYPE_CHANNEL_ID; \
	} \
	break; \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONFIG: { \
		enum_info->id = RELAY_BOARD_ID; \
		enum_info->field = RELAY_BOARD_FIELD_TYPE_CONFIG; \
	} \
	break; \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_WORK_STATE: { \
		enum_info->id = RELAY_BOARD_ID; \
		enum_info->field = RELAY_BOARD_FIELD_TYPE_WORK_STATE; \
	} \
	break; \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_OFFSET: { \
		enum_info->id = RELAY_BOARD_ID; \
		enum_info->field = RELAY_BOARD_FIELD_TYPE_OFFSET; \
	} \
	break; \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_NUMBER: { \
		enum_info->id = RELAY_BOARD_ID; \
		enum_info->field = RELAY_BOARD_FIELD_TYPE_NUMBER; \
	} \
	break; \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONNECT_STATE: { \
		enum_info->id = RELAY_BOARD_ID; \
		enum_info->field = RELAY_BOARD_FIELD_TYPE_CONNECT_STATE; \
	} \
	break; \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_FAULT: { \
		enum_info->id = RELAY_BOARD_ID; \
		enum_info->field = RELAY_BOARD_FIELD_TYPE_FAULT; \
	} \
	break

static void get_relay_board_enum_info(modbus_slave_addr_t addr, enum_info_t *enum_info)
{
	switch(addr) {
			add_get_relay_board_enum_info_case(0, enum_info);
			add_get_relay_board_enum_info_case(1, enum_info);
			add_get_relay_board_enum_info_case(2, enum_info);
			add_get_relay_board_enum_info_case(3, enum_info);
			add_get_relay_board_enum_info_case(4, enum_info);
			add_get_relay_board_enum_info_case(5, enum_info);
			add_get_relay_board_enum_info_case(6, enum_info);
			add_get_relay_board_enum_info_case(7, enum_info);
			add_get_relay_board_enum_info_case(8, enum_info);
			add_get_relay_board_enum_info_case(9, enum_info);
			add_get_relay_board_enum_info_case(10, enum_info);
			add_get_relay_board_enum_info_case(11, enum_info);
			add_get_relay_board_enum_info_case(12, enum_info);
			add_get_relay_board_enum_info_case(13, enum_info);
			add_get_relay_board_enum_info_case(14, enum_info);
			add_get_relay_board_enum_info_case(15, enum_info);
			add_get_relay_board_enum_info_case(16, enum_info);
			add_get_relay_board_enum_info_case(17, enum_info);
			add_get_relay_board_enum_info_case(18, enum_info);
			add_get_relay_board_enum_info_case(19, enum_info);
			add_get_relay_board_enum_info_case(20, enum_info);
			add_get_relay_board_enum_info_case(21, enum_info);
			add_get_relay_board_enum_info_case(22, enum_info);
			add_get_relay_board_enum_info_case(23, enum_info);

		default: {
			debug("not handle addr %s", get_modbus_slave_addr_des(addr));
			enum_info->id = 0xff;
			enum_info->field = 0xff;
		}
		break;
	}
}

#define add_modbus_data_get_set_module_info_case(MODULE_ID) \
	MODBUS_ADDR_MODULE_##MODULE_ID##_PDU_GROUP_ID: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_CHANNEL_ID: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_RELAY_BOARD_ID: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_WORK_STATE: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_VOLTAGE: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_REQUIRE_CURRENT: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_VOLTAGE: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_OUTPUT_CURRENT: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_MODULE_STATE: \
	case MODBUS_ADDR_MODULE_##MODULE_ID##_CONNECT_STATE

#define add_modbus_data_get_set_channel_info_case(CHANNEL_ID) \
	MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_PDU_GROUP_ID: \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_VOLTAGE: \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_REQUIRE_CURRENT: \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_VOLTAGE: \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_OUTPUT_CURRENT: \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_WORK_STATE: \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_CONNECT_STATE: \
	case MODBUS_ADDR_CHANNEL_##CHANNEL_ID##_FAULT

#define add_modbus_data_get_set_relay_board_info_case(RELAY_BOARD_ID) \
	MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CHANNEL_ID: \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONFIG: \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_WORK_STATE: \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_OFFSET: \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_NUMBER: \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_CONNECT_STATE: \
	case MODBUS_ADDR_RELAY_BOARD_##RELAY_BOARD_ID##_FAULT

static uint16_t get_module_info_by_id_field(channels_info_t *channels_info, uint8_t module_id, uint8_t field)
{
	power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;
	power_module_item_info_t *power_module_item_info = power_manager_info->power_module_item_info + module_id;

	if(module_id >= channels_info->power_module_number) {
		return 0xffff;
	}

	switch(field) {
		case MODULE_FIELD_TYPE_PDU_GROUP_ID: {
			power_module_group_info_t *power_module_group_info = (power_module_group_info_t *)power_module_item_info->power_module_group_info;
			power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_module_group_info->power_manager_group_info;
			return power_manager_group_info->id;
		}
		break;

		case MODULE_FIELD_TYPE_CHANNEL_ID: {
			power_module_group_info_t *power_module_group_info = (power_module_group_info_t *)power_module_item_info->power_module_group_info;
			power_manager_channel_info_t *power_manager_channel_info = (power_manager_channel_info_t *)power_module_group_info->power_manager_channel_info;

			if(power_manager_channel_info != NULL) {
				return power_manager_channel_info->id;
			} else {
				return 0xffff;
			}
		}
		break;

		case MODULE_FIELD_TYPE_RELAY_BOARD_ID: {
			return 0xffff;
		}
		break;

		case MODULE_FIELD_TYPE_WORK_STATE: {
			return power_module_item_info->status.state;
		}
		break;

		case MODULE_FIELD_TYPE_SETTING_VOLTAGE: {
			return power_module_item_info->status.setting_output_voltage;
		}
		break;

		case MODULE_FIELD_TYPE_SETTING_CURRENT: {
			return power_module_item_info->status.require_output_current;
		}
		break;

		case MODULE_FIELD_TYPE_OUTPUT_VOLTAGE: {
			return power_module_item_info->status.module_output_voltage;
		}
		break;

		case MODULE_FIELD_TYPE_OUTPUT_CURRENT: {
			return power_module_item_info->status.module_output_current;
		}
		break;

		case MODULE_FIELD_TYPE_MODULE_STATE: {
			return power_module_item_info->status.module_status;
		}
		break;

		case MODULE_FIELD_TYPE_CONNECT_STATE: {
			return power_module_item_info->status.connect_state;
		}
		break;

		default: {
			return 0xffff;
		}
		break;
	}
}

static uint16_t get_channnel_info_by_id_field(channels_info_t *channels_info, uint8_t channel_id, uint8_t field)
{
	power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;
	channel_info_t *channel_info = channels_info->channel_info + channel_id;
	power_manager_channel_info_t *power_manager_channel_info = power_manager_info->power_manager_channel_info + channel_id;

	if(channel_id >= channels_info->channel_number) {
		return 0xffff;
	}

	switch(field) {
		case CHANNEL_FIELD_TYPE_PDU_GROUP_ID: {
			power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_manager_channel_info->power_manager_group_info;
			return power_manager_group_info->id;
		}
		break;

		case CHANNEL_FIELD_TYPE_REQUIRE_VOLTAGE: {
			return power_manager_channel_info->status.require_output_voltage;
		}
		break;

		case CHANNEL_FIELD_TYPE_REQUIRE_CURRENT: {
			return power_manager_channel_info->status.require_output_current;
		}
		break;

		case CHANNEL_FIELD_TYPE_OUTPUT_VOLTAGE: {
			return power_manager_channel_info->status.charge_output_voltage;
		}
		break;

		case CHANNEL_FIELD_TYPE_OUTPUT_CURRENT: {
			return power_manager_channel_info->status.charge_output_current;
		}
		break;

		case CHANNEL_FIELD_TYPE_WORK_STATE: {
			return power_manager_channel_info->status.state;
		}
		break;

		case CHANNEL_FIELD_TYPE_CONNECT_STATE: {
			channels_config_t *channels_config = channels_info->channels_config;
			proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_channel_id(&channels_config->proxy_channel_info, channel_id);

			if(proxy_channel_item == NULL) {
				return 0xffff;
			} else {
				return channels_comm_proxy_get_connect_state(channels_info, proxy_channel_item->proxy_channel_index);
			}
		}
		break;

		case CHANNEL_FIELD_TYPE_FAULT: {
			uint8_t fault = *channel_info->faults->data;
			return fault;
		}
		break;

		default: {
			return 0xffff;
		}
		break;
	}
}

//static uint16_t get_relay_board_info_by_id_field(channels_info_t *channels_info, uint8_t board_id, uint8_t field)
//{
//	relay_board_item_info_t *relay_board_item_info = channels_info->relay_board_item_info + board_id;
//
//	if(board_id >= channels_info->relay_board_item_number) {
//		return 0xffff;
//	}
//
//	switch(field) {
//		case RELAY_BOARD_FIELD_TYPE_CHANNEL_ID: {
//			return relay_board_item_info->channel_id;
//		}
//		break;
//
//		case RELAY_BOARD_FIELD_TYPE_CONFIG: {
//			return relay_board_item_info->config;
//		}
//		break;
//
//		case RELAY_BOARD_FIELD_TYPE_WORK_STATE: {
//			return relay_board_item_info->remote_config;
//		}
//		break;
//
//		case RELAY_BOARD_FIELD_TYPE_OFFSET: {
//			return relay_board_item_info->offset;
//		}
//		break;
//
//		case RELAY_BOARD_FIELD_TYPE_NUMBER: {
//			return relay_board_item_info->number;
//		}
//		break;
//
//		case RELAY_BOARD_FIELD_TYPE_CONNECT_STATE: {
//			return relay_board_item_info->connect_state;
//		}
//		break;
//
//		case RELAY_BOARD_FIELD_TYPE_FAULT: {
//			uint8_t fault = *relay_board_item_info->faults->data;
//			return fault;
//		}
//		break;
//
//		default: {
//			return 0xffff;
//		}
//		break;
//	}
//}

void channels_modbus_data_action(void *fn_ctx, void *chain_ctx)
{
	channels_info_t *channels_info = (channels_info_t *)fn_ctx;
	modbus_data_ctx_t *modbus_data_ctx = (modbus_data_ctx_t *)chain_ctx;

	switch(modbus_data_ctx->addr) {
		case add_modbus_data_get_set_module_info_case(0):
		case add_modbus_data_get_set_module_info_case(1):
		case add_modbus_data_get_set_module_info_case(2):
		case add_modbus_data_get_set_module_info_case(3):
		case add_modbus_data_get_set_module_info_case(4):
		case add_modbus_data_get_set_module_info_case(5):
		case add_modbus_data_get_set_module_info_case(6):
		case add_modbus_data_get_set_module_info_case(7):
		case add_modbus_data_get_set_module_info_case(8):
		case add_modbus_data_get_set_module_info_case(9):
		case add_modbus_data_get_set_module_info_case(10):
		case add_modbus_data_get_set_module_info_case(11):
		case add_modbus_data_get_set_module_info_case(12):
		case add_modbus_data_get_set_module_info_case(13):
		case add_modbus_data_get_set_module_info_case(14): {
			enum_info_t module_enum_info;
			get_module_enum_info(modbus_data_ctx->addr, &module_enum_info);
			//debug("%s module_id %d field %s",
			//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) ? "get" :
			//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) ? "set" :
			//      "unknow",
			//      module_enum_info.id,
			//      get_module_field_type_des(module_enum_info.field));

			modbus_data_value_r(modbus_data_ctx, get_module_info_by_id_field(channels_info, module_enum_info.id, module_enum_info.field));
		}
		break;

		case add_modbus_data_get_set_channel_info_case(0):
		case add_modbus_data_get_set_channel_info_case(1):
		case add_modbus_data_get_set_channel_info_case(2):
		case add_modbus_data_get_set_channel_info_case(3):
		case add_modbus_data_get_set_channel_info_case(4):
		case add_modbus_data_get_set_channel_info_case(5):
		case add_modbus_data_get_set_channel_info_case(6):
		case add_modbus_data_get_set_channel_info_case(7):
		case add_modbus_data_get_set_channel_info_case(8):
		case add_modbus_data_get_set_channel_info_case(9):
		case add_modbus_data_get_set_channel_info_case(10):
		case add_modbus_data_get_set_channel_info_case(11): {
			enum_info_t channel_enum_info;
			get_channel_enum_info(modbus_data_ctx->addr, &channel_enum_info);
			//debug("%s channel_id %d field %s",
			//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) ? "get" :
			//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) ? "set" :
			//      "unknow",
			//      channel_enum_info.id,
			//      get_channel_field_type_des(channel_enum_info.field));
			modbus_data_value_r(modbus_data_ctx, get_channnel_info_by_id_field(channels_info, channel_enum_info.id, channel_enum_info.field));
		}
		break;

		case add_modbus_data_get_set_relay_board_info_case(0):
		case add_modbus_data_get_set_relay_board_info_case(1):
		case add_modbus_data_get_set_relay_board_info_case(2):
		case add_modbus_data_get_set_relay_board_info_case(3):
		case add_modbus_data_get_set_relay_board_info_case(4):
		case add_modbus_data_get_set_relay_board_info_case(5):
		case add_modbus_data_get_set_relay_board_info_case(6):
		case add_modbus_data_get_set_relay_board_info_case(7):
		case add_modbus_data_get_set_relay_board_info_case(8):
		case add_modbus_data_get_set_relay_board_info_case(9):
		case add_modbus_data_get_set_relay_board_info_case(10):
		case add_modbus_data_get_set_relay_board_info_case(11):
		case add_modbus_data_get_set_relay_board_info_case(12):
		case add_modbus_data_get_set_relay_board_info_case(13):
		case add_modbus_data_get_set_relay_board_info_case(14):
		case add_modbus_data_get_set_relay_board_info_case(15):
		case add_modbus_data_get_set_relay_board_info_case(16):
		case add_modbus_data_get_set_relay_board_info_case(17):
		case add_modbus_data_get_set_relay_board_info_case(18):
		case add_modbus_data_get_set_relay_board_info_case(19):
		case add_modbus_data_get_set_relay_board_info_case(20):
		case add_modbus_data_get_set_relay_board_info_case(21):
		case add_modbus_data_get_set_relay_board_info_case(22):
		case add_modbus_data_get_set_relay_board_info_case(23): {
			enum_info_t relay_board_enum_info;
			get_relay_board_enum_info(modbus_data_ctx->addr, &relay_board_enum_info);
			//debug("%s relay_board_id %d field %s",
			//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) ? "get" :
			//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) ? "set" :
			//      "unknow",
			//      relay_board_enum_info.id,
			//      get_relay_board_field_type_des(relay_board_enum_info.field));
			//modbus_data_value_r(modbus_data_ctx, get_relay_board_info_by_id_field(channels_info, relay_board_enum_info.id, relay_board_enum_info.field));
		}
		break;

		case MODBUS_ADDR_PDU_GROUP_NUMBER: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.power_manager_settings.power_manager_group_number);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP1_CHANNEL_NUMBER: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.power_manager_settings.power_manager_group_settings[0].channel_number);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD_NUMBER_PER_CHANNEL: {
			//modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.pdu_config.pdu_group_config[0].relay_board_number_per_channel);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD1_MODULE_NUMBER_PER_CHANNEL: {
			//modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.pdu_config.pdu_group_config[0].power_module_group_number_per_relay_board[0]);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP1_RELAY_BOARD2_MODULE_NUMBER_PER_CHANNEL: {
			//modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.pdu_config.pdu_group_config[0].power_module_group_number_per_relay_board[1]);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP1_POWER_MODULE_NUMBER_PER_POWER_MODULE_GROUP: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.power_manager_settings.power_manager_group_settings[0].power_module_group_settings[0].power_module_number);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP2_CHANNEL_NUMBER: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.power_manager_settings.power_manager_group_settings[1].channel_number);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD_NUMBER_PER_CHANNEL: {
			//modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.pdu_config.pdu_group_config[1].relay_board_number_per_channel);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD1_MODULE_NUMBER_PER_CHANNEL: {
			//modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.pdu_config.pdu_group_config[1].power_module_group_number_per_relay_board[0]);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP2_RELAY_BOARD2_MODULE_NUMBER_PER_CHANNEL: {
			//modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.pdu_config.pdu_group_config[1].power_module_group_number_per_relay_board[1]);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP2_POWER_MODULE_NUMBER_PER_POWER_MODULE_GROUP: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.power_manager_settings.power_manager_group_settings[0].power_module_group_settings[1].power_module_number);
		}
		break;

		case MODBUS_ADDR_MODULE_MAX_OUTPUT_VOLTAGE: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.module_max_output_voltage);
		}
		break;

		case MODBUS_ADDR_MODULE_MIN_OUTPUT_VOLTAGE: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.module_min_output_voltage);
		}
		break;

		case MODBUS_ADDR_MODULE_MAX_OUTPUT_CURRENT: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.module_max_output_current);
		}
		break;

		case MODBUS_ADDR_MODULE_MIN_OUTPUT_CURRENT: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.module_min_output_current);
		}
		break;

		case MODBUS_ADDR_MODULE_TYPE: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->display_cache_channels.power_module_type);

			if(modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) {
				channels_info->display_cache_channels.power_module_type_sync = 1;
			}
		}
		break;

		case MODBUS_ADDR_DAU_WORK_STATE: {
			modbus_data_value_r(modbus_data_ctx, 0);
		}
		break;

		case MODBUS_ADDR_DAU_FAULT: {
			uint8_t fault = *channels_info->faults->data;
			modbus_data_value_r(modbus_data_ctx, fault);
		}
		break;

		case MODBUS_ADDR_START_RELAY_BOARD_CHECK: {
			modbus_data_value_r(modbus_data_ctx, 0);

			if(modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) {
				if(modbus_data_ctx->value == 1) {
					//start_stop_channel(channels_info, 0xff, CHANNEL_EVENT_TYPE_RELAY_CHECK);
				}
			}
		}
		break;

		case MODBUS_ADDR_RELAY_BOARD_CHECK_FAULT_CHANNEL_ID: {
			//modbus_data_value_r(modbus_data_ctx, channels_info->relay_check_info.relay_check_channel_id);
		}
		break;

		case MODBUS_ADDR_RELAY_BOARD_CHECK_FAULT_MODULE_ID: {
			//modbus_data_value_r(modbus_data_ctx, channels_info->relay_check_info.relay_check_power_module_group_id);
		}
		break;

		case MODBUS_ADDR_PDU_POLICY: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.power_manager_group_policy);
		}
		break;

		case MODBUS_ADDR_MODULE_MAX_OUTPUT_POWER: {
			modbus_data_value_rw(modbus_data_ctx, channels_info->channels_settings.module_max_output_power);
		}
		break;

		case MODBUS_ADDR_VER_MAJOR: {
			modbus_data_value_r(modbus_data_ctx, VER_MAJOR);
		}
		break;

		case MODBUS_ADDR_VER_MINOR: {
			modbus_data_value_r(modbus_data_ctx, VER_MINOR);
		}
		break;

		case MODBUS_ADDR_VER_REV: {
			modbus_data_value_r(modbus_data_ctx, VER_REV);
		}
		break;

		case MODBUS_ADDR_PDU_GROUP_FAULT: {
			modbus_data_value_r(modbus_data_ctx, 0);
		}
		break;

		case MODBUS_ADDR_RELAY_FAULT_ID: {
			power_manager_info_t *power_manager_info = (power_manager_info_t *)channels_info->power_manager_info;
			power_manager_group_info_t *power_manager_group_info = power_manager_info->power_manager_group_info + 0;
			//modbus_data_value_r(modbus_data_ctx, pdu_group_info->relay_fault_id);
		}
		break;

		default:
			debug("error! op:%s, addr:%s(%d)",
			      (modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) ? "get" :
			      (modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) ? "set" :
			      "unknow",
			      get_modbus_slave_addr_des(modbus_data_ctx->addr),
			      modbus_data_ctx->addr);
			break;
	}

	//debug("op:%s, addr:%s(%d), value:%d",
	//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_GET) ? "get" :
	//      (modbus_data_ctx->action == MODBUS_DATA_ACTION_SET) ? "set" :
	//      "unknow",
	//      get_modbus_slave_addr_des(modbus_data_ctx->addr),
	//      modbus_data_ctx->addr,
	//      modbus_data_ctx->value);
}
