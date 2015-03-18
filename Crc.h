/****************************************************************************
*  Program/file: Crc.h
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
$Date:   07 Mar 2014 10:01:06  $
$Revision:   1.2  $
$Modtime:   05 Mar 2014 14:18:50  $
$Archive:   J:/sw/Tools/FlashLoader/FL7/Components/Crc/vcs/Crc.h_v  $

*/

#ifndef CRC_H
#define CRC_H

/****************************************************************************
*                               Include files                                 
****************************************************************************/


/****************************************************************************
*                              Macro definitions                             
****************************************************************************/


/****************************************************************************
*                     Enumerations/Type definitions/Structs                  
****************************************************************************/


/****************************************************************************
*                           Global variables/const                           
****************************************************************************/


/****************************************************************************
*                             Function prototypes                            
****************************************************************************/
void   InitCrc32Table(void);

uint32 InitCalculateCRC32(void);
uint32 GetCRC32(uint32 crc32);
uint32 BlockCalculateCRC32(uint32 crc32, uint16 *StartAddr, uint32 Length);

uint32 CalculateCRC32(uint16 *StartAddr, uint32 Length);
uint32 CalculateEmptyBlockCRC32(uint32 Length);
uint16 UpdateCrc(uint8 data, uint16 icrc);

#endif

