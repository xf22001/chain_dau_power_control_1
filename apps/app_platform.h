

/*================================================================
 *   
 *   
 *   文件名称：app_platform.h
 *   创 建 者：肖飞
 *   创建日期：2020年03月20日 星期五 09时12分39秒
 *   修改日期：2022年03月18日 星期五 17时33分04秒
 *   描    述：
 *
 *================================================================*/
#ifndef _APP_PLATFORM_H
#define _APP_PLATFORM_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f4xx_hal.h"

#define CCMRAM __attribute__ ((section (".ccmram")))

#ifdef __cplusplus
}
#endif

#endif //_APP_PLATFORM_H
