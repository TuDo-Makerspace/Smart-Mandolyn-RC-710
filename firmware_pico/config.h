#pragma once

#include "modes_t.h"

// -- Global Parameters --
#define ACTION_PIN 0
#define MODE MODE_ON_OFF // ON_OFF, TOGGLE, PULSE_LOW, PULSE_HIGH
#define WIFI_SSID "YOUR SSID"
#define WIFI_PSK "YOUR PASSWORD"
#define TCP_PORT 8080

// -- Properties for ON/OFF mode --
#if MODE == MODE_ON_OFF
#define TCP_OFF_DATA    0x00
#define TCP_ON_DATA     0x01
#define TCP_GET_STATE   0x03
#define BOOT_STATE      1 // 0: OFF, 1: ON

// -- Properties for TOGGLE mode --
#elif MODE == MODE_TOGGLE
#define TCP_TOGGLE_DATA 0x01
#define BOOT_STATE      1 // 0: OFF, 1: ON

// -- Properties for PULSE_LOW mode --
#elif MODE == MODE_PULSE_LOW
#define TCP_PULSE_DATA  0x01
#define PULSE_TIME      250 // ms

// -- Properties for PULSE_HIGH mode --
#elif MODE == MODE_PULSE_HIGH
#define TCP_PULSE_DATA  0x01
#define PULSE_TIME      250 // ms

#else
#error "No mode selected"
#endif