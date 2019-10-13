#include "Arduino.h"
#include "unity.h"

#ifdef UNIT_TEST

CanBusId instanceUnderTest;

void testSetsId() {
  instanceUnderTest.id(42);
  TEST_ASSERT_EQUAL(instanceUnderTest.id(), 42);
  instanceUnderTest.id(0x234567);
  TEST_ASSERT_EQUAL(instanceUnderTest.id(), 0x234567);
}

#endif
