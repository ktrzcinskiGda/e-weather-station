/*
 * stat_disp.c
 *
 *  Created on: 03.06.2018
 *      Author: Karol
 */


#include "STM32746G-Discovery/stm32746g_discovery_lcd.h"
#include "STM32746G-Discovery/stm32746g_discovery_ts.h"
#include "string.h"
#include "lwip.h"
#include "weather.h"

void sprintip(char *dst, uint32_t ip)
{
	sprintf(dst, "%d.%d.%d.%d",
			(uint8_t)(ip>>0),
			(uint8_t)(ip>>8),
			(uint8_t)(ip>>16),
			(uint8_t)(ip>>24));
}


void stat_disp_task(const void *argument)
{
	extern struct netif gnetif;
	extern TS_StateTypeDef ts_state;
	int ip_line = 2;
	int msg_line = 3;
	char buf[100];
	while(1) {
		sprintf(buf, "ip: ");
		sprintip(buf + strlen(buf), gnetif.ip_addr.addr);
		BSP_LCD_SelectLayer(1);
		BSP_LCD_ClearStringLine(ip_line);
		BSP_LCD_DisplayStringAtLine(ip_line, buf);

		const weather_struct* weather = wheater_get_last();
		BSP_LCD_ClearStringLine(msg_line);
		BSP_LCD_DisplayStringAtLine(msg_line, weather->msg);
		BSP_LCD_ClearStringLine(msg_line+1);
		BSP_LCD_DisplayStringAtLine(msg_line+1, weather->status1);
		BSP_LCD_ClearStringLine(msg_line+2);
		BSP_LCD_DisplayStringAtLine(msg_line+2, weather->status2);

		osDelay(100);

		BSP_TS_GetState(&ts_state);
		switch(ts_state.touchDetected) {
		case 1:
			ts_state.gestureId = GEST_ID_NO_GESTURE;
			BSP_LCD_DisplayOn();
			break;
		case 2:
			ts_state.gestureId = GEST_ID_NO_GESTURE;
			BSP_LCD_DisplayOff();
			__WFI();
			break;
		default:
			break;
		}
	}
}
