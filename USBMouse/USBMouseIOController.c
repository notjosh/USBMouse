//
//  USBMouseIOController.c
//  USBMouse
//
//  Created by joshua may on 15/6/16.
//  Copyright © 2016 joshua may. All rights reserved.
//

#include "USBMouseIOController.h"

#include "USBDeviceHandler.h"

static SInt32 const USBMouseIOControllerVendorID = 0xE6A;
static SInt32 const USBMouseIOControllerProductID = 0x101;

static IOUSBInterfaceInterface500 **intf_;
static USBMouseIOStatusCallback cb_;
static void *context_;

IOReturn _USBMouseIOHandshake1(IOUSBInterfaceInterface500 **intf);
IOReturn _USBMouseIOHandshake2(IOUSBInterfaceInterface500 **intf);
IOReturn _USBMouseIOPoke(IOUSBInterfaceInterface500 **intf, UInt32 command);

#pragma mark - Callbacks

void InterfaceUp(IOUSBInterfaceInterface500 **intf) {
    intf_ = intf;
    cb_(context_);
}

#pragma mark - API

void USBMouseIOInit(USBMouseIOStatusCallback cb, void *context) {
    cb_ = cb;
    context_ = context;

    USBDeviceHandlerInit(USBMouseIOControllerVendorID,
                         USBMouseIOControllerProductID,
                         InterfaceUp);
}

void USBMouseIOHandshake() {
    if (NULL == intf_) {
        printf("Interface isn't up! Bailing..");
        return;
    }

    _USBMouseIOHandshake1(intf_);
    _USBMouseIOHandshake2(intf_);
}

void USBMouseIOPoke(UInt32 command) {
    if (NULL == intf_) {
        printf("Interface isn't up! Bailing..");
        return;
    }

    _USBMouseIOPoke(intf_, command);
}

#pragma mark - Private API

IOReturn _USBMouseIOPoke(IOUSBInterfaceInterface500 **intf, UInt32 command)
{
    IOReturn err = 0;
    IOUSBDevRequest req;

    req.bmRequestType = USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice);
    req.bRequest = 0x0c;
    req.wValue = 0x5001;
    req.wIndex = 0x0000;
    req.wLength = 0x0001;
    req.pData = &command;

    command = HostToUSBLong(command);

    err = (*intf)->ControlRequest(intf, 0, &req);
    if (kIOReturnSuccess == err)
    {
        if (req.wLenDone != 1)
        {
            err = kIOReturnUnderrun;
        }
    }

    return err;
}

IOReturn _USBMouseIOHandshake1(IOUSBInterfaceInterface500 **intf)
{
    IOReturn err = 0;
    IOUSBDevRequest req;

    req.bmRequestType = USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice);
    req.bRequest = 0x0c;
    req.wValue = 0x5003;
    req.wIndex = 0xffff;
    req.wLength = 0x0001;

    UInt32 bits = 0x8;
    req.pData = &bits;

    bits = HostToUSBLong(bits);
#if NARRATEIO
    printf("Doing MouseHandshake1 with portbits %08x\n", USBToHostLong(bits));
#endif

    err = (*intf)->ControlRequest(intf, 0, &req);
    if (kIOReturnSuccess == err)
    {
        if (req.wLenDone != 1)
        {
            err = kIOReturnUnderrun;
        }
    }

    return err;
}

IOReturn _USBMouseIOHandshake2(IOUSBInterfaceInterface500 **intf)
{
    IOReturn err = 0;
    IOUSBDevRequest req;

    req.bmRequestType = USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice);
    req.bRequest = 0x0c;
    req.wValue = 0x5003;
    req.wIndex = 0x00f0;
    req.wLength = 0x0001;

    UInt32 bits = 0x10;
    req.pData = &bits;

    bits = HostToUSBLong(bits);
#if NARRATEIO
    printf("Doing MouseHandshake2 with portbits %08x\n", USBToHostLong(bits));
#endif

    err = (*intf)->ControlRequest(intf, 0, &req);
    if (kIOReturnSuccess == err)
    {
        if (req.wLenDone != 1)
        {
            err = kIOReturnUnderrun;
        }
    }
    
    return err;
}
