#ifndef _CLOUD_CLIENT_H_
#define _CLOUD_CLIENT_H_

#include <Arduino.h>

class CloudClient
{
public:
  CloudClient();
  CloudClient(String httpHost, uint32 httpPort, String httpUrl, String proxyAuthorizationHeader);
  ~CloudClient();

  void updateEndpoint(String httpHost, uint32 httpPort, String httpUrl, String proxyAuthorizationHeader) {
    this->httpHost = httpHost;
    this->httpPort = httpPort;
    this->httpUrl = httpUrl;
    this->proxyAuthorizationHeader = proxyAuthorizationHeader;
  }

  bool sendWifiMessage(String firmwareVersion, String gatewayId, String payload, String timestamp);

  void setHttpHost(String host) {
    this->httpHost = host;
  }
  void setHttpPort(uint32 port) {
    this->httpPort = port;
  }
  void setHttpUrl(String url) {
    this->httpUrl = url;
  }
private:
  String httpHost;
  uint32 httpPort;
  String httpUrl;
  String proxyAuthorizationHeader;
};

#endif
