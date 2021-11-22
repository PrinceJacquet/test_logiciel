//Author: LabJack
//Dec. 19, 2006
//Header for UE9 example helper functions.  Function descriptions are in ue9.c.


#ifndef _UE9_H
#define _UE9_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#ifdef WIN32
#include <winsock.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;

//structure for storing calibration constants
struct UE9_CALIBRATION_INFORMATION{
  double unipolarSlope[4];
  double unipolarOffset[4];
  double bipolarSlope;
  double bipolarOffset;
  double DACSlope[2];
  double DACOffset[2];
  double tempSlope;
  double tempSlopeLow;
  double calTemp;
  double Vref;
  double VrefDiv2;
  double VsSlope;
  double hiResUnipolarSlope;
  double hiResUnipolarOffset;
  double hiResBipolarSlope;
  double hiResBipolarOffset;
};

typedef struct UE9_CALIBRATION_INFORMATION ue9CalibrationInfo;






//functions

void normalChecksum(uint8 *b, int n);
void extendedChecksum(uint8 *b, int n);
uint8 normalChecksum8(uint8 *b, int n);
uint16 extendedChecksum16(uint8 *b, int n);
uint8 extendedChecksum8(uint8 *b);
int openTCPConnection(char *server, int port);
int closeTCPConnection(int fd);
long getTickCount();
long binaryToCalibratedAnalogVoltage(ue9CalibrationInfo *caliInfo, uint8 gainBip, uint8 resolution, uint16 bytesVoltage, double *analogVoltage);
long binaryToCalibratedAnalogTemperature(ue9CalibrationInfo *caliInfo, int powerLevel, uint16 bytesTemperature, double *analogTemperature);
long analogToCalibratedBinaryVoltage(ue9CalibrationInfo *caliInfo, int DACNumber, double analogVoltage, uint16 *bytesVoltage);
long binaryToUncalibratedAnalogVoltage(uint8 gainBip, uint16 bytesVoltage, double *analogVoltage);
long analogToUncalibratedBinaryVoltage(double analogVoltage, uint16 *bytesVoltage);
double FPuint8ArrayToFPDouble(uint8 *buffer, int startIndex);
long getCalibrationInfo(int fd, ue9CalibrationInfo *caliInfo);
  
#endif
