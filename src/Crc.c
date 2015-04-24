/****************************************************************************
*  Program/file: Crc.c
*   
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of 
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in 
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LB
*
*  MODULE:
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*   
*   
*  DESCRIPTION:
*   
*   
*   
*   
*   
****************************************************************************/

/****************************************************************************
*                                  PVCS info                                  
*****************************************************************************

$Author:   lb  $
$Date:   07 Mar 2014 10:01:32  $
$Revision:   1.4  $
$Modtime:   05 Mar 2014 14:14:20  $
$Archive:   J:/sw/Tools/FlashLoader/FL7/Components/Crc/vcs/Crc.c_v  $

*/


/****************************************************************************
*                               Include files                                 
****************************************************************************/
//#include <projcfg.h>
#include <stdio.h>
#include <stdlib.h>
#include "MRtxDef.h"
#include "MailDef.h"



/****************************************************************************
*                              Macro definitions                              
****************************************************************************/


/****************************************************************************
*                     Enumerations/Type definitions/Structs                   
****************************************************************************/


/****************************************************************************
*                            Global variables/const                           
****************************************************************************/
//extern void TrigWatchdog(void);

/****************************************************************************
*                            Local variables/const                            
****************************************************************************/


/****************************************************************************
*                          Local Function prototypes                          
****************************************************************************/
static uint32 crc32Table[256];                // 1024 bytes 


/****************************************************************************
*                                Implementation                               
****************************************************************************/

//------------------------------------------------------------------------------
// Generates Crc32 table
//------------------------------------------------------------------------------
void InitCrc32Table(void) 
{
   uint32 crc, poly;
   int i, j;

   poly = 0xEDB88320L;
   for (i=0; i<256; i++) {
      crc = i;
      for (j=8; j>0; j--) {
         if (crc&1) {
            crc = (crc >> 1) ^ poly;
         } else {
            crc >>= 1;
         }
      }
      crc32Table[i] = crc;
   }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint32 InitCalculateCRC32(void)
{
  return(0xFFFFFFFF);
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint32 GetCRC32(uint32 crc32)
{
  crc32=crc32^0xFFFFFFFF;
  return(crc32);
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint32 BlockCalculateCRC32(uint32 crc32, uint16 *StartAddr, uint32 Length)
{
  volatile uint16  *Start16ptr;

  uint8    data;
  uint16   data16;
  uint32   i;

  Start16ptr=StartAddr;

  //-----------------------------------------------------------------
  // Calculate CRC32 
  //-----------------------------------------------------------------
  i=0;
  while (i < Length) 
  {
       data16=*Start16ptr++;
       data=(uint8)data16;
       crc32 = ((crc32>>8) & 0x00FFFFFF) ^ crc32Table[ (crc32^data) & 0xFF ];
       if (++i < Length)
       {
           data=data16>>8;
           crc32 = ((crc32>>8) & 0x00FFFFFF) ^ crc32Table[ (crc32^data) & 0xFF ];
           ++i;
       }
       //TrigWatchdog();
    //-----------------------------------------------------------------
  }         
  return(crc32);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint32 CalculateCRC32(uint16 *StartAddr, uint32 Length)
{
  register uint32 crc32;
  crc32 = InitCalculateCRC32();
  crc32 = BlockCalculateCRC32(crc32, StartAddr, Length);
  crc32 = GetCRC32(crc32);
  return(crc32);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint32 CalculateEmptyBlockCRC32(uint32 Length)
{
  register uint32 crc32;
  uint32 i;
  uint8    data=0xFF;
  crc32 = InitCalculateCRC32();

  for (i=0; i<Length; ++i)
  {
	  crc32 = ((crc32>>8) & 0x00FFFFFF) ^ crc32Table[ (crc32^data) & 0xFF ];
  }
  crc32 = GetCRC32(crc32);
  return(crc32);
}


//-----------------------------------------------------------------------------
// Name       : UpdateCrc                                                      
// Input      : Data to update the crc with                                    
// Output     : The updated crc                                                
// Modifies   : crc                                                            
// ----------------------------------------------------------------------------
// Description: Update the CRC for transmitted and received data using         
//              the CCITT 16bit algorithm (X^16 + X^12 + X^5 + 1).             
//----------------------------------------------------------------------------- 
/* uint16 UpdateCrc(uint8 data, uint16 icrc) */
/* { */
/*   uint16 crc=icrc; */
/*   crc  = (uint16)((uint8)(crc >> 8) | (crc << 8)); */
/*   crc ^= (uint16)data; */
/*   crc ^= (uint16)((uint8)(crc & 0xff) >> 4); */
/*   crc ^= (uint16)((crc << 8) << 4); */
/*   crc ^= (uint16)(((crc & 0xff) << 4) << 1); */
/*   return(crc); */
/* } */

uint16 UpdateCrc(uint8 data, uint16 icrc)
{
  uint16 crc=icrc;
  crc  = (uint16)((uint8)(crc >> 8) | (crc << 8));
  crc ^= (uint16)data;
  crc ^= (uint16)((uint8)(crc & 0xff) >> 4);
  crc ^= (uint16)((crc << 8) << 4);
  crc ^= (uint16)(((crc & 0xff) << 4) << 1);
  return(crc);
}


