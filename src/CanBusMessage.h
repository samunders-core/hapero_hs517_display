#include "Arduino.h"
#include "mcp_can.h"
#include "CanBusId.h"
#include "print_utils.h"

struct CanBusMessage : public CanBusId {
  byte count;
  byte bytes[8];
public:
  static const byte UNINITIALIZED;

  CanBusMessage() : count (UNINITIALIZED) {}

  CanBusMessage(MCP_CAN& canBus) {
    recv(canBus);
  }

  CanBusMessage& add(byte value) {
    return set(count, value);
  }

  CanBusMessage& set(byte index, byte value) {
    if (count == UNINITIALIZED) {
      count = 1;
    } else if (count <= index) {
      count = 1 + index;
    }
    bytes[index] = value;
    return *this;
  }

  CanBusMessage& reset(byte newCount = UNINITIALIZED) {
    count = newCount;
    return *this;
  }

  byte size() {
    return count;
  }

  CanBusMessage& recv(MCP_CAN& canBus) {
    byte rv = canBus.readMsgBufID(&_id.value, &count, bytes);
    if (CAN_OK == rv) {
      CanBusId::set(EXTENDED, canBus.isExtendedFrame());
      CanBusId::set(RTR, canBus.isRemoteRequest());
    } else {
      reset();
      if (CAN_NOMSG != rv) {
        printfTo(Serial, "Error %d Receiving Message...", rv);
      }
    }
    return *this;
  }

  CanBusMessage& send(MCP_CAN& canBus, byte newCount = UNINITIALIZED) {
    if (size() != UNINITIALIZED) {
      byte rv = canBus.sendMsgBuf(id(), is(EXTENDED), is(RTR), count, bytes);
      if(rv == CAN_OK) {
        reset(newCount);
      } else {
        printfTo(Serial, "Error %d Sending Message...", rv);
      }
    }
    return *this;
  }

  CanBusMessage& to(Stream& stream) {
    stream.print(count, DEC);
    stream.print(" bytes from ");
    CanBusId::to(stream);
    char separator[3] = ": ";
    for (byte index = 0, separatorIndex = 0; index < count; ++index, separatorIndex = 1) {
      stream.print(&separator[separatorIndex]);
      print_zero_hex(stream, bytes[index]);
    }
    return *this;
  }
};

const byte CanBusMessage::UNINITIALIZED = 255;
