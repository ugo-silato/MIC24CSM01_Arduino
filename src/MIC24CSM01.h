/*
  Mem24CSM01 - Library for using Mem24CSM01 EEPROM chip
  Author: Ugo Silato hispired from Jerry Magnin library https://github.com/jerry-magnin/Mem24CSM01/
  More chip resources: https://www.microchip.com/en-us/product/24csm01
  Licence: MIT
*/

#ifndef MIC24CSM01_h
#define MIC24CSM01_h

#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

// Wire library accept only 7-bit addresses!
#define BASE_MEMREG_ADDR 0b1010000 // This is the default device address type for the memory register
#define BASE_CFGREG_ADDR 0b1011000 // This is the default device address type for the configuration register

#define CFGREG_WRD_ADDRH 0b10001000 // First word address byte trasmitted to the device immediately after the device address type for configuration register
#define CFGREG_WRD_ADDRL 0b00000000 // Second word address byte trasmitted to the device immediately after the first word for configuration register

#define SECREG_WRD_ADDRH 0b00001000 // First word address byte trasmitted to the device immediately after the device address type for security register
#define SECREG_WRD_ADDRL 0b00000000 // Second word address byte trasmitted to the device immediately after the first word for security register
#define SERIAL_NUMBER_BYTE_SIZE 16       // The size of the serial number array

#define FIRST_RESERVED_HOST_CODE 0b1111100  // Reserved host code for accessing the manufacturer register
#define SECOND_RESERVED_HOST_CODE 0b1111100 // Reserved host code for accessing the manufacturer register

#define ECS_MASK 0b1 << 15 // Error Correction State mask
#define EWPM_MASK 0b1 << 9 // Enhanced Software Write Protection Mode mask
#define LOCK_MASK 0b1 << 8 // Configuration Register Lock mask

// #define CFGREG_EWPM 9
// #define CFGREG_LOCK 8

#define REGISTER_LOCKED 0x99 // Attention! This is the value write to the configuration register lock it permanently
#define REGISTER_UNLOCKED 0x66

#define MAX_MEMORY_ADDRESS_VALUE 0x1FFFF // Maximum memory address value
#define MAX_MEMORY_PAGE_SIZE 256         // A page write operation allows up to 256 bytes to be written in the same write cycle

/**
 * @enum MEMORYRESULT
 * @brief Enumeration to represent the result of memory operations.
 *
 * This enumeration defines various possible outcomes of memory operations.
 *
 * @var MEMORYRESULT::OK
 * Operation completed successfully.
 *
 * @var MEMORYRESULT::ADDRESS_EXCEEDS_LIMIT
 * The specified address exceeds the memory limit.
 *
 * @var MEMORYRESULT::BUFFER_TOO_LARGE
 * The provided buffer is too large for the operation.
 *
 * @var MEMORYRESULT::NOT_ON_SINGLE_PAGE
 * The operation does not fit within a single memory page.
 *
 * @var MEMORYRESULT::ADDRESS_ERROR
 * There was an error with the specified address.
 *
 * @var MEMORYRESULT::DATA_ERROR
 * There was an error with the data.
 *
 * @var MEMORYRESULT::TIMEOUT
 * The operation timed out.
 *
 * @var MEMORYRESULT::GENERIC_ERROR
 * A generic error occurred during the operation.
 */
typedef enum
{
  OK,
  ADDRESS_EXCEEDS_LIMIT,
  BUFFER_TOO_LARGE,
  NOT_ON_SINGLE_PAGE,
  ADDRESS_ERROR,
  DATA_ERROR,
  TIMEOUT,
  GENERIC_ERROR,
} MEMORYRESULT;

// Configuration register structure
/*
bit 15: Error Correction State (ECS)
       (1 = The previously executed read operation did require the use of Error Correction Code (ECC),
        0 = No ECC required)
bit 14-10: Unimplemented: Read as ‘0’
bit 9: EWPM:  Enhanced Software Write Protection Mode
       (1 = Enhanced Protection: WP pin is treated as a “don’t care”, and the memory array is protected in accordance with the SWP bits defined in Register,
        0 = Legacy Protection (factory default): Entire memory array and Security register contents are protected via the WP pin)
bit 8: LOCK: Configuration Register Lock
       (1 = Configuration register is locked PERMANENT LOCKED!, 0 = Configuration register is not locked - default)
bit 7 SWP7: Software Write Protection Memory Zone (1C000h - 1FFFFh)
bit 6 SWP6: Software Write Protection Memory Zone (18000h - 1BFFFh)
bit 5 SWP5: Software Write Protection Memory Zone (14000h - 17FFFh)
bit 4 SWP4: Software Write Protection Memory Zone (10000h - 13FFFh)
bit 3 SWP3: Software Write Protection Memory Zone (0C000h - 0FFFFh)
bit 2 SWP2: Software Write Protection Memory Zone (08000h - 0BFFFh)
bit 1 SWP1: Software Write Protection Memory Zone (04000h - 07FFFh)
bit 0 SWP0: Software Write Protection Memory Zone (00000h - 03FFFh)
*/

/**
 * @struct ConfigurationRegister
 * @brief Represents the configuration register of the EEPROM 24CSM01.
 *
 * This structure holds various configuration settings and status flags for the EEPROM.
 *
 * @var ConfigurationRegister::isErrorCorrectionOccured
 * Indicates whether Error Correction Code (ECC) was required.
 * - true: ECC was required.
 * - false: No ECC required.
 *
 * @var ConfigurationRegister::isSoftwareWriteProtect
 * Indicates the mode of software write protection.
 * - true: Enhanced Software Write Protection Mode.
 * - false: Legacy Protection.
 *
 * @var ConfigurationRegister::isConfigLocked
 * Indicates whether the configuration register is locked.
 * - true: Configuration register is PERMANENTLY locked.
 * - false: Configuration register is not locked.
 *
 * @var ConfigurationRegister::zoneProtection
 * Represents the write protection status of each zone.
 * Each bit corresponds to a zone:
 * - 0: Write protection disabled.
 * - 1: Write protection enabled.
 */
typedef struct
{

  bool isErrorCorrectionOccured; // 1 = Error Correction Code (ECC)  was required, 0 = No ECC required
  bool isSoftwareWriteProtect;   // 1 = Enhanced Software Write Protection Mode, 0 = Legacy Protection
  bool isConfigLocked;           // 1 = Configuration register is locked, 0 = Configuration register is not locked
  uint8_t zoneProtection;        // Every bit represents a zone, 0 = write protection disabled, 1 = write protection enabled
} ConfigurationRegister;

typedef struct
{
  uint16_t manufacturer;  // Manufacturer identification
  uint16_t deviceDensity; // Device density
  uint8_t deviceRevision; // Device revision
} ManufacturerRegister;

/**
 * @struct WriteAddressPacket
 * @brief Structure to represent the address packet for writing to the EEPROM.
 *
 * This structure holds the necessary information to specify the address in the EEPROM
 * where data should be written.
 *
 * @var WriteAddressPacket::deviceMemoryAddress
 * The address of the memory device.
 *
 * @var WriteAddressPacket::memoryMSB
 * The most significant byte of the memory address.
 *
 * @var WriteAddressPacket::memoryLSB
 * The least significant byte of the memory address.
 */
typedef struct
{
  uint8_t deviceMemoryAddress;
  uint8_t memoryMSB;
  uint8_t memoryLSB;
} WriteAddressPacket;

uint8_t zoneProtection(bool zone7 = 0, bool zone6 = 0, bool zone5 = 0, bool zone4 = 0, bool zone3 = 0, bool zone2 = 0, bool zone1 = 0, bool zone0 = 0);

class Mem24CSM01
{
public:
  Mem24CSM01(uint8_t memoryRegister);
  Mem24CSM01(bool A1, bool A2);
  void begin();

  uint16_t getConfiguration();
  bool getSerialNumber(uint8_t *data, uint8_t arraySize);
  uint64_t getManufacturerRegister();
  bool updateConfigRegister(uint8_t confirmLock = 0x66);
  bool enableSoftwareWriteProtect();
  bool disableSoftwareWriteProtect();
  bool setWriteProtectionZone(uint8_t zone);
  bool writeProtection(uint8_t zones);
  bool removeWriteProtectionZone(uint8_t zone);

  MEMORYRESULT read(uint8_t *data);
  MEMORYRESULT read(uint32_t address, uint8_t *data);
  MEMORYRESULT read(uint32_t address, uint8_t *buffer, size_t size);
  MEMORYRESULT write(uint32_t address, uint8_t singleByte);
  MEMORYRESULT write(uint32_t address, uint8_t *dataArray, size_t arraySize);

private:
  WriteAddressPacket configureAddressPacket(uint32_t address);
  MEMORYRESULT processTransmissionResult(int transmissionResult);
  uint8_t addressMemoryPointer(uint32_t address);
  uint8_t m_dev_address_memory_access;     // Device address byte for Memory access
  uint8_t m_dev_address_configuration_reg; // Device address byte for Configuration register access
  uint8_t m_dev_address_security_register; // Device address byte for Security register access
  ConfigurationRegister m_configuration;   // Configuration register
  ManufacturerRegister m_manufacturer;     // Manufacturer identification register
};

#endif