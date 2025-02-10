// You can use this file to write your own code for testing the library
// The file is ready for debugging with the AVR8-stub library.
// Just uncomment the line with the "debug only" comment and use the "breakpoint()" function to stop the execution and start the debugging.

#include <Arduino.h>
#include "MIC24CSM01.h"

// #include <avr8-stub.h> // debug only

Mem24CSM01 memory(false, false);
uint16_t config;

void setup()
{
//  debug_init(); // debug only

  memory.begin(); // Initialize the I2C bus

  // Reading the EEPROM Configuration DONE WORK!
  // config = memory.getConfiguration();
  // explainConfig(config);

  // Reading serial number from security register DONE WORK!
  // uint8_t data[SERIAL_NUMBER_BYTE_SIZE];
  // if (memory.getSerialNumber(data, sizeof(data)))
  // {
  //   // do something with the data
  // }

  // Get the manufacturer register. The answer must be 00D0D0h DONE WORK!
  // uint32_t manufacturer = memory.getManufacturerRegister();
  // uint8_t byte0 = manufacturer & 0xFF;
  // uint8_t byte1 = (manufacturer >> 8) & 0xFF;
  // uint8_t byte2 = (manufacturer >> 16) & 0xFF;

  // Enabling the Software Write Protection DONE WORK!
  // memory.enableSoftwareWriteProtect();
  // config = memory.getConfiguration();
  // explainConfig(config);

  // Disabling the Software Write Protection DONE WORK!
  // memory.disableSoftwareWriteProtect();
  // delay(500);
  // config = memory.getConfiguration();
  // explainConfig(config);

  // Setting the Write Protection Zone DONE WORK!
  // memory.setWriteProtectionZone(2);
  // delay(500);
  // config = memory.getConfiguration();
  // explainConfig(config);

  // Setting the Write Protection Zones with a bitmask DONE WORK!
  // memory.writeProtection(0b00010000);
  // delay(500);
  // config = memory.getConfiguration();
  // explainConfig(config);

  // Removing the Write Protection Zone DONE WORK!
  // memory.removeWriteProtectionZone(4);
  // delay(500);
  // config = memory.getConfiguration();
  // explainConfig(config);

  // Writing a single byte to the EEPROM DONE WORK!
  // MEMORYRESULT res;
  // res = memory.write(0x0000, 0x33);
  // breakpoint(); // debug only purpose
  // if (res == OK)
  // {
  //   /* code */
  // }

  // Writing a array of bytes to the EEPROM DONE WORK!
  // uint8_t buffer[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
  // MEMORYRESULT res;
  // res = memory.write(0x0000, buffer, 5);
  // breakpoint(); // debug only purpose
  // if (res == OK)
  // {
  //   /* code */
  // }

  // Reading a single byte from the EEPROM based on the current address pointer DONE WORK!
  // uint8_t data;
  // MEMORYRESULT res;
  // res = memory.read(&data);
  // breakpoint(); // debug only purpose
  // if (res == OK) {}

  // Reading data from the EEPROM address DONE WORK!
  // uint8_t data;
  // MEMORYRESULT res;
  // res = memory.read(0x0000, &data);
  // breakpoint(); // debug only purpose
  // if (res == OK) {}

  // Reading a block of data from the EEPROM DONE WORK!
  // uint8_t buffer[5];
  // MEMORYRESULT res;
  // res = memory.read(0x0000, buffer, 5);
  // breakpoint(); // debug only
  // if (res == OK) {}
}

void loop()
{
}
