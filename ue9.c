//Author: LabJack
//Dec. 19, 2006
//
//Updates:
//
//(10/11/2005)
//  - added structure for storing calibration info
//  - added the following functions: binaryToCalibratedAnalogVoltage
//                                   binaryToCalibratedAnalogTemperature
//                                   analogToCalibratedBinaryVoltage
//                                   binaryToUncalibratedAnalogVoltage 
//                                   analogToUncalibratedBinaryVoltage
//                                   FPuint8ArrayToFPDouble
//                                   getCalibrationInfo
//  - removed the following functions: translateCalibratedVoltage 
//                                     translateUncalibratedVoltage
//                                     getAnalogCalibrationInfo
//                                     fpArrayTofpDouble (now FPuint8ArrayToFPDouble)
//
//(01/11/2006)
//  - changed header include order in ue9.h so that code compiles under MAC OS X.
//
//(12/19/2006)
//  - added calibrations for hi-res mode for the UE9 Pro
//  - added resolution parameter for binaryToCalibratedAnalogVoltage(ue9CalibrationInfo
//  - changed the name of CALIBRATION_INFORMATION struct to UE9_CALIBRATION_INFORMATION
#include "ue9.h"


/* Adds checksum to a data packet for normal command format */
void normalChecksum(uint8 *b, int n)
{
  b[0]=normalChecksum8(b,n);
}


/* Adds checksum to a data packet for extended command format */
void extendedChecksum(uint8 *b, int n)
{
  uint16 a;

  a = extendedChecksum16(b,n);
  b[4] = (uint8)(a & 0xff);
  b[5] = (uint8)((a >> 8) & 0xff);
  b[0] = extendedChecksum8(b);
}


/* Sum bytes 1 to n-1 as uint16. Sum quotient and remainder of 256 division. 
   Again, sum quotient and remainder of 256 division.  Return result as uint8. */
uint8 normalChecksum8(uint8 *b, int n)
{
  int i;
  uint16 a, bb;

  for(i = 1, a = 0; i < n; i++)
    a+=(uint16)b[i];

  bb=a/256;
  a=(a-256*bb)+bb;
  bb=a/256;

  return (uint8)((a-256*bb)+bb);
}


/* Sums bytes 6 to n-1 as uint16. */
uint16 extendedChecksum16(uint8 *b, int n)
{
  int i,a = 0;

  for(i = 6; i < n; i++)
    a += (uint16)b[i];

  return a;
}


/* Sum bytes 1 to 5. Sum quotient and remainder of 256 division. Again, sum
   quotient and remainder of 256 division. Return result as uint8. */
uint8 extendedChecksum8(uint8 *b)
{
  int i,a,bb;

  for(i=1,a=0;i<6;i++)
    a+=(uint16)b[i];

  bb=a/256;
  a=(a-256*bb)+bb;
  bb=a/256;

  return (uint8)((a-256*bb)+bb);  
}


/* Open a socket which connects to a specific port on server. Returns a value
   < 0 on failure. Returns the socket descriptor on success. */
int openTCPConnection(char *server, int port)
{
  int socketFd;
  struct sockaddr_in address;

  #ifdef WIN32
  WSADATA info;
  struct hostent *he;

  if (WSAStartup(MAKEWORD(1,1), &info) != 0)
  {
    printf("Error: Cannot initilize winsock\n");
    return 0;
  }
  #endif

  socketFd = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

  if(socketFd == -1)
  {
    fprintf(stderr,"Could not create socket. Exiting\n");
    return -1;
  }

  address.sin_family=AF_INET;
  address.sin_port=htons(port);

  #ifdef WIN32
  he = gethostbyname(server);
  address.sin_addr = *((struct in_addr *)he->h_addr);
  #else
  inet_pton(AF_INET,server,&address.sin_addr);

  int window_size = 128 * 1024;  //current window size is 128 kilobytes
  int rw = 0;
  int size = sizeof(rw);
  int err;
  err = setsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, (char*) & window_size, 
    sizeof(window_size));

  err = getsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, (char*) & rw, &size );
  #endif

  if((connect(socketFd,(struct sockaddr *)&address,sizeof(address)))<0)
  {
    fprintf(stderr,"Could not connect to %s:%d\n",inet_ntoa(address.sin_addr),
            port);
    return -2;
  }

  return socketFd;
}


int closeTCPConnection(int fd)
{
  #ifdef WIN32
  int err;

  err = closesocket(fd);
  WSACleanup();

  return err;
  #else
  return close(fd);
  #endif
}


/* Retrieves the number of milliseconds that has elasped since the system was
   started. */
long getTickCount()
{
#ifdef WIN32
  return ( (long)(GetTickCount()) );
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif


/* Translates the binary analog input bytes read from the ue9, to a voltage
   value (calibrated).  Call getCalibrationInfo first to set up caliInfo.
   Returns -1 on error, 0 on success.  */
long binaryToCalibratedAnalogVoltage(ue9CalibrationInfo *caliInfo, uint8 gainBip, uint8 resolution, uint16 bytesVoltage, double *analogVoltage)
{
  double slope;
  double offset;

  if(resolution < 18)
  {
    switch( (unsigned int)gainBip )
    {
      case 0:
        slope = caliInfo->unipolarSlope[0];
        offset = caliInfo->unipolarOffset[0];
        break;
      case 1:
        slope = caliInfo->unipolarSlope[1];
        offset = caliInfo->unipolarOffset[1];
        break;
      case 2:
        slope = caliInfo->unipolarSlope[2];
        offset = caliInfo->unipolarOffset[2];
        break;
      case 3:
        slope = caliInfo->unipolarSlope[3];
        offset = caliInfo->unipolarOffset[3];
        break;
      case 8:
        slope = caliInfo->bipolarSlope;
        offset = caliInfo->bipolarOffset;
        break;
      default:
        goto invalidGainBip;
    }
  }
  else
  {
    switch( (unsigned int)gainBip )
    {
      case 0:
        slope = caliInfo->hiResUnipolarSlope;
        offset = caliInfo->hiResUnipolarOffset;
        break;
      case 8:
        slope = caliInfo->hiResBipolarSlope;
        offset = caliInfo->hiResBipolarOffset;
        break;
      default:
        goto invalidGainBip;
    }
  }

  *analogVoltage = (slope*bytesVoltage) + offset;
  return 0;

invalidGainBip:
  printf("binaryToCalibratedAnalogVoltage error: invalid GainBip.\n");
  return -1;
}


/* Translates the binary analog bytes read from the ue9, to a Kelvin temperature
   value (calibrated).  Call getCalibrationInfo first to set up caliInfo.
   Returns -1 on error, 0 on success. */
long binaryToCalibratedAnalogTemperature(ue9CalibrationInfo *caliInfo, int powerLevel, uint16 bytesTemperature, double *analogTemperature)
{
  double slope = 0;

  switch( (unsigned int)powerLevel )
  {
    case 0:     //high power 
      slope = caliInfo->tempSlope;
      break;
    case 1:     //low power
      slope = caliInfo->tempSlopeLow;
      break;
    default:
      printf("binaryToCalibratedAnalogTemperatureK error: invalid powerLevel.\n");
      return -1;
  }

  *analogTemperature = (double)(bytesTemperature)*slope;
  return 0;
}







/* Translates a voltage value to binary analog input bytes (calibrated) that can be 
   sent to a ue9.  Call getCalibrationInfo first to set up caliInfo.
   Returns -1 on error, 0 on success. */
long analogToCalibratedBinaryVoltage(ue9CalibrationInfo *caliInfo, int channelNumber, double analogVoltage, uint16 *bytesVoltage)
{
  double slope;
  double offset;
  double tempBytesVoltage;

  switch(channelNumber) 
  {
    case 0:
      slope = caliInfo->DACSlope[0];
      offset = caliInfo->DACOffset[0];
      break;
    case 1:
      slope = caliInfo->DACSlope[1];
      offset = caliInfo->DACOffset[1];
      break;
    default:
      printf("analogToCalibratedBinaryVoltage error: invalid DACNumber.\n");
      return -1;
  }

  tempBytesVoltage = slope*analogVoltage + offset;

  //Checking to make sure bytesVoltage will be a value between 0 and 4095, 
  //or that a uint16 overflow does not occur.  A too high analogVoltage 
  //(above 5 volts) or too low analogVoltage (below 0 volts) will cause a 
  //value not between 0 and 4095.
  if(tempBytesVoltage < 0)
    tempBytesVoltage = 0;
  if(tempBytesVoltage > 4095)
    tempBytesVoltage = 4095;

  *bytesVoltage = (uint16)tempBytesVoltage; 

  return 0;
}


/* Translates the binary analog input bytes read from the ue9, to a voltage
   value (uncalibrated)  */
long binaryToUncalibratedAnalogVoltage(uint8 gainBip, uint16 bytesVoltage, double *analogVoltage)
{
  switch( (unsigned int)gainBip )
  {
    case 0: 
      *analogVoltage = ((double)bytesVoltage/65536.0)*5.08;
      break;
    case 1: 
      *analogVoltage = ((double)bytesVoltage/65536.0)*2.54;
      break;
    case 2: 
      *analogVoltage = ((double)bytesVoltage/65536.0)*1.27;
      break;
    case 3: 
      *analogVoltage = ((double)bytesVoltage/65536.0)*0.63;
      break;
    case 8: 
      *analogVoltage = ((double)bytesVoltage/65536.0)*10.25;
      break;
    default:  
      printf("binaryToUncalibratedAnalogVoltage error: invalid GainBip.\n");
      return -1;
  }
  return 0;
}


/* Translates a voltage value to binary analog input bytes (uncalibrated) that can be
   sent to a ue9.  Returns -1 on error, 0 on success.  */
long analogToUncalibratedBinaryVoltage(double analogVoltage, uint16 *bytesVoltage)
{
  double tempBytesVoltage;

  tempBytesVoltage = (analogVoltage/4.86)*4096;

  //Checking to make sure bytesVoltage will be a value between 0 and 4095,
  //or that a uint16 overflow does not occur.  A too high analogVoltage 
  //(above 5 volts) or too low analogVoltage (below 0 volts) will cause a
  //value not between 0 and 4095.
  if(tempBytesVoltage < 0)
    tempBytesVoltage = 0;
  if(tempBytesVoltage > 4095)
    tempBytesVoltage = 4095;

  *bytesVoltage = (uint16)tempBytesVoltage;

  return 0;
}


/* Converts a fixed point byte array (starting a startIndex) to a floating point
   double value. */
double FPuint8ArrayToFPDouble(uint8 *buffer, int startIndex)
{
  unsigned int resultDec;
  int resultWh;
  int i;

  resultDec = 0;
  resultWh = 0;

  for(i = 0; i < 4; i++) 
  {
    resultDec += (buffer[startIndex + i] << (i*8));
    resultWh += (buffer[startIndex + i + 4] << (i*8));
  }

  return ( (double)resultWh + (double)(resultDec)/4294967296.0 );
}


/* Gets calibration information from memory blocks 0-2 of a UE9 and stores the
   information into a ue9CalibrationInfo structure. */
long getCalibrationInfo(int fd, ue9CalibrationInfo *caliInfo)
{
  uint8 sendBuffer[8];
  uint8 recBuffer[136];
  int sentRec = 0;

  /* reading block 0 from memory */
  sendBuffer[1] = (uint8)(0xF8);  //command byte
  sendBuffer[2] = (uint8)(0x01);  //number of data words
  sendBuffer[3] = (uint8)(0x2A);  //extended command number
  sendBuffer[6] = (uint8)(0x00);
  sendBuffer[7] = (uint8)(0x00);  //Blocknum = 0
  extendedChecksum(sendBuffer, 8);

  sentRec = send(fd, sendBuffer, 8, 0);

  if(sentRec < 8)
  {
    if(sentRec == -1)
      goto sendError0;
    else  
      goto sendError1;
  }

  sentRec= recv(fd, recBuffer, 136, 0);

  if(sentRec < 136)
  {
    if(sentRec == -1)
      goto recvError0;
    else  
      goto recvError1;
  }

  if(recBuffer[1] != (uint8)(0xF8) || recBuffer[2] != (uint8)(0x41) || recBuffer[3] != (uint8)(0x2A))
    goto commandByteError;

  //block data starts on byte 8 of the buffer
  caliInfo->unipolarSlope[0] = FPuint8ArrayToFPDouble(recBuffer + 8, 0);
  caliInfo->unipolarOffset[0] = FPuint8ArrayToFPDouble(recBuffer + 8, 8);
  caliInfo->unipolarSlope[1] = FPuint8ArrayToFPDouble(recBuffer + 8, 16);
  caliInfo->unipolarOffset[1] = FPuint8ArrayToFPDouble(recBuffer + 8, 24);
  caliInfo->unipolarSlope[2] = FPuint8ArrayToFPDouble(recBuffer + 8, 32);
  caliInfo->unipolarOffset[2] = FPuint8ArrayToFPDouble(recBuffer + 8, 40);
  caliInfo->unipolarSlope[3] = FPuint8ArrayToFPDouble(recBuffer + 8, 48);
  caliInfo->unipolarOffset[3] = FPuint8ArrayToFPDouble(recBuffer + 8, 56);

  /* reading block 1 from memory */
  sendBuffer[7] = (uint8)(0x01);    //Blocknum = 1
  extendedChecksum(sendBuffer, 8);

  sentRec = send(fd, sendBuffer, 8, 0);

  if(sentRec < 8)
  {
    if(sentRec == -1)
      goto sendError0;
    else  
      goto sendError1;
  }

  sentRec= recv(fd, recBuffer, 136, 0);

  if(sentRec < 136)
  {
    if(sentRec == -1)
      goto recvError0;
    else  
      goto recvError1;
  }

  if(recBuffer[1] != (uint8)(0xF8) || recBuffer[2] != (uint8)(0x41) || recBuffer[3] != (uint8)(0x2A))
    goto commandByteError;

  //block data starts on byte 8 of the buffer
  caliInfo->bipolarSlope = FPuint8ArrayToFPDouble(recBuffer + 8, 0);
  caliInfo->bipolarOffset = FPuint8ArrayToFPDouble(recBuffer + 8, 8);

  /* reading block 2 from memory */
  sendBuffer[7] = (uint8)(0x02);    //Blocknum = 2
  extendedChecksum(sendBuffer, 8);

  sentRec = send(fd, sendBuffer, 8, 0);

  if(sentRec < 8)
  {
    if(sentRec == -1)
      goto sendError0;
    else  
      goto sendError1;
  }

  sentRec= recv(fd, recBuffer, 136, 0);

  if(sentRec < 136)
  {
    if(sentRec == -1)
      goto recvError0;
    else  
      goto recvError1;
  }

  if(recBuffer[1] != (uint8)(0xF8) || recBuffer[2] != (uint8)(0x41) || recBuffer[3] != (uint8)(0x2A))
    goto commandByteError;

  //block data starts on byte 8 of the buffer
  caliInfo->DACSlope[0] = FPuint8ArrayToFPDouble(recBuffer + 8, 0);
  caliInfo->DACOffset[0] = FPuint8ArrayToFPDouble(recBuffer + 8, 8);
  caliInfo->DACSlope[1] = FPuint8ArrayToFPDouble(recBuffer + 8, 16);
  caliInfo->DACOffset[1] = FPuint8ArrayToFPDouble(recBuffer + 8, 24);
  caliInfo->tempSlope = FPuint8ArrayToFPDouble(recBuffer + 8, 32);
  caliInfo->tempSlopeLow = FPuint8ArrayToFPDouble(recBuffer + 8, 48);
  caliInfo->calTemp = FPuint8ArrayToFPDouble(recBuffer + 8, 64);
  caliInfo->Vref = FPuint8ArrayToFPDouble(recBuffer + 8, 72);
  caliInfo->VrefDiv2 = FPuint8ArrayToFPDouble(recBuffer + 8, 88);
  caliInfo->VsSlope = FPuint8ArrayToFPDouble(recBuffer + 8, 96);


  /* reading block 3 from memory */
  sendBuffer[7] = (uint8)(0x03);    //Blocknum = 3
  extendedChecksum(sendBuffer, 8);

  sentRec = send(fd, sendBuffer, 8, 0);

  if(sentRec < 8)
  {
    if(sentRec == -1)
      goto sendError0;
    else  
      goto sendError1;
  }

  sentRec= recv(fd, recBuffer, 136, 0);

  if(sentRec < 136)
  {
    if(sentRec == -1)
      goto recvError0;
    else  
      goto recvError1;
  }

  if(recBuffer[1] != (uint8)(0xF8) || recBuffer[2] != (uint8)(0x41) || recBuffer[3] != (uint8)(0x2A))
    goto commandByteError;

  //block data starts on byte 8 of the buffer
  caliInfo->hiResUnipolarSlope = FPuint8ArrayToFPDouble(recBuffer + 8, 0);
  caliInfo->hiResUnipolarOffset = FPuint8ArrayToFPDouble(recBuffer + 8, 8);

  /* reading block 4 from memory */
  sendBuffer[7] = (uint8)(0x04);    //Blocknum = 4
  extendedChecksum(sendBuffer, 8);

  sentRec = send(fd, sendBuffer, 8, 0);

  if(sentRec < 8)
  {
    if(sentRec == -1)
      goto sendError0;
    else  
      goto sendError1;
  }

  sentRec= recv(fd, recBuffer, 136, 0);

  if(sentRec < 136)
  {
    if(sentRec == -1)
      goto recvError0;
    else  
      goto recvError1;
  }

  if(recBuffer[1] != (uint8)(0xF8) || recBuffer[2] != (uint8)(0x41) || recBuffer[3] != (uint8)(0x2A))
    goto commandByteError;

  //block data starts on byte 8 of the buffer
  caliInfo->hiResBipolarSlope = FPuint8ArrayToFPDouble(recBuffer + 8, 0);
  caliInfo->hiResBipolarOffset = FPuint8ArrayToFPDouble(recBuffer + 8, 8);

  return 0;

sendError0: 
  printf("Error : getCalibrationInfo send failed\n");
  return -1;

sendError1:
  printf("Error : getCalibrationInfo send did not send all of the buffer\n");
  return -1;

recvError0:
  printf("Error : getCalibrationInfo recv failed\n");
  return -1;

recvError1:
  printf("Error : getCalibrationInfo recv did not receive all of the buffer\n");
  return -1;

commandByteError:
  printf("Error : received buffer at byte 1, 2, or 3 are not 0xA3, 0x01, 0x2A \n");
  return -1;
}
