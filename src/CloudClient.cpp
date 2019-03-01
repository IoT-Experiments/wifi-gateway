#include "CloudClient.h"
#include <WiFiManager.h>

#define AVAILABLE_TIMEOUT 5000

CloudClient::CloudClient() {
}

CloudClient::CloudClient(String httpHost, uint32 httpPort, String httpUrl, String proxyAuthorizationHeader) {
  this->httpHost = httpHost;
  this->httpPort = httpPort;
  this->httpUrl = httpUrl;
  this->proxyAuthorizationHeader = proxyAuthorizationHeader;
}

CloudClient::~CloudClient() {

}

bool CloudClient::sendWifiMessage(String firmwareVersion, String gatewayId, String payload, String timestamp) {
  bool sendSuccess = false;

  WiFiClient* client = this->httpPort == 443 ? new WiFiClientSecure() : new WiFiClient();
  Serial.printf("\n[Connecting to %s ... ", this->httpHost.c_str());
  if (client->connect(this->httpHost.c_str(), this->httpPort)) {
    Serial.println("connected]");

    Serial.println("[Sending a request]");
    String request = String("POST ") + this->httpUrl + " HTTP/1.1\r\n" +
                 "Host: " + this->httpHost + ":" + this->httpPort + "\r\n" +
                 "Content-Type: text/plain\r\n" +
                 "X-fwv: " + firmwareVersion + "\r\n" +
                 "X-gwid: " + gatewayId + "\r\n" +
                 "X-WiFi-Gateway: true\r\n" +
                 "Content-Length: " + payload.length() + "\r\n";
//                 "Connection: Keep-Alive\r\n" +
//                 "Connection: close\r\n" +
    if(this->proxyAuthorizationHeader != NULL && this->proxyAuthorizationHeader.length() > 0) {
      request += "Proxy-Authorization: " + this->proxyAuthorizationHeader + "\r\n";
    }

    request += "\r\n" + payload;

    Serial.println(request);
    client->print(request);

    Serial.println("\r\n[Response:]");
    unsigned long timeout = millis();
    while (client->connected() == 0) {
      if(millis() - timeout > AVAILABLE_TIMEOUT) {
        Serial.println("-> Available timeout !");
        break;
      }
    }

    while(client->available()) {
      String line = client->readStringUntil('\n');
      Serial.println(line);
      if(line.indexOf("HTTP/1.1 200 OK") == 0) {
        Serial.println("Stop reading message as OK...");
        break;
      }
    }
    sendSuccess = true; // - TODO : improve response management
    client->stop();
    Serial.println("[Disconnected]");
  }
  else {
    Serial.println("connection failed!]");
    client->stop();
  }

  delete client;
  return sendSuccess;
}
