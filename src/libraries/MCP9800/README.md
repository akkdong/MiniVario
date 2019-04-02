# Arduino MCP9800 Temperature Sensor Library 1.1.0
https://github.com/JChristensen/MCP9800  
README file  
Jack Christensen Mar 2014

![CC BY-SA](http://mirrors.creativecommons.org/presskit/buttons/80x15/png/by-sa.png)

## Introduction
**Arduino Library for Microchip MCP9800/1/2/3 2-Wire High-Accuracy Temperature Sensors**

A lightweight implementation that exposes all functionality of the [Microchip MCP9800/1/2/3 sensors](http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en020949). Temperatures are dealt with in the integer domain to avoid the code size and runtime overhead associated with floating-point. Still, it is straightforward to perform the necessary conversions should the user wish to work in floating-point format.

Temperatures read from the device's registers are returned as °C\*16. (If the device resolution is set to less than 12 bits, the corresponding lower order bits are simply returned as zero.) Temperatures can alternately be read as °F\*10.

When writing the Hysteresis and Limit-Set registers, the value must be given as °C*2, which corresponds to the internal representation in these registers.

Bit masks for the control register are provided in the **MCP9800.h** file.     

"Arduino Library for Microchip MCP9800/1/2/3" by Jack Christensen is licensed under [CC BY-SA 4.0](http://creativecommons.org/licenses/by-sa/4.0/).


## Examples
The following example sketch is included with the **MCP9800** library:

- **MCP9800_EX1:** Demonstrates basic reading of ambient temperature in Celsius and Fahrenheit, conversion to floating-point, changing device options via the Cofiguration register, and changing the Limit-Set and Hysteresis registers.  

## Enumeration

### MCP9800_REGS_t
##### Description
Symbolic names for the MCP9800 registers.
##### Values
- AMBIENT -- for the Ambient Temperature Register  
- HYSTERESIS -- for the Temperature Hysteresis Register  
- LIMITSET -- for the Temperature Limit-Set Register

## Constructor

### MCP9800(uint8_t LS_ADDR_BITS)
##### Description
Instantiates an MCP9800 sensor object.
##### Syntax
```c++
MCP9800 mySensor(0);
// or
MCP9800 mySensor;
```
##### Parameters
**LS_ADDR_BITS:** An unsigned 8-bit integer *(uint8_t or byte)* representing the least-significant three bits of the I2C device address (i.e. a value between 0 and 7). Defaults to zero if omitted. (Note that some devices in the MCP9800 family allow the user to select the least significant three bits of the address, and some have a fixed address from the factory. See the datasheet for details.)

## Methods
### begin()
##### Description
Initializes the library. Call this method once in the setup code.
##### Syntax
`mySensor.begin();`
##### Parameters
None.
##### Returns
None.
##### Example
```c++
MCP9800 mySensor;
mySensor.begin();
```
### readTempC16(MCP9800_REGS_t reg)
##### Description
Reads one of the three temperature registers from the sensor.
##### Syntax
`mySensor.readTempC16(MCP9800_REGS_t reg);`
##### Parameters
**reg:** The temperature register to read, AMBIENT, HYSTERESIS or LIMITSET *(MCP9800_REGS_t)*
##### Returns
Temperature as degrees Celsius times 16 *(int)*.
##### Example
```c++
MCP9800 mySensor;
mySensor.begin();
int c16 = mySensor.readTempC16(AMBIENT);
float C = c16 / 16.0;
```
### readTempF10(MCP9800_REGS_t reg)
##### Description
Reads one of the three temperature registers from the sensor.
##### Syntax
`mySensor.readTempF10(MCP9800_REGS_t reg);`
##### Parameters
**reg:** The temperature register to read, AMBIENT, HYSTERESIS or LIMITSET *(MCP9800_REGS_t)*
##### Returns
Temperature as degrees Fahrenheit times 10 *(int)*.
##### Example
```c++
MCP9800 mySensor;
mySensor.begin();
int f10 = mySensor.readTempF10(AMBIENT);
float F = f10 / 10.0;
```
### writeTempC2(MCP9800_REGS_t reg, int value)
##### Description
Sets the Temperature Hysteresis Register or the Temperature Limit-Set Register.
##### Syntax
`mySensor.writeTempC2(MCP9800_REGS_t reg, int value);`
##### Parameters
**reg:** The temperature register to write, HYSTERESIS or LIMITSET *(MCP9800_REGS_t)*. If given the AMBIENT temperature register, no action is taken (AMBIENT is a read-only register).  
**value:** The temperature value to set, in degrees Celsius times two *(int)*
##### Returns
None.
##### Example
```c++
MCP9800 mySensor;
mySensor.begin();
mySensor.writeTempC2(HYSTERESIS, 25 * 2);	//set hysteresis register to 25°C
mySensor.writeTempC2(LIMITSET, 100 * 2);	//set limit-set register to 100°C
```
### readConfig()
##### Description
Reads the value of the configuration register.
##### Syntax
`mySensor.readConfig();`
##### Parameters
None.
##### Returns
Configuration register value *(byte)*
##### Example
```c++
MCP9800 mySensor;
mySensor.begin();
byte config = mySensor.readConfig();
```
### writeConfig(uint8_t value)
##### Description
Writes a value to the configuration register.
##### Syntax
`mySensor.writeConfig(byte value);`
##### Parameters
**value:** The value to be written to the register. See the MCP9800.h file for symbolic definitions for the bits in the configuration register.
##### Returns
None.
##### Example
```c++
MCP9800 mySensor;
mySensor.begin();
//set ADC resolution to 12 bits and the fault queue length to 6
byte config = ADC_RES_12BITS | FAULT_QUEUE_6;
mySensor.writeConfig(config);
```
