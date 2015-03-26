/****************************************************************************
*  Program/file: MRXTDEF.H
*  
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of 
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in 
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: <USER>
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
$Date:   23 Jan 2009 10:45:06  $
$Revision:   1.1  $
$Modtime:   12 Jan 2009 15:37:14  $
$Archive:   J:/sw/Tools/FlashLoader/FL7/Include/Standard/vcs/MRtxdef.h_v  $

*/

/*! \file
* Standard types and macros.
*/


#ifndef MRTXDEF_H
#define MRTXDEF_H

/****************************************************************************
*                               Include files                                 
****************************************************************************/


/****************************************************************************
*                              Macro definitions                             
****************************************************************************/

/****************************************************************************
*                              Macro definitions                             
****************************************************************************/

/** \name Values for boolean
* Values for the \ref boolean type. */
//@{
#define TRUE  1  /*!< Boolean true. */
#define FALSE 0  /*!< Boolean false. */
//@}

/****************************************************************************
* Bit macros
****************************************************************************/

/** \name Bit positions
*/
//@{
#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80

#define BIT8      0x0100
#define BIT9      0x0200
#define BIT10     0x0400
#define BIT11     0x0800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000

#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000

#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000
//@}


/****************************************************************************
*                     Enumerations/Type definitions/Structs                  
****************************************************************************/
/** \name Enumeration Macros
* \anchor enum_macros
* For defining enumerated values for use in external interfaces.
* The following code fragment shows the use of ENUMTAG and ENUM16.
*
* \code
* typedef enum ENUMTAG(ErrorIdType)
*   E_NONE,
*   ...
*   E_MAX = 0xFFFF
* } ENUM16(ErrorIdType);
* \endcode
*
*/
//@{
#define ENUM8(EnumName)  Enum_##EnumName; typedef uint8  EnumName
#define ENUM16(EnumName) Enum_##EnumName; typedef uint16 EnumName
#define ENUM32(EnumName) Enum_##EnumName; typedef uint32 EnumName
//@}


typedef unsigned char   uint8;     /*!<  byte 0..255        */
typedef   signed char   int8;      /*!<  byte -128..127     */
typedef unsigned short  uint16;    /*!<  word 0..65535      */
typedef   signed short  int16;     /*!<  16-bit -32768..32767 */
typedef unsigned long   uint32;    /*!<  32-bit 0..4294967295 */
typedef   signed long   int32;     /*!<  32-bit -2147483648..2147483647 */
typedef unsigned char   boolean;   /*!<  0 is logical FALSE, !0 is logical TRUE */


/****************************************************************************
*                           Global variables/const                           
****************************************************************************/


/****************************************************************************
*                             Function prototypes                            
****************************************************************************/


#endif

