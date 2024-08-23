/**
 * Generic firmware control a pin via TCP on a Pi Pico W
 *
 * The firmware offers four possible "actions":
 * - ON/OFF: The pin is set to 0 or 1 based on two unique TCP messages
 * - TOGGLE: The pin is toggled based on one unique TCP message
 * - PULSE_LOW: The pin is lowered to 0 for a short period of time upon receiving a unique TCP message
 * - PULSE_HIGH: The pin is raised to 1 for a short period of time upon receiving a unique TCP message
 * Additionally, the ON/OFF mode can also return the current state of the pin upon receiving a unique TCP message.
 *
 * The action as well as other parameters can be configured in the config.h file.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "stdio.h"

#include "config.h"

#define NO_ACTION -1

#define PULSE_LOW() gpio_put(ACTION_PIN, 0); sleep_ms(PULSE_TIME); gpio_put(ACTION_PIN, 1)
#define PULSE_HIGH() gpio_put(ACTION_PIN, 1); sleep_ms(PULSE_TIME); gpio_put(ACTION_PIN, 0)

#if MODE == MODE_ON_OFF
static uint8_t state = BOOT_STATE ? TCP_ON_DATA : TCP_OFF_DATA;
#endif

void init_action_pin() {
    gpio_init(ACTION_PIN);
    gpio_set_dir(ACTION_PIN, GPIO_OUT);
#if MODE == ON_OFF || MODE == TOGGLE
    gpio_put(ACTION_PIN, BOOT_STATE);
#elif MODE == PULSE_LOW
    gpio_put(ACTION_PIN, 1);
#elif MODE == PULSE_HIGH
    gpio_put(ACTION_PIN, 0);
#endif
}

void action(uint8_t data) {
#if MODE == MODE_ON_OFF
    switch(data) {
    case TCP_OFF_DATA:
        gpio_put(ACTION_PIN, 0);
        state = TCP_OFF_DATA;
        break;
    case TCP_ON_DATA:
        gpio_put(ACTION_PIN, 1);
        state = TCP_ON_DATA;
        break;
    default:
        break;
    }
#elif MODE == MODE_TOGGLE
    if(data == TCP_TOGGLE_DATA) {
        gpio_put(ACTION_PIN, !gpio_get(ACTION_PIN));
    }
#elif MODE == MODE_PULSE_LOW
    if(data == TCP_PULSE_DATA) {
        PULSE_LOW();
    }
#elif MODE == MODE_PULSE_HIGH
    if(data == TCP_PULSE_DATA) {
        PULSE_HIGH();
    }
#endif
}

// Callback function for when data is received
static err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    int16_t received_data = NO_ACTION;

    if (p != NULL) {
        received_data = *((uint8_t *)p->payload);
        pbuf_free(p);

#if MODE == MODE_ON_OFF
        if (received_data == TCP_GET_STATE)
            tcp_write(tpcb, &state, 1, 0);

        received_data = NO_ACTION;
#endif

    }

    // Close TCP connection
    tcp_close(tpcb);

    if (received_data != NO_ACTION) {
        action(received_data);
    }

    return ERR_OK;
}

// Callback function for when a connection is accepted
static err_t tcp_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_recv_callback);
    return ERR_OK;
}

void print_ip_address(struct netif *netif) {
    if (!netif_is_up(netif)) {
        printf("Network interface is down.\n");
        return;
    }

    const ip4_addr_t *ip = netif_ip4_addr(netif);
    if (ip == NULL) {
        printf("No IP address assigned.\n");
    } else {
        printf("IP Address: %s\n", ip4addr_ntoa(ip));
    }
}

int main() {
    stdio_init_all();
    init_action_pin();

    // Initialize the Wi-Fi and lwIP stack
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Connect to Wi-Fi
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PSK, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Wi-Fi connection failed\n");
        return -1;
    }
    printf("Connected to Wi-Fi\n");

    // Print the IP address
    struct netif *netif = netif_list;
    print_ip_address(netif);

    // Create a new TCP control block
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Error creating PCB\n");
        return -1;
    }

    // Bind the PCB to port
    if (tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT) != ERR_OK) {
        printf("TCP bind failed\n");
        return -1;
    }

    // Start listening for incoming connections
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, tcp_accept_callback);

    // Main loop to handle lwIP tasks
    while (true) {
        cyw43_arch_poll();
        sleep_ms(100); // Adjust based on your application's requirements
    }

    // Cleanup (if ever reached)
    cyw43_arch_deinit();
    return 0;
}
