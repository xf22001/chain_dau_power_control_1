#
#
#================================================================
#   
#   
#   文件名称：config.mk
#   创 建 者：肖飞
#   创建日期：2021年08月26日 星期四 11时10分19秒
#   修改日期：2022年06月02日 星期四 16时28分05秒
#   描    述：
#
#================================================================

CONFIG_LIST := 

#CONFIG_LIST += USER_APP
#CONFIG_LIST += PSEUDO_ENV

CONFIG_LIST += IAP_INTERNAL_FLASH

CONFIG_LIST += STORAGE_OPS_25LC1024
#CONFIG_LIST += STORAGE_OPS_24LC128
#CONFIG_LIST += STORAGE_OPS_W25Q256

#CONFIG_LIST += CHARGER_CHANNEL_NATIVE
CONFIG_LIST += CHARGER_CHANNEL_PROXY_REMOTE
#CONFIG_LIST += CHARGER_CHANNEL_PROXY_LOCAL

CONFIG_LIST += SAL_WIZNET
#CONFIG_LIST += SAL_AT
#CONFIG_LIST += SAL_DTU

#路特斯
CONFIG_LIST += POWER_MANAGER_GROUP_POLICY_1

#10枪环形
#CONFIG_LIST += POWER_MANAGER_GROUP_POLICY_2

$(foreach config_item,$(CONFIG_LIST),$(eval $(addprefix CONFIG_,$(config_item)) := $(config_item)))

CONFIG_CFLAGS := $(addprefix -D,$(CONFIG_LIST))
