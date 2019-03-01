#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <Arduino.h>

class Message
{
public:
  Message() {

  };

  Message(String firmwareVersion, String gatewayId, String payload, String timestamp) {
    this->firmwareVersion = firmwareVersion;
    this->gatewayId = gatewayId;
    this->payload = payload;
    this->timestamp = timestamp;
  };

  ~Message() {

  };

  String getFirmwareVersion() const {
    return firmwareVersion;
  }
  String getGatewayId() const {
    return gatewayId;
  }
  String getPayload() const {
    return payload;
  }
  String getTimestamp() const {
    return timestamp;
  }

private:
  String firmwareVersion;
  String gatewayId;
  String payload;
  String timestamp;
};

#endif
