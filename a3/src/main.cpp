/* ASSIGNMENT 3 - Advanced Networking
(Modified from Assignment 2)

Group 32:
Fabiano de Sá Filho
Felipe Noleto
João Antônio Astolfi
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>           // NEW: Required for AsyncMqttClient
#include <AsyncMqttClient.h>    // NEW: MQTT library from ex03.pdf
#include <avdweb_Switch.h>      // NEW: Button library from ex03.pdf

// WIFI (Kept from original)
#define WIFI_SSID "ADN-IOT"
#define WIFI_PASSWORD "WBNuyawB2a"
#define ADNGROUP "adn-group32" // Updated from "adn-groupXY" based on your comment

// MQTT (NEW: Based on ass03.pdf and ex03.pdf)
#define MQTT_HOST IPAddress(192, 168, 0, 1) // Broker IP from ass03.pdf Task 1.2
#define MQTT_PORT 1883                      // From ex03.pdf
#define LIGHT_SWITCH_ID "sonoff_03" // A unique ID for your light switch
#define MQTT_TOPIC_COMMAND "cmnd/" LIGHT_SWITCH_ID "/POWER"
#define MQTT_PAYLOAD_TOGGLE "TOGGLE" // From ass03.pdf Task 3

// BUTTON (NEW: From ex03.pdf)
#define PUSHBUTTON 17 // From ex03.pdf

// GLOBAL VARS
AsyncMqttClient mqttClient; // From ex03.pdf
Switch button = Switch(PUSHBUTTON); // From ex03.pdf

// FUNCTION PROTOTYPES (Updated)
void connect_to_wifi();
void slow_blink();
void fast_blink();
void publish_mqtt_toggle();
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char *topic, char *payload,
                   AsyncMqttClientMessageProperties properties, size_t len,
                   size_t index, size_t total);

void setup() {
  Serial.begin(115200);

  // LED setup
  pinMode(LED_BUILTIN, OUTPUT);

  // WiFi setup (Kept from original)
  connect_to_wifi();

  // MQTT setup (NEW: Based on ex03.pdf)
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setClientId(ADNGROUP);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  Serial.println("Connecting to MQTT broker...");
  mqttClient.connect();

  // Wait for MQTT connection (similar to your WiFi wait)
  unsigned long start_time = millis();
  const unsigned long timeout = 10000; // 10 seconds
  while (!mqttClient.connected() && millis() - start_time < timeout) {
    Serial.print(".");
    delay(500);
  }

  if (!mqttClient.connected()) {
    Serial.println("\nCould not connect to MQTT broker!");
    while (1) {
      fast_blink(); // Halt with fast blink on failure
    }
  }

  Serial.println("\nConnected to MQTT broker.");
  digitalWrite(LED_BUILTIN, HIGH); // Turn on LED when WiFi and MQTT are ready
}

void loop() {
  // Button logic (NEW: From ex03.pdf)
  button.poll();

  if (button.pushed()) {
    publish_mqtt_toggle();
  }
}

/**
 * @brief Publishes the MQTT TOGGLE command.
 */
void publish_mqtt_toggle() {
  Serial.printf("Button pushed! Publishing '%s' to topic '%s'\n",
                MQTT_PAYLOAD_TOGGLE, MQTT_TOPIC_COMMAND);
  
  // Publish the message (QoS 0, not retained)
  mqttClient.publish(MQTT_TOPIC_COMMAND, 0, false, MQTT_PAYLOAD_TOGGLE);
}

/**
 * @brief MQTT Connect Callback
 */
void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT Connected.");
  digitalWrite(LED_BUILTIN, HIGH); // Ensure LED is on if we reconnect
}

/**
 * @brief MQTT Disconnect Callback
 */
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.printf("MQTT Disconnected. Reason: %d\n", (int)reason);
  digitalWrite(LED_BUILTIN, LOW); // Turn off LED to show disconnected state
  // Try to reconnect
  mqttClient.connect();
}

/**
 * @brief MQTT Message Received Callback (from ex03.pdf)
 */
void onMqttMessage(char *topic, char *payload,
                   AsyncMqttClientMessageProperties properties, size_t len,
                   size_t index, size_t total) {
  // This function is required by the setup
  // We can log incoming messages for debugging
  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  
  // Create a null-terminated string from the payload
  char msg[len + 1];
  strncpy(msg, payload, len);
  msg[len] = '\0';
  Serial.println(msg);
}

/**
 * @brief Connects to WiFi (Kept from original, minor fixes)
 */
void connect_to_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(ADNGROUP);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Timeout vars
  unsigned long start_time = millis();
  const unsigned long timeout = 10000; // 10 seconds

  while (WiFi.status() != WL_CONNECTED && millis() - start_time < timeout) {
    delay(1000); // This delay is fine in setup
    slow_blink();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n Connected to WIFI \n");
    // Removed LED HIGH here, will set it after MQTT connects
  } else {
    Serial.printf("Could not connect");
    while (1) {
      fast_blink();
    }
  }

  // If connection successful, prints IP:
  // Fixed IP address printing
  Serial.printf("\n IP Address: %s\n", WiFi.localIP().toString().c_str());
}

/**
 * @brief Blinks LED slowly (Kept from original)
 */
void slow_blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

/**
 * @brief Blinks LED fast (Kept from original)
 */
void fast_blink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
}
