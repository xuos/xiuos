/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-10-01     Yi Qiu       first version
 * 2013-04-26     aozima       add DEVICEQUALIFIER support.
 * 2017-11-15     ZYH          fix ep0 transform error
 */

/**
* @file usb_common.h
* @brief define usb function and struct
* @version 1.0 
* @author AIIT XUOS Lab
* @date 2021-04-25
*/

/*************************************************
File name: usb_common.h
Description: define usb function and common struct
Others: take RT-Thread v4.0.2/components/drivers/include/drivers/usb_common.h for references
                https://github.com/RT-Thread/rt-thread/tree/v4.0.2
History: 
1. Date: 2021-04-25
Author: AIIT XUOS Lab
Modification: 
1. define usb common configure, function and struct
2. support bus driver framework 
*************************************************/

#ifndef USB_COMMON_H
#define USB_COMMON_H

#include <xiuos.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_DEBUG_USB                                0x00
#define USB_DYNAMIC                                      0x00

#define USB_CLASS_DEVICE                            0x00
#define USB_CLASS_AUDIO                              0x01
#define USB_CLASS_CDC                                   0x02
#define USB_CLASS_HID                                    0x03
#define USB_CLASS_PHYSICAL                       0x05
#define USB_CLASS_IMAGE                              0x06
#define USB_CLASS_PRINTER                         0x07
#define USB_CLASS_MASS_STORAGE          0x08
#define USB_CLASS_HUB                                  0x09
#define USB_CLASS_CDC_DATA                      0x0a
#define USB_CLASS_SMART_CARD                0x0b
#define USB_CLASS_SECURITY                        0x0d
#define USB_CLASS_VIDEO                               0x0e
#define USB_CLASS_HEALTHCARE                 0x0f
#define USB_CLASS_DIAG_DEVICE                0xdc
#define USB_CLASS_WIRELESS                       0xe0
#define USB_CLASS_MISC                                  0xef
#define USB_CLASS_APP_SPECIFIC              0xfe
#define USB_CLASS_VEND_SPECIFIC           0xff

#define USB_DESC_TYPE_DEVICE                         0x01
#define USB_DESC_TYPE_CONFIGURATION     0x02
#define USB_DESC_TYPE_STRING                        0x03
#define USB_DESC_TYPE_INTERFACE                 0x04
#define USB_DESC_TYPE_ENDPOINT                  0x05
#define USB_DESC_TYPE_DEVICEQUALIFIER   0x06
#define USB_DESC_TYPE_OTHERSPEED            0x07
#define USB_DESC_TYPE_IAD                                 0x0b
#define USB_DESC_TYPE_HID                                 0x21
#define USB_DESC_TYPE_REPORT                       0x22
#define USB_DESC_TYPE_PHYSICAL                    0x23
#define USB_DESC_TYPE_HUB                               0x29

#define USB_DESC_LENGTH_DEVICE                    0x12
#define USB_DESC_LENGTH_CONFIG                   0x9
#define USB_DESC_LENGTH_IAD                            0x8
#define USB_DESC_LENGTH_STRING                   0x4
#define USB_DESC_LENGTH_INTERFACE            0x9
#define USB_DESC_LENGTH_ENDPOINT             0x7

#define USB_REQ_TYPE_STANDARD                      0x00
#define USB_REQ_TYPE_CLASS                               0x20
#define USB_REQ_TYPE_VENDOR                           0x40
#define USB_REQ_TYPE_MASK                                 0x60

#define USB_REQ_TYPE_DIR_OUT                          0x00
#define USB_REQ_TYPE_DIR_IN                               0x80

#define USB_REQ_TYPE_DEVICE                               0x00
#define USB_REQ_TYPE_INTERFACE                       0x01
#define USB_REQ_TYPE_ENDPOINT                        0x02
#define USB_REQ_TYPE_OTHER                                0x03
#define USB_REQ_TYPE_RECIPIENT_MASK          0x1f

#define USB_FEATURE_ENDPOINT_HALT                 0x00
#define USB_FEATURE_DEV_REMOTE_WAKEUP   0x01
#define USB_FEATURE_TEST_MODE                          0x02

#define USB_REQ_GET_STATUS                                   0x00
#define USB_REQ_CLEAR_FEATURE                           0x01
#define USB_REQ_SET_FEATURE                                0x03
#define USB_REQ_SET_ADDRESS                               0x05
#define USB_REQ_GET_DESCRIPTOR                        0x06
#define USB_REQ_SET_DESCRIPTOR                        0x07
#define USB_REQ_GET_CONFIGURATION               0x08
#define USB_REQ_SET_CONFIGURATION               0x09
#define USB_REQ_GET_INTERFACE                           0x0A
#define USB_REQ_SET_INTERFACE                           0x0B
#define USB_REQ_SYNCH_FRAME                              0x0C
#define USB_REQ_SET_ENCRYPTION                       0x0D
#define USB_REQ_GET_ENCRYPTION                       0x0E
#define USB_REQ_RPIPE_ABORT                                0x0E
#define USB_REQ_SET_HANDSHAKE                        0x0F
#define USB_REQ_RPIPE_RESET                                 0x0F
#define USB_REQ_GET_HANDSHAKE                        0x10
#define USB_REQ_SET_CONNECTION                      0x11
#define USB_REQ_SET_SECURITY_DATA                 0x12
#define USB_REQ_GET_SECURITY_DATA                0x13
#define USB_REQ_SET_WUSB_DATA                        0x14
#define USB_REQ_LOOPBACK_DATA_WRITE         0x15
#define USB_REQ_LOOPBACK_DATA_READ           0x16
#define USB_REQ_SET_INTERFACE_DS                   0x17

#define USB_STRING_LANGID_INDEX                       0x00
#define USB_STRING_MANU_INDEX                          0x01
#define USB_STRING_PRODUCT_INDEX                  0x02
#define USB_STRING_SERIAL_INDEX                        0x03
#define USB_STRING_CONFIG_INDEX                       0x04
#define USB_STRING_INTERFACE_INDEX                0x05
#define USB_STRING_OS_INDEX                                 0x06
#define USB_STRING_MAX                  USB_STRING_OS_INDEX

#define USB_STRING_OS                   "MSFT100A"

#define USB_PID_OUT                     0x01
#define USB_PID_ACK                      0x02
#define USB_PID_DATA0                 0x03
#define USB_PID_SOF                     0x05
#define USB_PID_IN                         0x09
#define USB_PID_NACK                  0x0A
#define USB_PID_DATA1                0x0B
#define USB_PID_PRE                     0x0C
#define USB_PID_SETUP                0x0D
#define USB_PID_STALL                 0x0E

#define USB_EP_DESC_OUT                           0x00
#define USB_EP_DESC_IN                                0x80
#define USB_EP_DESC_NUM_MASK            0x0f

#define USB_EP_ATTR_CONTROL             0x00
#define USB_EP_ATTR_ISOC                       0x01
#define USB_EP_ATTR_BULK                      0x02
#define USB_EP_ATTR_INT                          0x03
#define USB_EP_ATTR_TYPE_MASK        0x03

#define USB_EPNO_MASK                            0x7f
#define USB_DIR_OUT                                   0x00
#define USB_DIR_IN                                        0x80
#define USB_DIR_INOUT                               0x40
#define USB_DIR_MASK                                 0x80

#define ID_UNASSIGNED               0
#define ID_ASSIGNED                     1

#define RH_GET_PORT_STATUS              0
#define RH_SET_PORT_STATUS              1
#define RH_CLEAR_PORT_FEATURE      2
#define RH_SET_PORT_FEATURE           3

#define USB_BUS_POWERED                  0
#define USB_SELF_POWERED                1
#define USB_REMOTE_WAKEUP            1
#define USB_EP_HALT                                0


#define PORT_FEAT_CONNECTION            0
#define PORT_FEAT_ENABLE                        1
#define PORT_FEAT_SUSPEND                    2
#define PORT_FEAT_OVER_CURRENT       3
#define PORT_FEAT_RESET                           4
#define PORT_FEAT_POWER                         8
#define PORT_FEAT_LOWSPEED                 9
#define PORT_FEAT_HIGHSPEED               10
#define PORT_FEAT_C_CONNECTION      16
#define PORT_FEAT_C_ENABLE                  17
#define PORT_FEAT_C_SUSPEND               18
#define PORT_FEAT_C_OVER_CURRENT 19
#define PORT_FEAT_C_RESET                      20


#define PORT_CCS                        0x00000001UL    
#define PORT_PES                        0x00000002UL    
#define PORT_PSS                        0x00000004UL   
#define PORT_POCI                      0x00000008UL    
#define PORT_PRS                        0x00000010UL    
#define PORT_PPS                        0x00000100UL    
#define PORT_LSDA                      0x00000200UL    
#define PORT_CCSC                      0x00010000UL
#define PORT_PESC                      0x00020000UL
#define PORT_PSSC                      0x00040000UL
#define PORT_POCIC                    0x00080000UL
#define PORT_PRSC                      0x00100000UL


#define HUB_STATUS_LOCAL_POWER          0x0001
#define HUB_STATUS_OVERCURRENT          0x0002

#define HUB_CHANGE_LOCAL_POWER          0x0001
#define HUB_CHANGE_OVERCURRENT          0x0002

#define USB_EP_ATTR(attr)                          (attr & USB_EP_ATTR_TYPE_MASK)
#define USB_EP_DESC_NUM(addr)           (addr & USB_EP_DESC_NUM_MASK)
#define USB_EP_DIR(addr)                           ((addr & USB_DIR_MASK)>>7)

#define HID_REPORT_ID_KEYBOARD1         1
#define HID_REPORT_ID_KEYBOARD2         2
#define HID_REPORT_ID_KEYBOARD3         3
#define HID_REPORT_ID_KEYBOARD4         7
#define HID_REPORT_ID_MEDIA                     4
#define HID_REPORT_ID_GENERAL               5
#define HID_REPORT_ID_MOUSE                   6


#ifndef USB_TIMEOUT_BASIC
#define USB_TIMEOUT_BASIC               (TICK_PER_SECOND)       
#endif
#ifndef USB_TIMEOUT_LONG
#define USB_TIMEOUT_LONG                (TICK_PER_SECOND * 5)  
#endif
#ifndef USB_DEBOUNCE_TIME
#define USB_DEBOUNCE_TIME               (TICK_PER_SECOND / 5)   
#endif

#define uswap_32(x) \
    ((((x) & 0xff000000) >> 24) | \
     (((x) & 0x00ff0000) >>  8) | \
     (((x) & 0x0000ff00) <<  8) | \
     (((x) & 0x000000ff) << 24))

#define  Uswap8(x) \
    (((uint16)(*((uint8 *)(x)))) + \
    (((uint16)(*(((uint8 *)(x)) + 1))) << 8))

typedef void (*FuncCallback) (void *context);

typedef enum
{
    USB_STATE_NOTATTACHED = 0,
    USB_STATE_ATTACHED,
    USB_STATE_POWERED,
    USB_STATE_RECONNECTING,
    USB_STATE_UNAUTHENTICATED,
    USB_STATE_DEFAULT,
    USB_STATE_ADDRESS,
    USB_STATE_CONFIGURED,
    USB_STATE_SUSPENDED
}UdeviceStatePointer;

typedef enum
{
    STAGE_IDLE,
    STAGE_SETUP,
    STAGE_STATUS_IN,
    STAGE_STATUS_OUT,
    STAGE_DIN,
    STAGE_DOUT
} Uep0StagePointer;

#pragma pack(1)

struct UsbDescriptor
{
    uint8 bLength;
    uint8 type;
};
typedef struct UsbDescriptor* UdescPointer;

struct UdeviceDescriptor
{
    uint8 bLength;
    uint8 type;
    uint16 bcdUSB;
    uint8 bDeviceClass;
    uint8 bDeviceSubClass;
    uint8 bDeviceProtocol;
    uint8 bMaxPacketSize0;
    uint16 idVendor;
    uint16 idProduct;
    uint16 bcdDevice;
    uint8 iManufacturer;
    uint8 iProduct;
    uint8 iSerialNumber;
    uint8 bNumConfigurations;
};
typedef struct UdeviceDescriptor* UdevDescPointer;

struct UconfigDescriptor
{
    uint8 bLength;
    uint8 type;
    uint16 wTotalLength;
    uint8 bNumInterfaces;
    uint8 bConfigurationValue;
    uint8 iConfiguration;
    uint8 bmAttributes;
    uint8 MaxPower;
    uint8 data[256];
};
typedef struct UconfigDescriptor* UcfgDescPointer;

struct UinterfaceDescriptor
{
    uint8 bLength;
    uint8 type;
    uint8 bInterfaceNumber;
    uint8 bAlternateSetting;
    uint8 bNumEndpoints;
    uint8 bInterfaceClass;
    uint8 bInterfaceSubClass;
    uint8 bInterfaceProtocol;
    uint8 iInterface;
};
typedef struct UinterfaceDescriptor* UintfDescPointer;


struct UiadDescriptor
{
    uint8 bLength;
    uint8 bDescriptorType;
    uint8 bFirstInterface;
    uint8 bInterfaceCount;
    uint8 bFunctionClass;
    uint8 bFunctionSubClass;
    uint8 bFunctionProtocol;
    uint8 iFunction;
};
typedef struct UiadDescriptor* UiadDescPointer;

struct UendpointDescriptor
{
    uint8  bLength;
    uint8  type;
    uint8  bEndpointAddress;
    uint8  bmAttributes;
    uint16 wMaxPacketSize;
    uint8  bInterval;
};
typedef struct UendpointDescriptor* UepDescPointer;

struct UstringDescriptor
{
    uint8 bLength;
    uint8 type;
    uint8 String[64];
};
typedef struct UstringDescriptor* UstrDescPointer;

struct UhubDescriptor
{
    uint8 length;
    uint8 type;
    uint8 NumPorts;
    uint16 characteristics;
    uint8 PwronToGood;        
    uint8 current;
    uint8 removable[8];
    uint8 PwrCtl[8];
};
typedef struct UhubDescriptor* UhubDescPointer;


struct UsbQualifierDescriptor
{
    uint8  bLength;
    uint8  bDescriptorType;

    uint16 bcdUSB; 
    uint8  bDeviceClass;
    uint8  bDeviceSubClass;
    uint8  bDeviceProtocol;
    uint8  bMaxPacketSize0;
    uint8  bNumConfigurations;
    uint8  bRESERVED;
} __attribute__ ((packed));

struct UsbOsHeaderCompIdDescriptor
{
    uint32 dwLength;
    uint16 bcdVersion;
    uint16 wIndex;
    uint8  bCount;
    uint8  reserved[7];
};
typedef struct UsbOsHeaderCompIdDescriptor * UsbOsHeaderDescPointer;

struct UsbOsFunctionCompIdDescriptor
{
    DoubleLinklistType list;
    uint8 bFirstInterfaceNumber;
    uint8 reserved1;
    uint8 compatibleID[8];
    uint8 subCompatibleID[8];
    uint8 reserved2[6];
};
typedef struct UsbOsFunctionCompIdDescriptor * UsbOsFuncCompIdDescPointer;

struct UsbOsCompIdDescriptor
{
    struct UsbOsHeaderCompIdDescriptor HeadDesc;
    DoubleLinklistType FuncDesc;
};
typedef struct UsbOsCompIdDescriptor * UsbOsCompIdDescPointer;

struct UsbOsPropertyHeader
{
    uint32 dwLength;
    uint16 bcdVersion;
    uint16 wIndex;
    uint16 wCount;
};
typedef struct UsbOsPropertyHeader * UsbOsPropertyHeaderPointer;
struct UsbOsProerty
{
    uint32 dwSize;
    uint32 dwPropertyDataType;
    uint16 wPropertyNameLength;
    const char * bPropertyName;
    uint32 dwPropertyDataLength;
    const char * bPropertyData;
};
typedef struct UsbOsProerty * UsbOsProertyPointer;


#define USB_OS_PROERTY_TYPE_REG_SZ                                  0x01UL
#define USB_OS_PROERTY_TYPE_REG_EXPAND_SZ               0x02UL
#define USB_OS_PROERTY_TYPE_REG_BINARY                        0x03UL
#define USB_OS_PROERTY_TYPE_REG_DWORD_LITTLE_ENDIAN          0x04UL
#define USB_OS_PROERTY_TYPE_REG_DWORD_BIG_ENDIAN                 0x05UL
#define USB_OS_PROERTY_TYPE_REG_LINK                                                    0x06UL
#define USB_OS_PROERTY_TYPE_REG_MULTI_SZ                                         0x07UL

#define USB_OS_PROERTY_DESC(PropertyDataType,PropertyName,PropertyData) \
{\
    .dwSize                 = sizeof(struct UsbOsProerty)-sizeof(const char *)*2\
                            +sizeof(PropertyName)*2+sizeof(PropertyData)*2,\
    .dwPropertyDataType     = PropertyDataType,\
    .wPropertyNameLength    = sizeof(PropertyName)*2,\
    .bPropertyName          = PropertyName,\
    .dwPropertyDataLength   = sizeof(PropertyData)*2,\
    .bPropertyData          = PropertyData\
}


#ifndef HID_SUB_DESCRIPTOR_MAX
#define  HID_SUB_DESCRIPTOR_MAX        1
#endif

struct UhidDescriptor
{
    uint8  bLength;
    uint8  type;
    uint16 bcdHID;
    uint8  bCountryCode;
    uint8  bNumDescriptors;
    struct HidDescriptorList
    {
        uint8 type;
        uint16 wLength;
    }Descriptor[HID_SUB_DESCRIPTOR_MAX];
};
typedef struct UhidDescriptor* UhidDescPointer;

struct HidReport
{
    uint8 ReportId;
    uint8 report[63];
    uint8 size;
};
typedef struct HidReport* HidReportPointer;
extern void HIDReportReceived(HidReportPointer report);

struct urequest
{
    uint8  RequestType;
    uint8  bRequest;
    uint16 wValue;
    uint16 wIndex;
    uint16 wLength;
};
typedef struct urequest* UreqPointer;

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define USBREQ_GET_MAX_LUN              0xfe
#define USBREQ_MASS_STORAGE_RESET       0xff

#define SIZEOF_CSW                      0x0d
#define SIZEOF_CBW                      0x1f
#define SIZEOF_INQUIRY_CMD              0x24
#define SIZEOF_MODE_SENSE_6             0x4
#define SIZEOF_READ_CAPACITIES          0xc
#define SIZEOF_READ_CAPACITY            0x8
#define SIZEOF_REQUEST_SENSE            0x12

#define CBWFLAGS_DIR_M                  0x80
#define CBWFLAGS_DIR_IN                 0x80
#define CBWFLAGS_DIR_OUT                0x00

#define SCSI_TEST_UNIT_READY            0x00
#define SCSI_REQUEST_SENSE              0x03
#define SCSI_INQUIRY_CMD                0x12
#define SCSI_ALLOW_REMOVAL              0x1e
#define SCSI_MODE_SENSE_6               0x1a
#define SCSI_START_STOP                 0x1b
#define SCSI_READ_CAPACITIES            0x23
#define SCSI_READ_CAPACITY              0x25
#define SCSI_READ_10                    0x28
#define SCSI_WRITE_10                   0x2a
#define SCSI_VERIFY_10                  0x2f

#define CBW_SIGNATURE                   0x43425355
#define CSW_SIGNATURE                   0x53425355
#define CBW_TAG_VALUE                   0x12345678

struct UstorageCbw
{
    uint32 signature;
    uint32 tag;
    uint32 XferLen;
    uint8 dflags;
    uint8 lun;
    uint8 CbLen;
    uint8 cb[16];
};
typedef struct UstorageCbw* UstorageCbwPointer;

struct UstorageCsw
{
    uint32 signature;
    uint32 tag;
    int32 DataReside;
    uint8  status;
};
typedef struct UstorageCsw* UstorageCswPointer;

#pragma pack()


#ifndef USBD_THREAD_STACK_SZ
#define USBD_THREAD_STACK_SZ 512
#endif


#ifndef USBD_THREAD_PRIO
#define USBD_THREAD_PRIO 8
#endif


#ifdef __cplusplus
}
#endif

#endif