#pragma once

#include "config_enums.h"

#define ANY_MODE_ON_OFF (MODE == MODE_ON_OFF || MODE == MODE_ON_OFF_INVERTED)

////////////////////////////////////////////
// Debug Configuration
////////////////////////////////////////////

#define DEBUG 0

////////////////////////////////////////////
// WiFi configuration
////////////////////////////////////////////

#define WIFI_SSID "YOUR SSID"
#define WIFI_PSK "YOUR PSK"
#define TCP_PORT 8080
#define CLIENT_TIMEOUT 30 * 1000 // ms

#define USE_STATIC_IP // Comment out for DHCP

// Only needs to be set if USE_STATIC_IP defined
#ifdef USE_STATIC_IP
#define STATIC_IP "192.168.1.10"
#define GATEWAY "192.168.1.1"
#define SUBNET "255.255.255.0"
#endif

////////////////////////////////////////////
// Push Button/Switch Configuration
////////////////////////////////////////////

#define PHYSICAL_INPUT PHYS_SWITCH // PHYS_BUTTON, PHYS_SWITCH or comment out if not used
#define PHYSICAL_PIN 4

#if PHYSICAL_INPUT == PHYS_BUTTON
#define DEBOUNCE_TIME 20 // ms
#define SAMPLES 10
#endif

////////////////////////////////////////////
// Action Configuration
////////////////////////////////////////////

#define ACTION_PIN 5

/**
 * Possible values:
 *  - MODE_ON_OFF
 *  - MODE_ON_OFF_INVERTED
 *  - MODE_TOGGLE
 *  - MODE_PULSE_LOW
 *  - MODE_PULSE_HIGH
 */
#define MODE MODE_ON_OFF_INVERTED


////////////////////////////////////////////
// ON/OFF Mode Configuration
////////////////////////////////////////////

#if ANY_MODE_ON_OFF

// TCP Messages
#define TCP_OFF_DATA    0x00
#define TCP_ON_DATA     0x01
#define TCP_GET_STATE   0x03

// Boot state
#define BOOT_STATE      OFF

////////////////////////////////////////////
// Toggle Mode Configuration
////////////////////////////////////////////

#elif MODE == MODE_TOGGLE
#define TCP_TOGGLE_DATA 0x01
#define BOOT_STATE      1 // 0: OFF, 1: ON

////////////////////////////////////////////
// Pulse Mode Configuration
////////////////////////////////////////////

#elif MODE == MODE_PULSE_LOW || MODE == MODE_PULSE_HIGH
#define TCP_PULSE_DATA  0x01
#define PULSE_TIME      250 // ms

#else
#error "No mode selected"
#endif
