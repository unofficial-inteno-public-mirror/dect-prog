/****************************************************************************
*  Program/file: PreLoader.h
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
$Date:   07 Mar 2014 10:14:20  $
$Revision:   1.0  $
$Modtime:   07 Mar 2014 08:26:26  $
$Archive:   J:/sw/Tools/FlashLoader/FL7/Projects/Tg/SC14441/Preloader/vcs/Preloader.h_v  $

*/

#ifndef PRELOADER_H
#define PRELOADER_H

/****************************************************************************
*                               Include files                                 
****************************************************************************/


/****************************************************************************
*                              Macro definitions                             
****************************************************************************/
//--------------------------------------------------------------------------
//     PC                            TARGET
//     ==                            ======
//             PRELOADER_START
//    --------------------------------->
//             PRELOADER_READY
//    <---------------------------------
//           PRELOADER_BAUD_xxxx
//    --------------------------------->
//         PRELOADER_NEW_BAUDRATE
//    --------------------------------->
//       PRELOADER_NEW_BAUDRATE_READY
//    <---------------------------------
//             msb code length
//    --------------------------------->
//            lsb code length
//    --------------------------------->
//                 code
//    --------------------------------->
//                 ....
// 			       ....
//                 code
//    --------------------------------->
//                 chk
//    <---------------------------------
//
//     Boot loader down loaded now
//
//--------------------------------------------------------------------------

#define PRELOADER_START              1
#define PRELOADER_READY              2
#define PRELOADER_NEW_BAUDRATE       3
#define PRELOADER_NEW_BAUDRATE_READY 4

#define PRELOADER_BAUD_9600          5
#define PRELOADER_BAUD_19200         6
#define PRELOADER_BAUD_57600         7
#define PRELOADER_BAUD_115200        8
#define PRELOADER_BAUD_230400        9
#define PRELOADER_BAUD_460800       10
#define PRELOADER_BAUD_921600       11
#define PRELOADER_BAUD_1843200      12



/****************************************************************************
*                     Enumerations/Type definitions/Structs                  
****************************************************************************/


/****************************************************************************
*                           Global variables/const                           
****************************************************************************/

/****************************************************************************
*                             Function prototypes                            
****************************************************************************/

#endif

