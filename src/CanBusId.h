#include "mcp_can.h"
#include "print_utils.h"

struct CanBusId {
  static const byte EXTENDED, RTR;
#define MSB 3

  byte is(byte kind) {
    return _id.bytes[MSB] & kind;
  }

  unsigned long id(unsigned long value = 0x20000000) {  // unused bit marks illegal id
    if (0x20000000 != value) {
      _id.value = value;
      set(EXTENDED, value > 0x7FF);
      return value;
    }
    return is(EXTENDED) ? (_id.value & 0x1FFFFFFF) : (_id.value & 0x7FF);
  }

  void set(byte bit, byte value = 1) {
    _id.bytes[MSB] = value ? _id.bytes[MSB] | bit : (_id.bytes[MSB] & (~bit));
  }

  void to(Stream& stream) {
    stream.print("0x");
    print_zero_hex(stream, id());
    stream.print("(");
    stream.print(is(RTR) ? "rtr, " : "");
    stream.print(is(EXTENDED) ? 29 : 11, DEC);
    stream.print("bit)");
  }
protected:
  union {
    unsigned long value;
    byte bytes[4];
  } _id;
};

const byte CanBusId::EXTENDED = 0x40;
const byte CanBusId::RTR = 0x80;
