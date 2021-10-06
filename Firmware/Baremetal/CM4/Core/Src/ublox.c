/*
 * ublox.c
 *
 *  Created on: 16 Aug 2021
 *      Author: Daniel Babatunde
 */

#include "ublox.h"
#include "usart.h"

uint8_t data_byte[1] = {0};
const int data_size = sizeof(NAV_PVT);

// void calcChecksum(GPSHandle* gps, unsigned char* ck)
// {
//     memset(ck, 0, 2);
//     for (int i = 0; i < (int)sizeof(gps->pvt); i++)
//      {
//        ck[0] = ck[0] + ((unsigned char*)(&gps->pvt))[i];
//        ck[1] = ck[1] + ck[0];
//      }
//
// }

 void calcChecksum(unsigned char* ck)
 {
     memset(ck, 0, 2);
     for (int i = 0; i < (int)sizeof(pvt); i++)
      {
        ck[0] = ck[0] + ((unsigned char*)(&pvt))[i];
        ck[1] = ck[1] + ck[0];
      }

 }


 void processGPS(GPSHandle* gps)
 {
     gps->latitude = pvt.latitude * 1.0e-7;
     gps->longitude = pvt.longitude * 1.0e-7;
     gps->altitude = pvt.hMSL / 1000.0f;  // use Mean sea level instead

     // format to ENU
     gps->vel_x = pvt.velE / 1000.0f;
     gps->vel_y = pvt.velN / 1000.0f;
     gps->vel_z = pvt.velD / -1000.0f;
     gps->gndSpeed = pvt.gSpeed / 1000.0f;
     gps->num_satellites = pvt.numSV;


 }



// GPSHandle processGPS(GPSHandle* gps)
// {
//     //GPSHandle gpsStruct;
//
//     gps->latitude =  gps->pvt.latitude * 1.0e-7;
//     gps->longitude = gps->pvt.longitude * 1.0e-7;
//     gps->altitude =  gps->pvt.height / 1000.0f;
//
//     // format to ENU
//     gps->vel_x = gps->pvt.velE / 1000.0f;
//     gps->vel_y = gps->pvt.velN / 1000.0f;
//     gps->vel_z = gps->pvt.velD / -1000.0f;
//     gps->gndSpeed = gps->pvt.gSpeed / 1000.0f;
//
//     return *gps;
// }


// GPSHandle processGPS(GPSHandle* gps)
// {
//     //GPSHandle gpsStruct;
//
//     gps->latitude =  gps->pvt.latitude * 1.0e-7;
//     gps->longitude = gps->pvt.longitude * 1.0e-7;
//     gps->altitude =  gps->pvt.height / 1000.0f;
//
//     // format to ENU
//     gps->vel_x = gps->pvt.velE / 1000.0f;
//     gps->vel_y = gps->pvt.velN / 1000.0f;
//     gps->vel_z = gps->pvt.velD / -1000.0f;
//     gps->gndSpeed = gps->pvt.gSpeed / 1000.0f;
//
//     return *gps;
// }


 void ConfigGPS(void)
 {
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_10HZ_RATE, sizeof(UBLOX_10HZ_RATE));
   HAL_Delay(50);
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_GXGGA_OFF, sizeof(UBLOX_GXGGA_OFF));
   HAL_Delay(50);
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_GXGLL_OFF, sizeof(UBLOX_GXGLL_OFF));
   HAL_Delay(50);
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_GXGSA_OFF, sizeof(UBLOX_GXGSA_OFF));
   HAL_Delay(50);
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_GXGSV_OFF, sizeof(UBLOX_GXGSV_OFF));
   HAL_Delay(50);
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_GXRMC_OFF, sizeof(UBLOX_GXRMC_OFF));
   HAL_Delay(50);
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_GXVTG_OFF, sizeof(UBLOX_GXVTG_OFF));
   HAL_Delay(50);
   HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UBLOX_NAVPVT_ON, sizeof(UBLOX_NAVPVT_ON));

   HAL_UART_Receive_DMA(&huart3, (uint8_t *)data_byte, 1);
 }

 void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
 {
    if(huart->Instance == USART3)
    {

    }
 }


 void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
 {
    static int byte_pos = 0;
    static unsigned char checksum[2];

    if(huart->Instance == USART3)
    {
      if(byte_pos < 2)
      {
        if(data_byte[0] == UBX_HEADER[byte_pos])
        {
          byte_pos++;
        }
        else
        {
          byte_pos = 0;
        }
      }

      else
      {
        if((byte_pos -2) < data_size)
        {
          ((unsigned char*)(&pvt))[byte_pos-2] = data_byte[0];
        }
        byte_pos++;

        if (byte_pos == (data_size+2))
        {
          calcChecksum(checksum);
        }

        else if (byte_pos == (data_size+3) )
        {
          if (data_byte[0] != checksum[0])
          {
            byte_pos = 0;
          }
        }
        else if (byte_pos == (data_size+4) )
        {
          byte_pos = 0;
          //if( data_byte[0] == checksum[1] )
            //LEDB_ON();
        }
        else if ( byte_pos > (data_size+4) )
        {
          byte_pos = 0;
        }
      }
    }

    //processGPS(&gps);
 }

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//    static int byte_pos = 0;
//    static unsigned char checksum[2];
//    GPSHandle gps;
//
//    if(huart->Instance == USART3)
//    {
//      if(byte_pos < 2)
//      {
//        if(data_byte[0] == UBX_HEADER[byte_pos])
//        {
//          byte_pos++;
//        }
//        else
//        {
//          byte_pos = 0;
//        }
//      }
//
//      else
//      {
//        if((byte_pos -2) < data_size)
//        {
//          ((unsigned char*)(&gps.pvt))[byte_pos-2] = data_byte[0];
//        }
//        byte_pos++;
//
//        if (byte_pos == (data_size+2))
//        {
//          calcChecksum(&gps, checksum);
//        }
//
//        else if (byte_pos == (data_size+3) )
//        {
//          if (data_byte[0] != checksum[0])
//          {
//            byte_pos = 0;
//          }
//        }
//        else if (byte_pos == (data_size+4) )
//        {
//          byte_pos = 0;
//          //if( data_byte[0] == checksum[1] )
//            //LEDB_ON();
//        }
//        else if ( byte_pos > (data_size+4) )
//        {
//          byte_pos = 0;
//        }
//      }
//    }
//
//    //processGPS(&gps);
// }
