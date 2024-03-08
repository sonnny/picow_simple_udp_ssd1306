
/**
 * Copyright (c) 2022 Daniel Perron
 *
 * M.I.T.license
 * 
 * modified from https://forums.raspberrypi.com/viewtopic.php?t=340208&sid=97d2e6da53f7e442a5b72858d744d571&start=25
 * 
 * cd build
 * cmake -DPICO_BOARD=pico_w ..
 * make
 * 
 * demo udp server for pico w
 * picow should show up on attached ssd1306
 * to test on linux
 * 
 * nc -u <ip that shows up on oled> 8080 <keyboard enter>
 * type LEDON <enter> gpio17 led should turn on
 * type LEDOF <enter> gpio17 led should turn off
 * 
 */

#include <string.h>
#include <time.h>


#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "i2c_lib.h"

#define  RCV_FROM_IP IP_ADDR_ANY
#define  RCV_FROM_PORT 8080
#define BUF_SIZE 64

struct udp_pcb  * rcv_udp_pcb;

void RcvFromUDP(void * arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t*addr,u16_t port)
{
     char *buffer=calloc(p->len+1,sizeof(char));  // use calloc to set all zero in buffer
     printf("received from IP: %d.%d.%d.%d port: %d  length: %d\n",
		addr->addr&0xff,
		(addr->addr>>8)&0xff,
		(addr->addr>>16)&0xff,
		addr->addr>>24,
        port,
        p->len);
     strncpy(buffer, (char *)p->payload,p->len);
     buffer[5] = '\0';
     (strcmp(buffer, "LEDON") == 0) ? gpio_put(17,1) : gpio_put(17,0);
     //printf("message: %s\n", buffer);
     free(buffer);
     pbuf_free(p);
}



int main() {
    int loop=0;
    char buffer[BUF_SIZE];
    extern struct netif gnetif;
    gpio_init(17); gpio_set_dir(17,GPIO_OUT);
    stdio_init_all();
    init_i2c_lib();
    if (cyw43_arch_init()) {
        printf("Init failed!\n");
        return 1;
    }
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE,200,1,1,10);

    cyw43_arch_enable_sta_mode();

    printf("WiFi ... ");
    if (cyw43_arch_wifi_connect_timeout_ms("SSID", "PASSWORD", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed!\n");
        return 1;
    } else {
        printf("Connected.n");
        //printf("IP: %s\n",ipaddr_ntoa(((const ip_addr_t *)&cyw43_state.netif[0].ip_addr)));
        print_ip(ipaddr_ntoa(((const ip_addr_t *)&cyw43_state.netif[0].ip_addr)));
  }


   rcv_udp_pcb = udp_new();

   err_t err = udp_bind(rcv_udp_pcb,RCV_FROM_IP,RCV_FROM_PORT);
   udp_recv(rcv_udp_pcb, RcvFromUDP,NULL);

    while(1)
    {

     sleep_ms(10);
    cyw43_arch_poll();
    }


    udp_remove(rcv_udp_pcb);
    cyw43_arch_deinit();
    return 0;
}
