/****************************************************************************
*  Program/file: MailDef.h
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
$Date:   13 Sep 2010 10:34:36  $
$Revision:   1.9  $
$Modtime:   13 Sep 2010 10:33:18  $
$Archive:   J:/sw/Tools/FlashLoader/FL7/Components/Mail/vcs/MailDef.h_v  $
*/

/*! \file
* Mail header file.
*/

#ifndef MAILDEF_H
#define MAILDEF_H

/****************************************************************************
*                               Include files                                 
****************************************************************************/

/****************************************************************************
*                              Macro definitions                             
****************************************************************************/
//-----------------------------------------------------------------------------
//   
//   Packet format
//   =============
//
//  +-----+----------------- 
//  | 0   | Header
//  +-----+----------------- 
//  | 1   | Length msb          n=Length
//  +-----+----------------- 
//  | 2   | Length lsb
//  +-----+----------------- 
//  | 3   | Payload 1
//  +-----+----------------- 
//  | .   | ...
//  +-----+----------------- 
//  | .   | ... 
//  +-----+----------------- 
//  | 2+n | Payload n
//  +-----+----------------- 
//  | 3+n | crc msb
//  +-----+----------------- 
//  | 4+n | crc lsb
//  +-----+----------------- 
//   
//   
//-----------------------------------------------------------------------------

/*----------------------------------------------------------------------------- 
MAX PAYLOAD SIZE :
 
typedef struct
{
  PrimitiveType     Primitive;        1
  uint32            Address;          4
  uint16            Length;           2
  uint8             Data[1];       2048   (MAX_PAYLOAD_DATA_BYTES)
                                   -----
                                   2055 Bytes
                                   -----                                    
} Unit8DataMailType;


-------------------------------------------------------------------------------*/

/** \name Set max number of payload data bytes  
*/
//@{
#define MAX_PAYLOAD_DATA_BYTES     (2*1024) //MAX 2kByte Bytes  
//@}

/** \name Max size of mail payload   
*/
//@{
#define MAX_PAYLOAD_SIZE           (MAX_PAYLOAD_DATA_BYTES + sizeof(Unit8DataMailType)-1)
//@}

/** \name Number of Packet  overhead bytes
*/
//@{
#define PACKET_OVER_HEAD           5        // Header Length msb,lsb and crc msb,lsb
//@}

/** \name Packet Header char
*/
//@{
#define UART_PACKET_HEADER             'F'
//@}

/** \name Max size of mail buffer both in PC and target application
*/
//@{
#define MAX_MAIL_BUFFER_SIZE       (MAX_PAYLOAD_SIZE+PACKET_OVER_HEAD)
//@}
            
/****************************************************************************
*                     Enumerations/Type definitions/Structs                  
****************************************************************************/

/// Mail Primitives 
typedef enum
{
  FIRST_PRIMITIVE,            /*!< 0x00  Reserved */ 

  READ_MEM8_REQ,              /*!< 0x01  Read memory request (8 bit mode) */ 
  READ_MEM8_CFM,              /*!< 0x02  Read memory confirm (8 bit mode) */ 

  READ_MEM16_REQ,             /*!< 0x03  Read memory request (16 bit mode) */ 
  READ_MEM16_CFM,             /*!< 0x04  Read memory confirm (16 bit mode) */ 
 
  READ_MEM32_REQ,             /*!< 0x05  Read memory request (32 bit mode) */ 
  READ_MEM32_CFM,             /*!< 0x06  Read memory confirm (32 bit mode) */ 
                              
  WRITE_MEM8_REQ,             /*!< 0x07  Write memory request (8 bit mode) */ 
  WRITE_MEM8_CFM,             /*!< 0x08  Write memory confirm (8 bit mode) */ 

  WRITE_MEM16_REQ,            /*!< 0x09  Write memory request (16 bit mode) */ 
  WRITE_MEM16_CFM,            /*!< 0x0A  Write memory confirm (16 bit mode) */ 
                                   
  WRITE_MEM32_REQ,            /*!< 0x0B  Write memory request (32 bit mode) */ 
  WRITE_MEM32_CFM,            /*!< 0x0C  Write memory confirm (32 bit mode) */ 
 
  READ_SW_VERSION_REQ,        /*!< 0x0D  Read software version of application loader request */ 
  READ_SW_VERSION_CFM,        /*!< 0x0E  Read software version of application loader confirm */ 

  WRITE_CONFIG_REQ,           /*!< 0x0F  Write application loader configuration request */  
  WRITE_CONFIG_CFM,           /*!< 0x10  Write application loader configuration confirm */ 

  PROG_FLASH_REQ,             /*!< 0x11  Program flash data request */ 
  PROG_FLASH_CFM,             /*!< 0x12  Program flash data confirm */ 

  CALC_CRC32_REQ,             /*!< 0x13  Calculate Crc32 request */ 
  CALC_CRC32_CFM,             /*!< 0x14  Calculate Crc32 result and confirm */ 

  FLASH_ERASE_REQ,            /*!< 0x15  Flash erase request */ 
  FLASH_ERASE_CFM,            /*!< 0x16  Flash erase confirm */ 

  SET_BAUDRATE_REQ,           /*!< 0x17  Change baudrate request */ 
  SET_BAUDRATE_CFM,           /*!< 0x18  Change baudrate confirm */ 

  WRITE_BMC_REG_REQ,          /*!< 0x19  Write BMC Register request */ 
  WRITE_BMC_REG_CFM,          /*!< 0x1A  Write BMC Register confirm */ 

  JUMP_TO_ADDR_REQ,           /*!< 0x1B  Jump to code address  request */ 
  JUMP_TO_ADDR_CFM,           /*!< 0x1C  Jump to code address confirm */ 

  FILL_MEM16_REQ,             /*!< 0x1D  Fill memory request (16 bit mode) */ 
  FILL_MEM16_CFM,             /*!< 0x1E  Fill memory confirm (16 bit mode) */ 

  READ_PROPRIETARY_DATA_REQ,  /*!< 0x1F  Read proprietary data request  */ 
  READ_PROPRIETARY_DATA_CFM,  /*!< 0x20  Read proprietary data confirm  */ 

  READ_PROG_MEM16_REQ,        /*!< 0x21  Read program memory request (16 bit mode) */ 
  READ_PROG_MEM16_CFM,        /*!< 0x22  Read program memory confirm (16 bit mode) */ 

  LAST_PRIMITIVE,                         
} ENUM8(PrimitiveType);


/// Flashloader Configurations word
typedef enum
{
  INTERNAL_FLASH_CONFIG,      /*!< Internal flash  */
  EXTERNAL_FLASH_CONFIG,      /*!< External flash  */
  SPI_FLASH_CONFIG,           /*!< SPI flash  */
  QSPI_FLASH_CONFIG,          /*!< QSPI flash  */
} ENUM8(FlashLoaderConfigType);    


//--------------------------------------
//#include <Include/Standard/AlignBegin.h>
//--------------------------------------
/// Parameter indication type 
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             Data[1];        /*!< Array of data (8bit) */
} ParmIndType;
/// Read memory request type
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
} ReadMemReqType;
/// Data Mail with 8 bits data
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint8             Data[1];        /*!< Array of data (8 bit) */
} Unit8DataMailType;
/// Data Mail with 16 bits data
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint16            Data[1];        /*!< Array of data (16 bit)  */
} Unit16DataMailType;
/// Data Mail with 32 bits data
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint32            Data[1];        /*!< Array of data (32 bit)  */
} Unit32DataMailType;
/// Write data to memory confirm mail 
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint8             Confirm;        /*!< Confirm TRUE/FALSE */
} WriteMemCfmType;
/// Read software version mail
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
} SwVersionReqType;

typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
} EmptySignalType;

/// Software version confirm mail 
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             Version;        /*!< Application software version */
  uint8             Revision;       /*!< Application software revision */
  uint8             FlashLoaderId;  /*!< Application software Flash loader identification number */
} SwVersionCfmType;
/// Set Application configuration request
typedef struct
{
  PrimitiveType         Primitive;      /*!< Primitive */
  uint32                FirstProgWord;  /*!< First Program word */
  uint32                SecondProgWord; /*!< Second Program word  */
  uint32                OffsetAddress;  /*!< Data offset address  */
  FlashLoaderConfigType Config;         /*!< Configuration word */
} WriteConfigReqType;
/// Set Application configuration confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             Confirm;        /*!< Confirm TRUE/FALSE */
} WriteConfigCfmType;
/// Program flash data (8 bit mode) request
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint8             Data[1];        /*!< array of 16 bits data */
} ProgFlashData8ReqType;
/// Program flash data (16 bit mode) request
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint16            Data[1];        /*!< array of 16 bits data */
} ProgFlashData16ReqType;
/// Program flash data (32 bit mode) request
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint32            Data[1];        /*!< array of 32 bits data */
} ProgFlashData32ReqType;          
/// Program flash data confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint16            Length;         /*!< Length of data */
  uint8             Confirm;        /*!< Confirm TRUE/FALSE */
} ProgFlashCfmType;
/// Calculate Crc32 request
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address */
  uint32            Length;         /*!< Length of data */
} CalcCrc32ReqType;
/// Calculate Crc32 confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address  */
  uint32            Length;         /*!< Length of data */
  uint32            Crc32;          /*!< Crc32 */
} CalcCrc32CfmType;
/// Erase flash request
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address  */
  uint16            EraseCommand;   /*!< Erase command Chip/Sector/Block */    
} EraseFlashReqType;
/// Erase flash confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Address;        /*!< Memory address  */
  uint8             Confirm;        /*!< Confirm TRUE/FALSE */
} EraseFlashCfmType;
/// Change baudrate request
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32            Baudrate;       /*!< New baudrate */
} SetBaudrateReqType;
/// Change baudrate confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             Confirm;        /*!< Confirm TRUE/FALSE */
} SetBaudrateCfmType;
/// Write data to BMC register. Data is written to register after sending confirm mail.
typedef struct
{
  PrimitiveType     Primitive;       /*!< Primitive */
  uint32            RegisterAddress; /*!< Register address */
  uint16            RegisterData;    /*!< Register data  */
} WriteBmcRegReqType;
/// Change baudrate confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             Confirm;        /*!< Confirm TRUE/FALSE */
} WriteBmcRegCfmType;
/// Jump to address - change PC counter 
typedef struct
{
  PrimitiveType     Primitive;       /*!< Primitive */
  uint32            JumpAddress;     /*!< Address to jump to*/
} JumpToAddressReqType;
/// Jump to address confirm 
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             Confirm;        /*!< Confirm TRUE/FALSE */
} JumpToAddressCfmType;
/// Read Proprietary Data Request
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             RequestID;      /*!< Request ID */
} ReadProprietaryDataReqType;
/// Read Proprietary Data Confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             RequestID;      /*!< Request ID */
  uint16            Length;         /*!< Length of data */
  uint8             Data[1];        /*!< array of 8 bits data */
} ReadProprietaryDataCfmType;
/// Read Proprietary Data QSPI flashtype Confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             RequestID;      /*!< Request ID */
  uint8             MaxFreqSingle;  /*!< MaxFreqSingle */
  uint8             MaxFreqQuad;    /*!< MaxFreqQuad   */
  uint8             Manufacturer;   /*!< Manufacturer */
  uint16            DeviceId;       /*!< DeviceId      */
  uint32            TotalSize;      /*!< TotalSize     */
  uint32            SectorSize;     /*!< SectorSize    */
} ReadQSPIFlashTypeCfmType;
/// Read Proprietary Data SPI flashtype Confirm
typedef struct
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8             RequestID;      /*!< Request ID */
  uint8             MaxFreqSingle;  /*!< MaxFreqSingle */
  uint8             MaxFreqQuad;    /*!< MaxFreqQuad   */
  uint16            Manufacturer;   /*!< Manufacturer */
  uint16            DeviceId;       /*!< DeviceId      */
  uint32            TotalSize;      /*!< TotalSize     */
  uint32            SectorSize;     /*!< SectorSize    */
} ReadSPIFlashTypeCfmType;


/// MailType
typedef union   
{
  ParmIndType                 ParmInd;                 /*!< Parameter indication type */
  ReadMemReqType              ReadMemReq;              /*!<  */
  Unit8DataMailType           Unit8DataMail;           /*!<  */
  Unit16DataMailType          Unit16DataMail;          /*!<  */
  Unit32DataMailType          Unit32DataMail;          /*!<  */
  WriteMemCfmType             WriteMemCfm;             /*!<  */
  SwVersionReqType            SwVersionReq;            /*!<  */
  SwVersionCfmType            SwVersionCfm;            /*!<  */
  WriteConfigReqType          WriteConfigReq;          /*!<  */
  WriteConfigCfmType          WriteConfigCfm;          /*!<  */
  ProgFlashData8ReqType       ProgFlashData8Req;       /*!<  */
  ProgFlashData16ReqType      ProgFlashData16Req;      /*!<  */
  ProgFlashData32ReqType      ProgFlashData32Req;      /*!<  */
  ProgFlashCfmType            ProgFlashCfm;            /*!<  */
  CalcCrc32ReqType            CalcCrc32Req;            /*!<  */
  CalcCrc32CfmType            CalcCrc32Cfm;            /*!<  */
  EraseFlashReqType           EraseFlashReq;           /*!<  */
  EraseFlashCfmType           EraseFlashCfm;           /*!<  */
  SetBaudrateReqType          SetBaudrateReq;          /*!<  */
  SetBaudrateCfmType          SetBaudrateCfm;          /*!<  */
  WriteBmcRegReqType          WriteBmcRegReq;          /*!<  */
  WriteBmcRegCfmType          WriteBmcRegCfm;          /*!<  */
  JumpToAddressReqType        JumpToAddressReq;        /*!<  */
  JumpToAddressCfmType        JumpToAddressCfm;        /*!<  */     
  ReadProprietaryDataReqType  ReadProprietaryDataReq;  /*!<  */      
  ReadProprietaryDataCfmType  ReadProprietaryDataCfm;  /*!<  */     
  ReadQSPIFlashTypeCfmType    ReadQSPIFlashTypeCfm;    /*!<  */      
  ReadSPIFlashTypeCfmType     ReadSPIFlashTypeCfm;     /*!<  */      

} MyMailType;


//------------------------------------
//#include <Include/Standard/AlignEnd.h>
//------------------------------------


//-----------------------------------------------------------------------------
// General Mail pointer macro
//-----------------------------------------------------------------------------
/** \name Mail pointer macro
*/
//@{
#define MAIL_PTR ((MyMailType *) MailPtr)
//@}
//-----------------------------------------------------------------------------





/****************************************************************************
*                           Global variables/const                           
****************************************************************************/


/****************************************************************************
*                             Function prototypes                            
****************************************************************************/

/** \name External array of Mail strings array  
*/
//@{
extern const char *MailNameStr[];
//@}

/// Receive Mail Error Types 
typedef enum
{
  NO_MAILRECEIVED_YET,      /*!< Mail not received yet  */
  MAIL_RECEIVED_NOW,        /*!< A mail is received now */
  MAIL_HEADER_ERROR,        /*!< No correct mail header received */
  MAIL_LENGTH_ERROR,        /*!< Data length field out of range */
  MAIL_CRC_ERROR,           /*!< Mail crc error */
  MAIL_BYTE_TIMEOUT_ERROR,  /*!< Timeout error - time space between bytes to long. */
} MailReceivedErrorType;    

/// Mail Receiver states
typedef enum
{
  FIND_PACKET_HEADER,       /*!< Looking for mail header  */
  WAIT_FOR_LSB_LENGTH,      /*!< Waiting for lsb length   */
  WAIT_FOR_MSB_LENGTH,      /*!< Waiting for msb length   */
  RX_PAYLOAD,               /*!< Receiving payload data   */
  WAIT_FOR_LSB_CRC,         /*!< Waiting for lsb CRC      */
  WAIT_FOR_MSB_CRC,         /*!< Waiting for msb CRC      */
} ENUM8(RxStateType);

/// Mail Receiver statics data
typedef struct
{
    uint8       RxMailBuffer[MAX_MAIL_BUFFER_SIZE];     /*!< Temporary mail buffer  */
    RxStateType RxState;                                /*!< Current mail receiver state  */
    uint16      PayloadLength;                          /*!< Current Payload data length   */
    uint16      Index;                                  /*!< Current index to Temporary mail buffer  */
    uint16      RxCrc;                                  /*!< Received Crc  */
    uint16      Crc;                                    /*!< Calculated Crc  */
    uint32      ByteSpaceTimer;                         /*!< Timer to control Byte space time   */
}RxMailDataType;  

//=============================================================================
/// Receive mail from UART interface (polled)   
/** \param  RxMailData pointer to datastruct (local data used in routine MailReceived())
*   \param  RxByte number of received bytes
*   \param  MailPtr Pointer to received mail
*   \return  Status 
*/
MailReceivedErrorType MailReceived(RxMailDataType *RxMailData ,uint8 RxByte, MyMailType **MailPtr);


#endif

