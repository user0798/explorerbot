/*
             Bluetooth Robot
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
        www.fourwalledcubicle.com
*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "BMA150.h"

void BMA150_Init(SensorData_t* const AccelSensorInfo)
{
	uint8_t RegisterAddress;
	uint8_t PacketBuffer[1];

	/* Sensor considered not connected until it has been sucessfully initialized */
	AccelSensorInfo->Connected = false;

	/* Attempt to read the sensor's ID register, return error if sensor cannot be communicated with */
	RegisterAddress = BMA150_CHIP_ID_REG;
	if (TWI_ReadPacket(BMA150_ADDRESS, 100, &RegisterAddress, sizeof(uint8_t), PacketBuffer, 1) != TWI_ERROR_NoError)
	  return;

	/* Verify the returned sensor ID against the expected sensor ID */
	AccelSensorInfo->Connected = (PacketBuffer[0] == BMA150_CHIP_ID);
	
	/* Abort if sensor not connected and initialized */
	if (!(AccelSensorInfo->Connected))
	  return;

	/* Force reset of the sensor */
	RegisterAddress = BMA150_SMB150_CTRL_REG;
	PacketBuffer[0] = 0x01;
	if (TWI_WritePacket(BMA150_ADDRESS, 100, &RegisterAddress, sizeof(uint8_t), PacketBuffer, 1) != TWI_ERROR_NoError)
	  return;

	/* Enable automatic wake-up of the sensor, enable interrupt line on end of conversion */
	RegisterAddress = BMA150_SMB150_CONF2_REG;
	PacketBuffer[0] = 0x21;
	if (TWI_WritePacket(BMA150_ADDRESS, 100, &RegisterAddress, sizeof(uint8_t), PacketBuffer, 1) != TWI_ERROR_NoError)
	  return;
	  
	/* Set 25MHz bandwidth, 2g range */
	RegisterAddress = BMA150_RANGE_BWIDTH_REG;
	PacketBuffer[0] = 0;
	if (TWI_WritePacket(BMA150_ADDRESS, 100, &RegisterAddress, sizeof(uint8_t), PacketBuffer, 1) != TWI_ERROR_NoError)
	  return;
}

void BMA150_Update(SensorData_t* const AccelSensorInfo)
{	
	uint8_t PacketBuffer[6];
	uint8_t RegisterAddress;

	/* Abort if sensor not connected and initialized */
	if (!(AccelSensorInfo->Connected))
	  return;

	/* Wait for sensor interrupt line to go high to signal end of conversion */
	while (!(PINB & (1 << 2)));

	/* Read the converted sensor data as a block packet */
	RegisterAddress = BMA150_X_AXIS_LSB_REG;
	if (TWI_ReadPacket(BMA150_ADDRESS, 100, &RegisterAddress, sizeof(uint8_t), PacketBuffer, 6) != TWI_ERROR_NoError)
	  return;		  

	/* Save updated sensor data */
	AccelSensorInfo->Data.Triplicate.X = (((uint16_t)PacketBuffer[1] << 8) | PacketBuffer[0]);
	AccelSensorInfo->Data.Triplicate.Y = (((uint16_t)PacketBuffer[3] << 8) | PacketBuffer[2]);
	AccelSensorInfo->Data.Triplicate.Z = (((uint16_t)PacketBuffer[5] << 8) | PacketBuffer[4]);
}
