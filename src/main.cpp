#include <Arduino.h>

#define SimpleFIFO_NONVOLATILE

#include <SoftwareSerial.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <TimeLib.h>

#include <Ticker.h>
#include <ESP8266HTTPUpdate.h>

#include "LEDBlinker.h"

#include "SimpleFIFO.h"
#include "Message.h"
#include "ntp.h"
#include "CloudClient.h"
#include "GatewayConfiguration.h"

///////////
#define PIN_CARD_RX 14
#define PIN_CARD_TX 12
#define PIN_PORTAL_MODE 16
#define PIN_GSM_LED 4
#define READ_TIMEOUT 2000
#define MESSAGE_AREA_COUNT 4
#define WAIT_BEFORE_RETRY_SEC 60

#define GATEWAY_KEEP_ALIVE_INTERVAL 60*60*24 - 60
#define GATEWAY_CHECK_VERSION_INTERVAL 60*60*4

#ifndef NTP_SERVER
  #define NTP_SERVER "pool.ntp.org"
#endif
///////////

bool firstNtpSync = true;
bool shouldSendKeepAlive = false;
bool shouldCheckVersion = false;
bool shouldUpdateNtp = false;

Ticker keepAliveTicker;
Ticker checkVersionTicker;

LEDBlinker ledGSM(PIN_GSM_LED);

GatewayConfiguration gatewayConfiguration;
CloudClient cloudClient;
SoftwareSerial swSerial(PIN_CARD_RX, PIN_CARD_TX, false, 512);

SimpleFIFO<Message, 20> msgFIFO;
unsigned long lastErrorTimestamp = 0;

///////////
bool sendMessage(Message msg);
void ledBlinkCallback();
void keepAliveTickerCallback();
void checkVersion();
void checkVersionTickerCallback();
void sendAck();
String getFirmwareVersion();
///////////

WiFiEventHandler disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
  Serial.println("Station disconnected");
  ledGSM.off();
  firstNtpSync = false;
  shouldUpdateNtp = false;
  keepAliveTicker.detach();
  checkVersionTicker.detach();
});

WiFiEventHandler gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
  Serial.println("Station got IP");
  ledGSM.on();
  if(WiFi.getMode() == WIFI_STA) {
    firstNtpSync = true;
    shouldUpdateNtp = true;
    keepAliveTicker.attach(GATEWAY_KEEP_ALIVE_INTERVAL, keepAliveTickerCallback);
    checkVersionTicker.attach(GATEWAY_CHECK_VERSION_INTERVAL, checkVersionTickerCallback);
  }
});

//////////

void setup() {
  pinMode(PIN_PORTAL_MODE, INPUT);

  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println();
  Serial.println(String("Wifi Gateway V") + VERSION_CODE);

  swSerial.begin(115200);

  gatewayConfiguration.startWifiConfiguration(true);
  cloudClient.updateEndpoint(gatewayConfiguration.getHttpHost(), gatewayConfiguration.getHttpPort(), gatewayConfiguration.getHttpUrl(), gatewayConfiguration.getProxyAuthorizationHeader());
}

void loop() {
  if (digitalRead(PIN_PORTAL_MODE) == HIGH) {
    Serial.println("Config button pressed");
    ledGSM.off();
    gatewayConfiguration.startWifiConfiguration(false);
    cloudClient.updateEndpoint(gatewayConfiguration.getHttpHost(), gatewayConfiguration.getHttpPort(), gatewayConfiguration.getHttpUrl(), gatewayConfiguration.getProxyAuthorizationHeader());
  }

  if(shouldUpdateNtp) {
    shouldUpdateNtp = false;
    updateNtp(String(NTP_SERVER));
  }

  if(firstNtpSync && getCurrentTimestamp() > 0) {
    firstNtpSync = false;
    shouldSendKeepAlive = true;
    shouldCheckVersion = true;
  }

  if(shouldSendKeepAlive) {
    shouldSendKeepAlive = false;
    Message msg(getFirmwareVersion(),
      gatewayConfiguration.getGatewayId(),
      String("keepalive"),
      String(getCurrentTimestamp()));
    msgFIFO.enqueue(msg);
  }

  if(shouldCheckVersion) {
    shouldCheckVersion = false;
    checkVersion();
  }

  while(swSerial.available() > 0) {
    String messageReceived = swSerial.readStringUntil('\n');
    if(messageReceived.length() > 0 && messageReceived.indexOf('\r') != -1) {
      Serial.print("Message received: ");
      Serial.println(messageReceived);

      sendAck();

      Message msg(getFirmwareVersion(),
        gatewayConfiguration.getGatewayId(),
        messageReceived,
        String(getCurrentTimestamp()));
        msgFIFO.enqueue(msg);
    } else {
      Serial.println("-> Message reception timeout");
    }
  }

  while(msgFIFO.count() > 0 && getCurrentTimestamp() - lastErrorTimestamp > WAIT_BEFORE_RETRY_SEC) {
    Message msg = msgFIFO.peek();
    bool result = sendMessage(msg);
    if(result) {
      msgFIFO.dequeue();
    } else {
      lastErrorTimestamp = getCurrentTimestamp();
    }
  }

  ledGSM.update();

  delay(0);
}

void checkVersion() {
  t_httpUpdate_return ret = ESPhttpUpdate.update(HTTP_UPDATE_HOST, HTTP_UPDATE_PORT, HTTP_UPDATE_URL, String(VERSION_CODE));
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("[update] Update failed.");
      Serial.println(ESPhttpUpdate.getLastErrorString());
    break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[update] No update.");
    break;
    case HTTP_UPDATE_OK:
      Serial.println("[update] Update ok."); // may not be called; reboot the ESP
    break;
  }
}

void checkVersionTickerCallback() {
  Serial.println("Should check version");
  shouldCheckVersion = true;
}

void keepAliveTickerCallback() {
  Serial.println("Should send keep-alive");
  shouldSendKeepAlive = true;
}

void ledBlinkCallback() {
  if(WiFi.status() == WL_IDLE_STATUS) {
    ledGSM.blink(500, 500, 3, 0, 0, NULL);
  } else if(WiFi.status() == WL_CONNECTED) {
    ledGSM.on();
  } else {
    ledGSM.off();
  }
}

void sendAck() {
  swSerial.println(0xAA);
}

bool sendMessage(Message msg) {
  ledGSM.blink(100, 100, 3, 0, 1, ledBlinkCallback);

  return cloudClient.sendWifiMessage(msg.getFirmwareVersion(),
    msg.getGatewayId(),
    msg.getPayload(),
    msg.getTimestamp());
}

String getFirmwareVersion() {
  String firmwareVersion = gatewayConfiguration.getFirmware();
  return firmwareVersion[0] == '\0' ? String(VERSION_CODE) : firmwareVersion;
}
