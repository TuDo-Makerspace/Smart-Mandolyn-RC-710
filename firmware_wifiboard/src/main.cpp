#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"

////////////////////////////////////////////
// Meta
////////////////////////////////////////////

#define VERSION "1.0.1_ESP8266"

////////////////////////////////////////////
// Debugging
////////////////////////////////////////////

#if DEBUG > 0
#define DEBUG_PRINTLN(level, x) if (level <= DEBUG) Serial.println(x)
#else
#define DEBUG_PRINTLN(level, x)
#endif

////////////////////////////////////////////
// Actions and States
////////////////////////////////////////////

/**
 * enum device_state
 * Maps device states to their corresponding electrical states
 */

#if MODE == MODE_ON_OFF_INVERTED

enum device_state {
    OFF = HIGH,
    ON = LOW,
};

#else

enum device_state {
    OFF = LOW,
    ON = HIGH,
};

#endif

/**
 * enum action_type
 * Contains the possible actions that can be performed
 * for the selected mode.
 */

#if ANY_MODE_ON_OFF

enum action_type {
    DISABLE,
    ENABLE,
};

#elif MODE == MODE_TOGGLE

enum action_type {
    TOGGLE,
};

#elif MODE == MODE_PULSE_LOW || MODE == MODE_PULSE_HIGH

enum action_type {
    PULSE,
};

#endif

/**
 * Globals
 */

static uint8_t g_state; // Current gpio state of the action pin

/**
 * void init_action_pin()
 * Initializes the action pin as an output and sets it to the boot state.
 * The boot state is defined in config.h.
 */

#if ANY_MODE_ON_OFF || MODE == MODE_TOGGLE

void init_action_pin() {
    pinMode(ACTION_PIN, OUTPUT);
    digitalWrite(ACTION_PIN, BOOT_STATE);
    g_state = BOOT_STATE;
}

#elif MODE == MODE_PULSE_LOW

void init_action_pin() {
    pinMode(ACTION_PIN, OUTPUT);
    digitalWrite(ACTION_PIN, HIGH);
    g_state = HIGH;
}

#elif MODE == MODE_PULSE_HIGH

void init_action_pin() {
    pinMode(ACTION_PIN, OUTPUT);
    digitalWrite(ACTION_PIN, LOW);
    g_state = LOW;
}

#endif

/**
 * void action(action_type action)
 * Performs the action corresponding to the given action type.
 * The action type is determined by the mode defined in config.h.
 */

#if ANY_MODE_ON_OFF

void action(action_type action) {
    switch (action) {
    case DISABLE:
        DEBUG_PRINTLN(1, "Action: Disabling");
        digitalWrite(ACTION_PIN, OFF);
        g_state = OFF;
        break;
    case ENABLE:
        DEBUG_PRINTLN(1, "Action: Enabling");
        digitalWrite(ACTION_PIN, ON);
        g_state = ON;
        break;
    }
}

#elif MODE == MODE_TOGGLE

void action(action_type action) {
    if (action == TOGGLE) {
        DEBUG_PRINTLN(1, "Action: Toggling");
        digitalWrite(ACTION_PIN, !digitalRead(ACTION_PIN));
        g_state = digitalRead(ACTION_PIN);
    }
}

#elif MODE == MODE_PULSE_LOW

void action(action_type action) {
    if (action == PULSE) {
        DEBUG_PRINTLN(1, "Action: Pulsing low");
        digitalWrite(ACTION_PIN, LOW);
        delay(PULSE_TIME);
        digitalWrite(ACTION_PIN, HIGH);
        g_state = HIGH;
    }
}

#elif MODE == MODE_PULSE_HIGH

void action(action_type action) {
    if (action == PULSE) {
        DEBUG_PRINTLN(1, "Action: Pulsing high");
        digitalWrite(ACTION_PIN, HIGH);
        delay(PULSE_TIME);
        digitalWrite(ACTION_PIN, LOW);
        g_state = LOW;
    }
}

#endif

////////////////////////////////////////////
// TCP Requests and Responses
////////////////////////////////////////////

/**
 * uint8_t device_state2tcp(device_state state)
 * Maps a device state to its corresponding tcp data.
 */
#if ANY_MODE_ON_OFF

uint8_t device_state2tcp(device_state state) {
    return state == ON ? TCP_ON_DATA : TCP_OFF_DATA;
}

#elif MODE == MODE_TOGGLE

uint8_t device_state2tcp(device_state state) {
    return TCP_TOGGLE_DATA;
}

#elif MODE == MODE_PULSE_LOW || MODE == MODE_PULSE_HIGH

uint8_t device_state2tcp(device_state state) {
    return TCP_PULSE_DATA;
}

#endif

/**
 * int tcp2action_type(uint8_t data, action_type *type)
 * Maps a tcp data to its corresponding action type.
 * If data can't be mapped to an action type, -1 is returned.
 */

#if ANY_MODE_ON_OFF

int tcp2action_type(uint8_t data, action_type *type) {
    switch (data) {
    case TCP_OFF_DATA:
        *type = DISABLE;
        return 0;
    case TCP_ON_DATA:
        *type = ENABLE;
        return 0;
    default:
        return -1;
    }
}

#elif MODE == MODE_TOGGLE

int tcp2action_type(uint8_t data, action_type *type) {
    if (data == TCP_TOGGLE_DATA) {
        *type = TOGGLE;
        return 0;
    }

    return -1;
}

#elif MODE == MODE_PULSE_LOW || MODE == MODE_PULSE_HIGH

int tcp2action_type(uint8_t data, action_type *type) {
    if (data == TCP_PULSE_DATA) {
        *type = PULSE;
        return 0;
    }

    return -1;
}

#endif

/**
 * bool answer(WiFiClient *c, uint8_t get_request)
 * Answers the client based on the get_request.
 * If get_request is not recognized (i.e. its a set_request), false is returned.
 */
#if ANY_MODE_ON_OFF

bool answer(WiFiClient *c, uint8_t get_request) {
    if (get_request == TCP_GET_STATE) {
        DEBUG_PRINTLN(2, "get_request: TCP_GET_STATE");
        c->write(device_state2tcp(device_state(g_state)));
        return true;
    }
    return false;
}


#else

bool answer(WiFiClient *c, uint8_t get_request) {
    return false;
}

#endif

////////////////////////////////////////////
// Physical Inputs
////////////////////////////////////////////

/**
 * void init_physical_input()
 * Initializes the physical input pin as an input with pullup.
 * If no physical input is defined, this function does nothing.
 */

#if PHYSICAL_INPUT == PHYS_BUTTON || PHYSICAL_INPUT == PHYS_SWITCH

void init_physical_input() {
    pinMode(PHYSICAL_PIN, INPUT_PULLUP);
}

#else

void init_physical_input() {}

#endif

/**
 * bool eval_physical_input()
 * Returns true if the physical input has changed state.
 * - In case of a button this means the button has been pressed.
 * - In case of a switch this means the switch has been toggled.
 * If no physical input is defined, always returns false.
 */
#if PHYSICAL_INPUT == PHYS_BUTTON

bool eval_physical_input() {
    static bool last_state = digitalRead(PHYSICAL_PIN);
    bool current_state = digitalRead(PHYSICAL_PIN);

    bool pressed = last_state && !current_state;
    last_state = current_state;

    if (pressed) {
        delay(DEBOUNCE_TIME);

        uint32_t wrong = 0;
        for (int i = 0; i < SAMPLES; i++) {
            if (digitalRead(PHYSICAL_PIN) != current_state)
                wrong++;
        }

        return wrong < (SAMPLES / 2);
    }

    return false;
}

#elif PHYSICAL_INPUT == PHYS_SWITCH

bool eval_physical_input() {
    static bool last_state = digitalRead(PHYSICAL_PIN);
    bool current_state = digitalRead(PHYSICAL_PIN);

    bool toggled = last_state != current_state;
    last_state = current_state;

    return toggled;
}
#else
bool eval_physical_input() {
    return false;
}

#endif

/**
 * void action_on_phys()
 * Executes the appropriate action on physical input.
 */

#ifdef ANY_MODE_ON_OFF

void action_on_phys() {
    action(g_state == ON ? DISABLE : ENABLE);
}

#elif MODE == MODE_TOGGLE

void action_on_phys() {
    action(TOGGLE);
}

#elif MODE == MODE_PULSE_LOW || MODE == MODE_PULSE_HIGH

void action_on_phys() {
    action(PULSE);
}

#endif


////////////////////////////////////////////
// WiFi
////////////////////////////////////////////

/**
 * void config_wifi()
 * Configures static IP, gateway and subnet if USE_STATIC_IP is defined.
 * Else, this function does nothing.
 */
#ifdef USE_STATIC_IP

void config_wifi() {
    IPAddress ip, gateway, subnet;
    ip.fromString(STATIC_IP);
    gateway.fromString(GATEWAY);
    subnet.fromString(SUBNET);

    WiFi.config(ip, gateway, subnet);

    DEBUG_PRINTLN(2, "\nConfigured WiFi with: ");
    DEBUG_PRINTLN(2, "IP: " STATIC_IP);
    DEBUG_PRINTLN(2, "Gateway: " GATEWAY);
    DEBUG_PRINTLN(2, "Subnet: " SUBNET "\n");
}

#else

void config_wifi() {}

#endif

////////////////////////////////////////////
// Timeouts
////////////////////////////////////////////

static unsigned long tstamp;

void set_timeout(unsigned long timeout) {
    tstamp = millis() + timeout;
}

bool timeout() {
    return millis() > tstamp;
}

////////////////////////////////////////////
// Misc
////////////////////////////////////////////

#define BUILTIN_LED_OFF HIGH
#define BUILTIN_LED_ON LOW

////////////////////////////////////////////
// Main
////////////////////////////////////////////

WiFiServer server(TCP_PORT);

void setup() {
    init_physical_input();
    init_action_pin();

#ifdef DEBUG
    Serial.begin(115200);
    delay(1000);
#endif

    DEBUG_PRINTLN(1, "====== TCP Action Server " VERSION "======");

    config_wifi();

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    pinMode(LED_BUILTIN, OUTPUT);
    bool led_state = BUILTIN_LED_OFF;

    // Connect to Wi-Fi
    DEBUG_PRINTLN(1, "Connecting to " WIFI_SSID "...");
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    while (WiFi.status() != WL_CONNECTED) {
        if (eval_physical_input()) {
            DEBUG_PRINTLN(1, "Physical input detected");
            action_on_phys();
        }

        digitalWrite(LED_BUILTIN, led_state = !led_state);
        delay(250);
    }

    digitalWrite(LED_BUILTIN, led_state = BUILTIN_LED_OFF);
    pinMode(LED_BUILTIN, INPUT);

    DEBUG_PRINTLN(1, "Connected to " WIFI_SSID);

    server.begin();

    DEBUG_PRINTLN(1, "Started TCP server");
}

void loop() {
    // Handle physical input
    if (eval_physical_input()) {
        DEBUG_PRINTLN(1, "Physical input detected");
        action_on_phys();
    }

    // Handle TCP requests
    WiFiClient client = server.accept();
    if (client) {
        DEBUG_PRINTLN(2, "Client connected");
        set_timeout(CLIENT_TIMEOUT);

        while (client.connected() && !timeout()) {
            if (!client.available()) {
                DEBUG_PRINTLN(3, "Awaiting data...");
                continue;
            }

            uint8_t data = client.read();

            if (answer(&client, data)) {
                set_timeout(CLIENT_TIMEOUT);
                continue;
            }

            action_type a;

            if (tcp2action_type(data, &a) == 0)
                action(a);

            set_timeout(CLIENT_TIMEOUT);
        }

        if (timeout()) {
            DEBUG_PRINTLN(1, "Client timed out");
        } else {
            DEBUG_PRINTLN(2, "Client disconnected");
        }

        client.stop();
    }
}
