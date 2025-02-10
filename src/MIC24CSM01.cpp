#include "MIC24CSM01.h"

/**
 * @brief Generates a protection pattern based on the input zone protection flags.
 *
 * This function takes eight boolean parameters representing the protection status of
 * eight different zones (zone7 to zone0). It constructs a single byte (uint8_t) where
 * each bit corresponds to the protection status of a zone, with zone7 being the most
 * significant bit and zone0 being the least significant bit.
 * The chip 24CSM01 has 8 zones.
 *
 * @param zone7 Protection status of zone 7 (most significant bit).
 * @param zone6 Protection status of zone 6.
 * @param zone5 Protection status of zone 5.
 * @param zone4 Protection status of zone 4.
 * @param zone3 Protection status of zone 3.
 * @param zone2 Protection status of zone 2.
 * @param zone1 Protection status of zone 1.
 * @param zone0 Protection status of zone 0 (least significant bit).
 * @return uint8_t A byte where each bit represents the protection status of a zone.
 */
uint8_t zoneProtection(bool zone7, bool zone6, bool zone5, bool zone4, bool zone3, bool zone2, bool zone1, bool zone0)
{
  uint8_t protectionPattern = 0;
  protectionPattern |= (zone7 << 7) | (zone6 << 6) | (zone5 << 5) | (zone4 << 4) | (zone3 << 3) | (zone2 << 2) | (zone1 << 1) | zone0;
  return (protectionPattern);
}

/**
 * @brief Constructor for the Mem24CSM01 class.
 *  This constructor initializes the memory access word and configures the
 * memory configuration register.
 *
 * @param word_mem_acc The memory register value.
 */
Mem24CSM01::Mem24CSM01(uint8_t word_mem_acc)
{
  m_dev_address_memory_access = word_mem_acc;
  m_dev_address_configuration_reg = m_dev_address_memory_access | (1 << 4); // from 0b1010xxxx to 0b1011xxxx
}

/**
 * @brief Constructor for the Mem24CSM01 class.
 *
 * This constructor initializes the memory access and configuration register addresses
 * based on the provided A1 and A2 address bits.
 *
 * @param A1 The A1 address bit (Chip PIN A1 (2) VCC = 1, VSS = 0).
 * @param A2 The A2 address bit (Chip PIN A2 (3) VCC = 1, VSS = 0).
 */
Mem24CSM01::Mem24CSM01(bool A1, bool A2)
{
  m_dev_address_memory_access = BASE_MEMREG_ADDR | (A2 << 3) | (A1 << 2);     // 0b1010 A2 A1 00
  m_dev_address_configuration_reg = BASE_CFGREG_ADDR | (A2 << 3) | (A1 << 2); // 0b1011 A2 A1 00
  m_dev_address_security_register = m_dev_address_configuration_reg;          // 0b1011 A2 A1 00
}

/**
 * @brief Initializes the Mem24CSM01 device.
 *
 * This function initializes the I2C bus, preparing the device for communication.
 */
void Mem24CSM01::begin()
{
  Wire.begin(); // Initialize the I2C bus
}

/**
 * @brief Reads the configuration register from the 24CSM01 EEPROM device.
 *
 * This function communicates with the 24CSM01 EEPROM device to read its configuration
 * register. It sends the address of the configuration register, requests two bytes of
 * data, and then processes these bytes to extract various configuration settings.
 *
 * @return uint16_t The 16-bit value read from the configuration register.
 *
 * - Extracts and updates the following configuration settings:
 *   - `zoneProtection`: The lower byte of the configuration register.
 *   - `isConfigLocked`: The LOCK bit from the configuration register.
 *   - `isSoftwareWriteProtect`: The EWPM bit from the configuration register.
 *   - `isErrorCorrectionOccured`: The ECC bit from the configuration register.
 */
uint16_t Mem24CSM01::getConfiguration()
{
  uint16_t result;   // Variable to store the two concatenated bytes read from the device
  uint8_t low, high; // Variables to store the two single bytes read from the device

  Wire.beginTransmission(m_dev_address_configuration_reg);                      // Start the transmission with the device
  Wire.write(CFGREG_WRD_ADDRH);                                                 // Write the first word address byte
  Wire.write(CFGREG_WRD_ADDRL);                                                 // Write the second word address byte
  Wire.endTransmission(false);                                                  // Send a restart message to keep the bus open
  Wire.requestFrom(m_dev_address_configuration_reg, (uint8_t)2, (uint8_t)true); // Request 2 bytes from the device
  high = Wire.read();                                                           // Read the first byte
  low = Wire.read();                                                            // Read the second byte
  result = (high << 8) | low;                                                   // Concatenate the two bytes
  m_configuration.zoneProtection = low;
  m_configuration.isConfigLocked = result & LOCK_MASK;          // Extract the LOCK bit
  m_configuration.isSoftwareWriteProtect = result & EWPM_MASK;  // Extract the EWPM bit
  m_configuration.isErrorCorrectionOccured = result & ECS_MASK; // Extract the ECC bit
  return (result);
}

/**
 * @brief Retrieves the serial number from the EEPROM device.
 *
 * This function reads the serial number from the security register of the EEPROM device
 * and stores it in the provided data array. The array size must match the expected serial
 * number byte size.
 *
 * @param data Pointer to the array where the serial number will be stored.
 * @param arraySize Size of the provided data array. Must be equal to SERIAL_NUMBER_BYTE_SIZE.
 * @return true if the serial number was successfully retrieved, false otherwise.
 */
bool Mem24CSM01::getSerialNumber(uint8_t *data, uint8_t arraySize)
{
  if (arraySize != SERIAL_NUMBER_BYTE_SIZE) // Check if the array size is correct
  {
    return (false);
  }
  Wire.beginTransmission(m_dev_address_security_register);
  Wire.write(SECREG_WRD_ADDRH);
  Wire.write(SECREG_WRD_ADDRL);
  Wire.endTransmission(false);
  Wire.requestFrom(m_dev_address_security_register, (uint8_t)16, (uint8_t)true); // Request 16 bytes from the device
  for (int nBytes = 0; nBytes < arraySize; nBytes++)
  {
    data[nBytes] = Wire.read();
  }
  return (true);
}

/**
 * @brief Retrieves the manufacturer register value from the EEPROM device.
 *
 * This function communicates with the EEPROM device to read the manufacturer
 * register, which consists of three bytes. The bytes are concatenated into a
 * single 32-bit unsigned integer and returned.
 *
 * @return uint64_t The concatenated manufacturer register value.
 */
uint64_t Mem24CSM01::getManufacturerRegister() // TODO return error code
{
  uint32_t result; // Variable to store the two concatenated bytes read from the device

  Wire.beginTransmission(FIRST_RESERVED_HOST_CODE);
  Wire.write(m_dev_address_memory_access << 1);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)SECOND_RESERVED_HOST_CODE, (uint8_t)3, (uint8_t)true); // Request 3 bytes from the device
  uint8_t byte0, byte1, byte2;
  byte0 = Wire.read(); // Read the first byte
  byte1 = Wire.read(); // Read the second byte
  byte2 = Wire.read(); // Read the third byte
  result = (static_cast<uint32_t>(byte0) << 16) | (static_cast<uint32_t>(byte1) << 8) |
           static_cast<uint32_t>(byte2); // Concatenate the three bytes
  return (result);
}

// Parameters: confirmLock - 66h unlocked, 99h locked - default is unlocked
/**
 * @brief Updates the configuration register of the Mem24CSM01 device.
 *
 * This function prepares the configuration bytes based on the current
 * configuration settings and writes them to the device's configuration
 * register. The configuration includes software write protection,
 * configuration lock status, and zone protection settings.
 *
 * @param confirmLock A byte used to confirm the locking of the configuration.
 *                     This should be provided by the caller to ensure the
 *                     configuration is locked as intended. 66h unlocked, 99h locked
 *                     - default is unlocked
 *
 * @return bool Returns true if the configuration update was successful,
 *              false otherwise.
 */
bool Mem24CSM01::updateConfigRegister(uint8_t confirmLock)
{
  // Preparing the config bytes
  uint8_t cfgHighByte = 0 | (m_configuration.isSoftwareWriteProtect << 1) | m_configuration.isConfigLocked;
  uint8_t cfgLowByte = m_configuration.zoneProtection;

  // Writing the configuration bytes to the device
  Wire.beginTransmission(m_dev_address_configuration_reg); // Start the transmission with the device
  Wire.write(CFGREG_WRD_ADDRH);                            // Write the first word address byte
  Wire.write(CFGREG_WRD_ADDRL);                            // Write the second word address byte
  Wire.write(cfgHighByte);                                 // Write the high byte of the configuration
  Wire.write(cfgLowByte);                                  // Write the low byte of the configuration
  Wire.write(confirmLock);                                 // Write the confirmation byte
  if (Wire.endTransmission() != 0)                         // End the transmission and check for errors
  {
    return (false);
  }
  return (true);
}


bool Mem24CSM01::enableSoftwareWriteProtect()
{
  m_configuration.isSoftwareWriteProtect = true; // Set the software write protection bit
  return updateConfigRegister();                 // Update the configuration register
}

/**
 * @brief Enables the software write protection for the EEPROM.
 *
 * This function sets the software write protection bit in the configuration
 * register, preventing any further write operations to the EEPROM until the
 * write protection is disabled.
 *
 * @return true if the operation was successful, false otherwise.
 */
bool Mem24CSM01::disableSoftwareWriteProtect()
{
  m_configuration.isSoftwareWriteProtect = false; // Clear the software write protection bit
  return updateConfigRegister();                  // Update the configuration register
}

/**
 * @brief Sets the write protection zone for the EEPROM.
 *
 * This function sets the write protection zone for the EEPROM by updating the
 * zoneProtection configuration. The zone must be between 0 and 7 inclusive.
 *
 * @param zone The zone to set write protection for. Valid values are from 0 to 7.
 * @return true if the zone is valid and the write protection is set successfully, false otherwise.
 */
bool Mem24CSM01::setWriteProtectionZone(uint8_t zone)
{
  if (zone >= 0 && zone <= 7)
  {
    uint8_t temp_zoneProtection = bitSet(m_configuration.zoneProtection, zone);
    m_configuration.zoneProtection = temp_zoneProtection;
    if (updateConfigRegister())
      return (true);
  }

  return (false);
}

/**
 * @brief Sets the write protection zones for the memory.
 *
 * This function configures the write protection zones by updating the
 * zoneProtection member of the m_configuration structure and then
 * calls updateConfigRegister() to apply the changes.
 *
 * @param zones The bitmask representing the zones to be write-protected.
 */
bool Mem24CSM01::writeProtection(uint8_t zones)
{
  m_configuration.zoneProtection = zones;
  return updateConfigRegister();
}

/**
 * @brief Removes the write protection from a specified zone.
 *
 * This function clears the write protection bit for the specified zone
 * in the configuration register. The zone must be between 0 and 7 inclusive.
 *
 * @param zone The zone number to remove write protection from (0-7).
 * @return true if the write protection was successfully removed, false otherwise.
 */
bool Mem24CSM01::removeWriteProtectionZone(uint8_t zone)
{
  if (zone >= 0 && zone <= 7)
  {
    uint8_t temp_zoneProtection = bitClear(m_configuration.zoneProtection, zone);
    m_configuration.zoneProtection = temp_zoneProtection;
    if (updateConfigRegister())
      return (true);
  }

  return (false);
}

/**
 * @brief Writes a single byte to the specified address in the EEPROM.
 *
 * This function writes a single byte to the EEPROM at the given address.
 * It internally calls the overloaded write function that handles writing
 * multiple bytes, passing the address, a pointer to the single byte, and
 * the length of 1.
 *
 * @param address The address in the EEPROM where the byte should be written.
 * @param singleByte The byte value to be written to the EEPROM.
 * @return MEMORYRESULT indicating the result of the write operation.
 */
MEMORYRESULT Mem24CSM01::write(uint32_t address, uint8_t singleByte)
{
  return (write(address, &singleByte, 1));
}

/**
 * @brief Writes a block (Max 256 bytes) of data to the EEPROM memory.
 *
 * This function writes a block of data to the EEPROM memory starting at the specified address.
 * It performs several checks to ensure the write operation is valid:
 * - The address must be within the memory limits.
 * - The size of the data block must not exceed the maximum page size (256 bytes).
 * - The write operation must be contained within a single memory page.
 *
 * @param address The starting address in the EEPROM memory where the data will be written.
 * @param dataArray A pointer to the array of data to be written to the EEPROM memory.
 * @param arraySize The size of the data array.
 * @return MEMORYRESULT The result of the write operation, indicating success or the type of error encountered.
 *
 * Possible return values:
 * - MEMORYRESULT::ADDRESS_EXCEEDS_LIMIT: The specified address exceeds the memory limits.
 * - MEMORYRESULT::BUFFER_TOO_LARGE: The size of the data block exceeds the maximum page size.
 * - MEMORYRESULT::NOT_ON_SINGLE_PAGE: The write operation spans across multiple memory pages.
 * - Other values indicating the result of the I2C transmission.
 */
MEMORYRESULT Mem24CSM01::write(uint32_t address, uint8_t *dataArray, size_t arraySize)
{
  if (address > MAX_MEMORY_ADDRESS_VALUE) // Check if the address is within the memory limits
  {
    return (MEMORYRESULT::ADDRESS_EXCEEDS_LIMIT);
  }
  if (arraySize > MAX_MEMORY_PAGE_SIZE) // Check if the buffer size is within the limits
  {
    return (MEMORYRESULT::BUFFER_TOO_LARGE);
  }
  if ((address + arraySize - 1) > 0xFF) // Check if the write operation is within a single page
  {
    return (MEMORYRESULT::NOT_ON_SINGLE_PAGE);
  }

  WriteAddressPacket writeAddressPacket = configureAddressPacket(address); //
  Wire.beginTransmission(writeAddressPacket.deviceMemoryAddress);
  Wire.write(writeAddressPacket.memoryMSB);
  Wire.write(writeAddressPacket.memoryLSB);
  for (unsigned int i = 0; i < arraySize; ++i)
  {
    Wire.write(dataArray[i]);
  }

  int transmissionResult = Wire.endTransmission();
  return (processTransmissionResult(transmissionResult));
}

/**
 * @brief Reads a byte of data from the EEPROM at the current address pointer.
 *
 * This function initiates a transmission to the EEPROM device, requests one byte
 * of data from the current address pointer, and stores the received byte in the
 * provided data buffer. The 24CSM01 contains an internal Address Pointer that
 * maintains the word address of the last byte accessed, internally incremented by
 * one. Therefore, if the previous read access was to address ‘n’ (n is any legal
 * address), the next current address read operation would access data from address ‘n+1’.
 *
 * @param data Pointer to a buffer where the read byte will be stored.
 * @return MEMORYRESULT::OK if the read operation was successful.
 *         MEMORYRESULT::GENERIC_ERROR if the read operation failed.
 */
MEMORYRESULT Mem24CSM01::read(uint8_t *data)
{ // Read at current adress pointer
  Wire.beginTransmission(m_dev_address_memory_access);
  Wire.endTransmission();
  if (Wire.requestFrom(m_dev_address_memory_access, (uint8_t)1) != 1)
  {
    return (MEMORYRESULT::GENERIC_ERROR);
  }
  data[0] = Wire.read();
  return (MEMORYRESULT::OK);
}

/**
 * @brief Reads a single byte from the specified address in the EEPROM.
 *
 * This function performs a random read operation from the EEPROM memory.
 *
 * @param address The memory address to read from.
 * @param data Pointer to a variable where the read byte will be stored.
 * @return MEMORYRESULT::OK if the read operation is successful.
 *         MEMORYRESULT::ADDRESS_EXCEEDS_LIMIT if the address is beyond the maximum allowed memory address.
 *         MEMORYRESULT::BUFFER_TOO_LARGE if the buffer size exceeds the maximum allowed size.
 */
MEMORYRESULT Mem24CSM01::read(uint32_t address, uint8_t *data)
{ // Random read {
  return (read(address, data, 1));
}

/**
 * @brief Reads data from the memory at the specified address into the provided buffer.
 *
 * @param address The memory address to read from.
 * @param buffer Pointer to the buffer where the read data will be stored.
 * @param bufferSize The size of the buffer and the number of bytes to read.
 * @return MEMORYRESULT::OK if the read operation is successful.
 *         MEMORYRESULT::ADDRESS_EXCEEDS_LIMIT if the address is beyond the maximum allowed memory address.
 *         MEMORYRESULT::BUFFER_TOO_LARGE if the buffer size exceeds the maximum allowed size.
 */
MEMORYRESULT Mem24CSM01::read(uint32_t address, uint8_t *buffer, size_t bufferSize)
{
  if (address > MAX_MEMORY_ADDRESS_VALUE)
  {
    return (MEMORYRESULT::ADDRESS_EXCEEDS_LIMIT);
  }
  if (bufferSize > 256)
  {
    return (MEMORYRESULT::BUFFER_TOO_LARGE);
  }
  uint8_t deviceAddress = addressMemoryPointer(address);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, bufferSize);
  size_t i = 0;
  while (Wire.available())
  {
    buffer[i] = Wire.read();
    ++i;
  }
  return (MEMORYRESULT::OK);
}

/**
 * @brief Configures a WriteAddressPacket with the given address.
 *
 * This function takes a 24-bit address and splits it into three parts:
 * - The device memory address, which includes the 17th bit of the address.
 * - The most significant byte (MSB) of the address.
 * - The least significant byte (LSB) of the address.
 *
 * @param address The 24-bit address to be configured into the WriteAddressPacket.
 * @return WriteAddressPacket The configured WriteAddressPacket containing the device memory address, MSB, and LSB.
 */
WriteAddressPacket Mem24CSM01::configureAddressPacket(uint32_t address)
{
  WriteAddressPacket writeAddressPacket;
  uint8_t highAddrBit = (address >> 17) & 1;                                                 // Extract the 17th bit of the address
  writeAddressPacket.deviceMemoryAddress = m_dev_address_memory_access | (highAddrBit << 1); // Set the device address in the right position
  writeAddressPacket.memoryMSB = (address >> 8) & 0xFF;                                      // Extract the most significant byte of the address
  writeAddressPacket.memoryLSB = address & 0xFF;                                             // Extract the least significant byte of the address
  return (writeAddressPacket);
}

/**
 * @brief Processes the result of a transmission and returns the corresponding MEMORYRESULT.
 *
 * This function takes an integer representing the result of a transmission and maps it to a MEMORYRESULT
 * enumeration value. The mapping is as follows:
 * - 0: MEMORYRESULT::OK
 * - 2: MEMORYRESULT::ADDRESS_ERROR
 * - 3: MEMORYRESULT::DATA_ERROR
 * - 5: MEMORYRESULT::TIMEOUT
 * - Any other value: MEMORYRESULT::GENERIC_ERROR
 *
 * @param transmissionResult The result of the transmission as an integer.
 * @return MEMORYRESULT The corresponding MEMORYRESULT enumeration value.
 */
MEMORYRESULT Mem24CSM01::processTransmissionResult(int transmissionResult)
{
  switch (transmissionResult)
  {
  case 0:
    return (MEMORYRESULT::OK);
    break;
  case 2:
    return (MEMORYRESULT::ADDRESS_ERROR);
    break;
  case 3:
    return (MEMORYRESULT::DATA_ERROR);
    break;
  case 5:
    return (MEMORYRESULT::TIMEOUT);
    break;
  default:
    return (MEMORYRESULT::GENERIC_ERROR);
    break;
  }
}

uint8_t Mem24CSM01::addressMemoryPointer(uint32_t address)
{
  WriteAddressPacket writeAddressPacket = configureAddressPacket(address);
  Wire.beginTransmission(writeAddressPacket.deviceMemoryAddress);
  Wire.write(writeAddressPacket.memoryMSB);
  Wire.write(writeAddressPacket.memoryLSB);
  return (writeAddressPacket.deviceMemoryAddress);
}