#ifndef PAIRLINK_H
#define PAIRLINK_H

#include <Arduino.h>
#include "PLConfig.h"
#include "PLState.h"
#include "PLWiFi.h"
#include "PLWebSocket.h"
#include "PLProtocol.h"
#include "PLPairing.h"
#include "PLLed.h"
#include "PLIdentity.h"

typedef void (*PLValueCallback)(const char* channel, float value);
typedef void (*PLChannelValueCallback)(float value);
typedef void (*PLStatusCallback)(PLConnectionState state);

/**
 * @class PairLink
 * @brief Main class for PairLink communication with ESP32 devices.
 * 
 * PairLink allows you to publish values on channels and subscribe to values from other devices.
 * It handles WiFi connection, WebSocket communication, and device pairing automatically.
 * 
 * Example:
 * @code
 * #include <PairLink.h>
 * 
 * PairLink pairLink;
 * 
 * void onLedBrightness(float value) {
 *   Serial.printf("LED brightness: %.1f%%\n", value);
 * }
 * 
 * void setup() {
 *   Serial.begin(115200);
 *   
 *   // Simple configuration for local-only use
 *   pairLink.beginSimple("YOUR_WIFI_SSID", "YOUR_PASSWORD", "ws://192.168.1.100:8080/ws");
 *   
 *   // Register channels
 *   pairLink.addPublishChannel("sensor.value");
 *   pairLink.addSubscribeChannel("led.brightness");
 *   pairLink.onChannel("led.brightness", onLedBrightness);
 * }
 * 
 * void loop() {
 *   pairLink.update();
 *   // Your code here
 * }
 * @endcode
 */
class PairLink {
public:
  /**
   * @brief Initialize PairLink with a full configuration.
   * @param config PLConfig struct with all settings.
   * 
   * Use this for advanced configuration. For simple local-only use, prefer beginSimple().
   * 
   * @code
   * PLConfig config;
   * plConfigSetLocalOnly(config, "YOUR_SSID", "YOUR_PASSWORD", "ws://server:8080/ws");
   * config.pairButtonPin = 12;  // Optional pairing button
   * config.statusLedPin = 2;    // Optional status LED
   * config.debugMode = true;    // Enable verbose logging
   * pairLink.begin(config);
   * @endcode
   */
  void begin(const PLConfig& config);
  
  /**
   * @brief Update PairLink - must be called regularly in loop().
   * 
   * This method handles:
   * - WiFi connection and reconnection
   * - WebSocket connection and reconnection
   * - Pairing button events
   * - Status LED updates
   * - Heartbeat messages to server
   * - Serial configuration commands
   */
  void update();

  /**
   * @brief Publish a value on a channel with optional throttling.
   * @param channel The channel name (e.g., "sensor.temperature")
   * @param value The value to send (float)
   * @param intervalMs Minimum time between sends in milliseconds (default: 50ms)
   * 
   * Values are sent at most every `intervalMs` milliseconds.
   * Use publishImmediate() for buttons or time-critical events.
   * 
   * @note If not connected, the value is silently dropped.
   * 
   * @code
   * // Publish sensor value every 50ms (default)
   * pairLink.publish("sensor.value", analogRead(34) / 40.95f);
   * 
   * // Publish only every 500ms
   * pairLink.publish("sensor.value", value, 500);
   * 
   * // Send immediately (no throttling)
   * pairLink.publishImmediate("button", 1.0f);
   * @endcode
   */
  void publish(const char* channel, float value, unsigned long intervalMs = 50);
  
  /**
   * @brief Publish a value immediately without throttling.
   * @param channel The channel name
   * @param value The value to send
   * 
   * Use this for buttons or time-critical events where you need immediate delivery.
   */
  void publishImmediate(const char* channel, float value);
  
  /**
   * @brief Register a channel for publishing.
   * @param channel The channel name to register
   * 
   * Call this for each channel you want to publish values on.
   * Maximum 16 channels can be registered.
   * 
   * @code
   * pairLink.addPublishChannel("sensor.temperature");
   * pairLink.addPublishChannel("sensor.humidity");
   * @endcode
   */
  void addPublishChannel(const char* channel);
  
  /**
   * @brief Register a channel for subscribing.
   * @param channel The channel name to subscribe to
   * 
   * Call this for each channel you want to receive values from.
   * Maximum 16 channels can be registered.
   * 
   * @code
   * pairLink.addSubscribeChannel("led.brightness");
   * pairLink.addSubscribeChannel("motor.speed");
   * @endcode
   */
  void addSubscribeChannel(const char* channel);

  /**
   * @brief Set a callback for all incoming values.
   * @param callback Function to call when a value arrives
   * 
   * This callback is called for values on channels that don't have a specific
   * channel callback set via onChannel().
   * 
   * @code
   * void onValueReceived(const char* channel, float value) {
   *   Serial.printf("Received %s = %.2f\n", channel, value);
   * }
   * 
   * pairLink.onValue(onValueReceived);
   * @endcode
   */
  void onValue(PLValueCallback callback);
  
  /**
   * @brief Set a callback for a specific channel.
   * @param channel The channel name
   * @param callback Function to call when a value arrives on this channel
   * 
   * This is the easiest way to handle incoming values.
   * The callback is only called for the specified channel.
   * 
   * @note Call addSubscribeChannel() for this channel BEFORE calling onChannel().
   * 
   * @code
   * void onLedBrightness(float value) {
   *   // value is between 0.0 and 100.0
   *   bool on = value > 0.5f;
   * }
   * 
   * pairLink.addSubscribeChannel("led.brightness");
   * pairLink.onChannel("led.brightness", onLedBrightness);
   * @endcode
   */
  void onChannel(const char* channel, PLChannelValueCallback callback);
  
  /**
   * @brief Set a callback for connection status changes.
   * @param callback Function to call when connection state changes
   * 
   * Use this to track connection progress or trigger actions based on state.
   * 
   * @code
   * void onStatusChanged(PLConnectionState state) {
   *   Serial.println(plStateToString(state));
   *   // States: BOOTING, WIFI_CONNECTING, LOCAL_CONNECTED, etc.
   * }
   * 
   * pairLink.onStatus(onStatusChanged);
   * @endcode
   */
  void onStatus(PLStatusCallback callback);

  /**
   * @brief Get the current connection state.
   * @return Current PLConnectionState enum value
   */
  PLConnectionState getState() const;
  
  /**
   * @brief Get the session ID assigned by the server.
   * @return Session ID string, or empty string if not registered
   */
  const char* getSessionId() const;
  
  /**
   * @brief Get the device ID.
   * @return Device ID string (ephemeral or persistent)
   */
  const char* getDeviceId() const;
  
  /**
   * @brief Check if paired with another device.
   * @return true if currently paired, false otherwise
   */
  bool isPaired() const;

  /**
   * @brief Get current WiFi SSID (for runtime config).
   * @return Current SSID string
   */
  const char* getDefaultSsid() const;
  
  /**
   * @brief Get current WiFi password (for runtime config).
   * @return Current password string
   */
  const char* getDefaultPassword() const;
  
  /**
   * @brief Get current local server URL.
   * @return Local server WebSocket URL
   */
  const char* getLocalServerUrl() const;
  
  /**
   * @brief Get current cloud/relay server URL.
   * @return Cloud server URL, or empty string if not configured
   */
  const char* getCloudServerUrl() const;

  /**
   * @brief Change network and server configuration at runtime.
   * @param ssid WiFi SSID (or nullptr to keep current)
   * @param password WiFi password (or nullptr to keep current)
   * @param localUrl Local server WebSocket URL
   * @param cloudUrl Optional cloud/relay server URL (or nullptr for local only)
   * 
   * This method reconnects with the new configuration.
   * Use serial commands ("wifi,SSID,Pass" and "server,ws://...") instead
   * for simple runtime changes without reflashing.
   * 
   * @note Buffer limits: SSID 32 chars, password 64 chars, URLs 128 chars
   */
  void setNetworkAndServer(const char* ssid, const char* password, const char* localUrl, const char* cloudUrl = nullptr);

  /**
   * @brief Simple initialization for local-only use.
   * @param ssid WiFi SSID
   * @param password WiFi password
   * @param serverUrl Local server WebSocket URL (e.g., "ws://192.168.1.100:8080/ws")
   * This is the easiest way to get started with PairLink.
   * It automatically configures:
   * - No identity (ephemeral device)
   * - Auto pairing mode
   *
   * @code
   * // Basic usage (button/LED disabled)
   * pairLink.beginSimple("MyWiFi", "secret123", "ws://192.168.1.100:8080/ws");
   *
   * // Enable BOOT button for pairing (and built-in status LED)
   * pairLink.beginSimple("MyWiFi", "secret123", "ws://192.168.1.100:8080/ws",
   *                       0, 2);
   * @endcode
   *
   * @note For debug mode or additional hardware features, use begin() with PLConfig.
   */
  void beginSimple(const char* ssid, const char* password, const char* serverUrl);

  /**
   * @brief Simple initialization for local-only use with optional button/LED.
   * @param ssid WiFi SSID
   * @param password WiFi password
   * @param serverUrl Local server WebSocket URL (e.g., "ws://192.168.1.100:8080/ws")
   * @param pairButtonPin Optional GPIO for pairing button (LOW = pressed). Pass -1 to disable.
   * @param statusLedPin Optional GPIO for connection status LED. Pass -1 to disable.
   */
  void beginSimple(const char* ssid, const char* password, const char* serverUrl,
                    int pairButtonPin, int statusLedPin);

private:
  PLConfig _config;
  PLWiFiManager _wifi;
  PLWebSocketClient _ws;
  PLProtocol _protocol;
  PLPairing _pairing;
  PLLed _led;
  PLIdentity _identity;

  PLValueCallback _valueCallback = nullptr;
  PLStatusCallback _statusCallback = nullptr;

  static constexpr int MAX_CHANNELS = 16;
  PLChannelValueCallback _channelCallbacks[MAX_CHANNELS] = {nullptr};
  const char* _publishChannels[MAX_CHANNELS];
  int _publishChannelCount = 0;
  const char* _subscribeChannels[MAX_CHANNELS];
  int _subscribeChannelCount = 0;

  char _sessionId[32] = {0};
  bool _paired = false;

  unsigned long _lastPublish[MAX_CHANNELS] = {0};
  unsigned long _lastHeartbeat = 0;
  unsigned long _publishSeq = 0;

  PLConnectionState _connectionState = PL_STATE_BOOTING;

  static constexpr int MAX_LOCAL_RETRIES = 3;
  int _localRetryCount = 0;
  unsigned long _lastLocalProbeMs = 0;
  static constexpr unsigned long LOCAL_PROBE_INTERVAL_MS = 30000;

  char _configSsid[PL_CONFIG_SSID_MAX] = {0};
  char _configPassword[PL_CONFIG_PASSWORD_MAX] = {0};
  char _configLocalUrl[PL_CONFIG_URL_MAX] = {0};
  char _configCloudUrl[PL_CONFIG_URL_MAX] = {0};

  // Built-in serial configuration (wifi/server commands + NVS) state
  bool _serialConfigEnabled = true;
  Stream* _serial = &Serial;
  static constexpr size_t _serialLineMax = 256;
  char _serialLineBuf[_serialLineMax] = {0};
  size_t _serialLineLen = 0;
  bool _serialBootPrinted = false;

  void handleSerialInput();
  void processSerialLine();

  void handleIncomingMessage(const char* payload);
  void sendRegister();
  void sendHeartbeat();
  void setState(PLConnectionState newState);
  int findPublishChannelIndex(const char* channel) const;
  void initiateConnection();
};

#endif