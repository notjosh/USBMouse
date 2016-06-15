//
//  USBDeviceHandler.h
//  USBMouse
//
// Sourced mostly from Apple samples
//

#ifndef USBDeviceHandler_h
#define USBDeviceHandler_h

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/usb/IOUSBLib.h>

typedef void (*USBDeviceHandlerStatusCallback)(IOUSBInterfaceInterface500 **intf);

void USBDeviceHandlerInit(SInt32 vendorID, SInt32 productID, USBDeviceHandlerStatusCallback cb);

#endif /* USBDeviceHandler_h */
