/*-----------------------------------------------------------------------------*
 * Arduino Library for Microchip MCP9800/1/2/3 2-Wire High-Accuracy            *
 * Temperature Sensors                                                         *
 *                                                                             *
 * A lightweight implementation that exposes all functionality of the MCP9800  *
 * sensors. Temperatures are dealt with in the integer domain to avoid         *
 * the code size and runtime overhead associated with floating-point. Still,   *
 * it is easy enough to perform the necessary conversions should the user      *
 * wish to work in floating-point format.                                      *
 *                                                                             *
 * Temperatures read from the device's registers are returned as °C*16.        *
 * (If the device resolution is set to less than 12 bits, the corresponding    *
 * lower order bits are simply returned as zero.) Temperatures can             *
 * alternately be read as °F*10.                                               *
 *                                                                             *
 * When writing the Hysteresis and Limit-Set registers, the value must be      *
 * given as °C*2, which corresponds to the internal representation in these    *
 * registers.                                                                  *
 *                                                                             *
 * Bit masks for the control register are provided in the MCP9800.h file.      *
 *                                                                             *
 * Jack Christensen 21Mar2014 v1.0                                             *
 *                                                                             *
 * "Arduino library for Microchip MCP9800/1/2/3" by Jack Christensen           *
 * is licensed under CC BY-SA 4.0,                                             *
 * http://creativecommons.org/licenses/by-sa/4.0/                              *
 *-----------------------------------------------------------------------------*/

#include <MCP9800.h>

// instantiate a temperature sensor object given the least three significant
// bits (A2:0) of its I2C address (0-7)
MCP9800::MCP9800(uint8_t LS_ADDR_BITS)
{
    m_devAddr = MCP9800_BASE_ADDR + (LS_ADDR_BITS & 7);
}

void MCP9800::begin()
{
    Wire.begin();
}

// read one of the sensor's three temperature registers.
// returns the temperature as an integer which is °C times 16.
int MCP9800::readTempC16(MCP9800_REGS_t reg)
{
    Wire.beginTransmission(m_devAddr);
    Wire.write(reg);
    Wire.endTransmission();
    
    Wire.requestFrom(m_devAddr, (uint8_t)2);
    int8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    return (msb << 4) + (lsb >> 4);
}

// read one of the sensor's three temperature registers.
// returns the temperature as an integer which is °F times 10.
int MCP9800::readTempF10(MCP9800_REGS_t reg)
{
    long tF160 = (long)readTempC16(reg) * 18L;
    int tF10 = tF160 / 16;
    if ( (tF160 & 15) >= 8) ++tF10;    // round up to the next tenth if needed
    tF10 += 320;                       // add in the offset (*10)
    return tF10;
}

void MCP9800::writeTempC2(MCP9800_REGS_t reg, int value)
{
    union intByte_t
    {
        int i;
        byte b[2];
    } t;
    
    if (reg > AMBIENT) {    // ambient temp reg is read-only
        t.i = value << 7;
        Wire.beginTransmission(m_devAddr);
        Wire.write(reg);
        Wire.write(t.b[1]);
        Wire.write(t.b[0]);
        Wire.endTransmission();
    }
}

// read the sensor's configuration register
uint8_t MCP9800::readConfig()
{
    Wire.beginTransmission(m_devAddr);
    Wire.write(CONFIG_REG);
    Wire.endTransmission();
    
    Wire.requestFrom(m_devAddr, (uint8_t)1);
    return Wire.read();
}

// write the sensor's configuration register
void MCP9800::writeConfig(uint8_t value)
{
    Wire.beginTransmission(m_devAddr);
    Wire.write(CONFIG_REG);
    Wire.write(value);
    Wire.endTransmission();
}
