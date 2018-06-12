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

typedef struct
{
	int temp;
	char msg[50];
}  weather_struct;

weather_struct current_wheater;

static struct tcp_pcb *testpcb;

uint32_t tcp_send_packet(void)
{
    char *string =
    		"HEAD /process.php?data1=12&data2=5 HTTP/1.0\r\n"
    		"Host: mywebsite.com\r\n"
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

err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL) {
    	sprintf(current_wheater.msg, "Connection closed");
        tcp_close(tpcb);
        return ERR_ABRT;
    } else {
    	snprintf(current_wheater.msg, 50, "received %dB: %s", p->tot_len, (char *)p->payload);
    }

    return 0;
}


void tcpErrorHandler(void *arg, err_t err)
{}


err_t tcpSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	return 0;
}


void weather_request()
{
	uint32_t data = 0xdeadbeef;

	/* create an ip */
	ip_addr_t ip;

	IP4_ADDR(&ip, 110,777,888,999);    //IP of my PHP server

	/* create the control block */
	testpcb = tcp_new();    //testpcb is a global struct tcp_pcb
							// as defined by lwIP


	/* dummy data to pass to callbacks*/

	tcp_arg(testpcb, &data);

	/* register callbacks with the pcb */
	tcp_err(testpcb, tcpErrorHandler);
	tcp_recv(testpcb, tcpRecvCallback);
	tcp_sent(testpcb, tcpSendCallback);

	/* now connect */
	tcp_connect(testpcb, &ip, 80, connectCallback);
}
