/*
 * weather.h
 *
 *  Created on: 13.06.2018
 *      Author: Karol
 */

#ifndef APP_WEATHER_H_
#define APP_WEATHER_H_


#define WEATHER_LINE_LEN 25

typedef struct
{
	int temp, wind_speed, wind_deg, humidity, pressure;
	char description[WEATHER_LINE_LEN];
	char msg[WEATHER_LINE_LEN];
	char localization[WEATHER_LINE_LEN];
	char status1[WEATHER_LINE_LEN];
	char status2[WEATHER_LINE_LEN];
}  weather_struct;


#endif /* APP_WEATHER_H_ */
