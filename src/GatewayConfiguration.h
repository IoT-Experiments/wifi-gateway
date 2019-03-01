#ifndef _GATEWAY_CONFIGURATION_H_
#define _GATEWAY_CONFIGURATION_H_

#include <Arduino.h>

#define FIELD_HOST_LENGTH 100
#define FIELD_PORT_LENGTH 7
#define FIELD_URL_LENGTH 150
#define FIELD_ID_LENGTH 37
#define FIELD_FIRMWARE_LENGTH 4
#define FIELD_PROXY_AUTHORIZATION_HEADER_LENGTH 100

class GatewayConfiguration
{
public:
  GatewayConfiguration();
  ~GatewayConfiguration();

  void startWifiConfiguration(bool autoConnect);
  void readConfigFile();
  bool saveConfigFile();
  void saveConfigCallback();

  String getHttpHost() {
    return String(this->httpHost);
  }
  uint32 getHttpPort() {
    return atoi(this->httpPort);
  }
  String getHttpUrl() {
    return String(this->httpUrl);
  }
  String getGatewayId() {
    return String(this->gatewayId);
  }
  String getFirmware() {
    return String(this->firmware);
  }
  String getProxyAuthorizationHeader() {
    return String(this->proxyAuthorizationHeader);
  }

private:
  bool shouldSaveConfig = false;
  char httpHost[FIELD_HOST_LENGTH];
  char httpPort[FIELD_PORT_LENGTH];
  char httpUrl[FIELD_URL_LENGTH];
  char gatewayId[FIELD_ID_LENGTH];
  char firmware[FIELD_FIRMWARE_LENGTH];
  char proxyAuthorizationHeader[FIELD_PROXY_AUTHORIZATION_HEADER_LENGTH];
};

#endif
