/*
 * weather_requester.c
 *
 *  Created on: 10.06.2018
 *      Author: Karol
 */

#include "lwip.h"
#include "tcp.h"
#include "string.h"
#include "netif.h"
#include "weather.h"

weather_struct current_wheater;

static struct tcp_pcb _testpcb;
static struct tcp_pcb *testpcb = &_testpcb;

const weather_struct* wheater_get_last()
{
	return &current_wheater;
}

uint32_t tcp_send_packet(void)
{
    char *string =
    		"GET /data/2.5/weather?id=3099434&appid=dc9bedfedd6ab50770a9f0a23ad34f3c&units=metric HTTP/1.1\r\n"
    		"Host: api.openweathermap.org\r\n"
    		"\r\n ";
    uint32_t len = strlen(string);

    /* push to buffer */
	err_t error = tcp_write(testpcb, string, len, TCP_WRITE_FLAG_COPY);

    if (error) {
    	sprintf(current_wheater.msg, "ERROR: Code: %d (tcp_send_packet :: tcp_write)\n", error);
        return 1;
    }

    /* now send */
    error = tcp_output(testpcb);
    if (error) {
    	sprintf(current_wheater.msg, "ERROR: Code: %d (tcp_send_packet :: tcp_output)\n", error);
        return 1;
    }
    return 0;
}

/* connection established callback, err is unused and only return 0 */
err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	strcpy(current_wheater.msg, "Connected");
    tcp_send_packet();
    return 0;
}

char* json_find(const char* data, const char* key, int* len)
{
	char *res_start;
	int key_len = strlen(key);
	int i;
	while(data[0] != 0){
		i = 0;
		while(data[i] == key[i] && key[i] != 0) {
			++i;
		}

		if(i != key_len){
			data += i + 1;
		} else {
			i += 1; // ommit "
			if(data[i] != ':') ++i;
			++i; // ommit :
			if(data[i] == '"') ++i;
			res_start = &data[i];
			while(data[i] != '"' && data[i] != ',' && data[i] != 0) ++i;
			*len = (int)(&data[i] - res_start) + 1;
			return res_start;
		}
	}
	return 0;
}

err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL) {
    	//sprintf(current_wheater.msg + strlen(current_wheater.msg), "Connection closed");
        tcp_close(tpcb);
        return ERR_ABRT;
    } else {
    	char *data;
    	int len;

    	data = json_find((char *)p->payload, "name", &len);
    	if(data != 0) strncpy(current_wheater.localization, data, len);

    	data = json_find((char *)p->payload, "temp", &len);
    	if(data != 0) current_wheater.temp = atoi(data);

    	data = json_find((char *)p->payload, "pressure", &len);
    	if(data != 0) current_wheater.pressure = atoi(data);

    	data = json_find((char *)p->payload, "speed", &len);
    	if(data != 0) current_wheater.wind_speed = atoi(data);

    	data = json_find((char *)p->payload, "deg", &len);
    	if(data != 0) current_wheater.wind_deg = atoi(data);

    	data = json_find((char *)p->payload, "humidity", &len);
    	if(data != 0) current_wheater.humidity = atoi(data);

    	data = json_find((char *)p->payload, "main\"", &len);
    	if(data != 0) strncpy(current_wheater.description, data-1, len);

    	snprintf(current_wheater.msg, WEATHER_LINE_LEN, "%s %s", current_wheater.localization, current_wheater.description);
    	snprintf(current_wheater.status1, WEATHER_LINE_LEN, "%2d*C %3d%% %4dhPa", current_wheater.temp, current_wheater.humidity, current_wheater.pressure);
    	snprintf(current_wheater.status2, WEATHER_LINE_LEN, "wind %2dm/s %3ddeg", current_wheater.wind_speed, current_wheater.wind_deg);
    }

    return 0;
}


void tcpErrorHandler(void *arg, err_t err)
{
	sprintf(current_wheater.msg, "Error %d", err);
}


err_t tcpSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	sprintf(current_wheater.msg, "Tcp send %d", len);
	return 0;
}


void weather_request()
{
	/* create an ip */
	ip_addr_t ip;

	//IP4_ADDR(&ip, 192,168,1,166);    //IP of my PHP server
	IP4_ADDR(&ip, 162,243,53,59);    //IP of api.openweathermap.org
	//IP4_ADDR(&ip, 192,241,169,168);    //IP of api.openweathermap.org
	//IP4_ADDR(&ip, 34,237,148,235);    //IP of api.openweathermap.org

	/* now connect */
	tcp_connect(testpcb, &ip, 80, connectCallback);
}

void weather_request_task_init()
{
	/* create the control block */
	testpcb = tcp_new();    //testpcb is a global struct tcp_pcb
							// as defined by lwIP

	/* dummy data to pass to callbacks*/

	uint32_t data = 0xdeadbeef;
	tcp_arg(testpcb, &data);

	/* register callbacks with the pcb */
	tcp_err(testpcb, tcpErrorHandler);
	tcp_recv(testpcb, tcpRecvCallback);
	tcp_sent(testpcb, tcpSendCallback);
}

void weather_request_task(void const* arguments)
{
	weather_request_task_init();

	do {
		weather_request();
		osDelay(10*60*1000);
	} while(0);
}
