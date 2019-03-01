#include "GatewayConfiguration.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

GatewayConfiguration::GatewayConfiguration() {
  sprintf(httpHost, "%s", DEFAULT_SERVER_HOST);
  sprintf(httpPort, "%d", DEFAULT_SERVER_PORT);
  sprintf(httpUrl, "%s", DEFAULT_SERVER_URL);
  sprintf(gatewayId, "%s", DEFAULT_GATEWAY_ID);
  sprintf(firmware, "%d", VERSION_CODE);
  sprintf(proxyAuthorizationHeader, "%s", "");
}

GatewayConfiguration::~GatewayConfiguration() {
}

void GatewayConfiguration::startWifiConfiguration(bool autoConnect) {
  this->readConfigFile();

  WiFiManagerParameter custom_http_host("host", "HTTP Host", this->httpHost, FIELD_HOST_LENGTH);
  WiFiManagerParameter custom_http_port("port", "HTTP Port", this->httpPort, FIELD_PORT_LENGTH);
  WiFiManagerParameter custom_http_url("url", "HTTP URL", this->httpUrl, FIELD_URL_LENGTH);
  WiFiManagerParameter custom_gw_id("id", "GW ID", this->gatewayId, FIELD_ID_LENGTH);
  WiFiManagerParameter custom_firmware("firmware", "Firmware version", firmware, FIELD_FIRMWARE_LENGTH);
  WiFiManagerParameter custom_proxy_authorization_header("proxyAuthorizationHeader", "Proxy Authorization Header", proxyAuthorizationHeader, FIELD_PROXY_AUTHORIZATION_HEADER_LENGTH);


  WiFiManager wifiManager;
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.addParameter(&custom_http_host);
  wifiManager.addParameter(&custom_http_port);
  wifiManager.addParameter(&custom_http_url);
  wifiManager.addParameter(&custom_gw_id);
  wifiManager.addParameter(&custom_firmware);
  wifiManager.addParameter(&custom_proxy_authorization_header);
  // Reset settings for testing
  //wifiManager.resetSettings();
  // Set minimum quality of signal so it ignores AP's under that quality (defaults to 8%)
  //wifiManager.setMinimumSignalQuality();
  //sets timeout until configuration portal gets turned off
  //wifiManager.setTimeout(120);

  String APName = String("Wifi Gateway") + " (" + ESP.getChipId() + ")";
  bool connectionResult = autoConnect ? wifiManager.autoConnect(APName.c_str()) : wifiManager.startConfigPortal(APName.c_str());
  if (!connectionResult) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  Serial.println("connected...yeey :)");

  strcpy(this->httpHost, custom_http_host.getValue());
  strcpy(this->httpPort, custom_http_port.getValue());
  strcpy(this->httpUrl, custom_http_url.getValue());
  strcpy(this->gatewayId, custom_gw_id.getValue());
  strcpy(this->firmware, custom_firmware.getValue());
  strcpy(this->proxyAuthorizationHeader, custom_proxy_authorization_header.getValue());

  //if (this->shouldSaveConfig) {
    this->saveConfigFile();
  //  this->shouldSaveConfig = false;
  //}

  Serial.println("ESP8266 IP");
  Serial.println(WiFi.localIP());
}

void GatewayConfiguration::readConfigFile() {
  //clean FS, for testing
  //SPIFFS.format();

  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("Config file opened");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(this->httpHost, json["http_host"]);
          strcpy(this->httpPort, json["http_port"]);
          strcpy(this->httpUrl, json["http_url"]);
          strcpy(this->gatewayId, json["gateway_id"]);
          strcpy(this->firmware, json["firmware"]);
          strcpy(this->proxyAuthorizationHeader, json["proxyAuthorizationHeader"]);
        } else {
          Serial.println("Failed to load json config");
        }
      }
    } else {
      Serial.println("Config file doesn't exists");
    }
  } else {
    Serial.println("Failed to mount FS");
  }
}

bool GatewayConfiguration::saveConfigFile() {
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["http_host"] = this->httpHost;
  json["http_port"] = this->httpPort;
  json["http_url"] = this->httpUrl;
  json["gateway_id"] = this->gatewayId;
  json["firmware"] = this->firmware;
  json["proxyAuthorizationHeader"] = this->proxyAuthorizationHeader;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
    return false;
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();

  return true;
}

void GatewayConfiguration::saveConfigCallback() {
  Serial.println("Should save config");
  this->shouldSaveConfig = true;
}
