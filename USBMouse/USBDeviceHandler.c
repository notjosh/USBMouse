//
//  USBDeviceHandler.c
//  USBMouse
//
// Sourced mostly from Apple samples
//

#include "USBDeviceHandler.h"

#include <IOKit/usb/IOUSBLib.h>

static IONotificationPortRef notificationPort;
static CFRunLoopRef runLoop;
static io_iterator_t iterator;

USBDeviceHandlerStatusCallback cb_;


void printInterpretedError(char *s, IOReturn err)
{
    UInt32 system, sub, code;

    fprintf(stderr, "%s (0x%08X) ", s, err);

    system = err_get_system(err);
    sub = err_get_sub(err);
    code = err_get_code(err);

    if(system == err_get_system(sys_iokit))
    {
        if(sub == err_get_sub(sub_iokit_usb))
        {
            fprintf(stderr, "USB error %d(0x%X) ", (int)code, (unsigned int)code);
        }
        else if(sub == err_get_sub(sub_iokit_common))
        {
            fprintf(stderr, "IOKit common error %d(0x%X) ", (int)code, (unsigned int)code);
        }
        else
        {
            fprintf(stderr, "IOKit error %d(0x%X) from subsytem %d(0x%X) ", (int)code, (unsigned int)code, (int)sub, (unsigned int)sub);
        }
    }
    else
    {
        fprintf(stderr, "error %d(0x%X) from system %d(0x%X) - subsytem %d(0x%X) ", (int)code, (unsigned int)code, (int)system, (unsigned int)system, (int)sub, (unsigned int)sub);
    }
}

void useInterface(IOUSBInterfaceInterface500 **intf)
{
    cb_(intf);
}

UInt32 openInterface(IOUSBInterfaceInterface500 **intf)
{
    IOReturn ret;

    ret = (*intf)->USBInterfaceOpen(intf);
    if (ret != kIOReturnSuccess) {
        printInterpretedError("Could not set configuration on device", ret);
        return(-1);
    }

    return(0);
}

IOUSBInterfaceInterface500 **getInterfaceInterface(io_service_t usbInterface)
{
    IOReturn err;
    IOCFPlugInInterface **plugInInterface=NULL;
    IOUSBInterfaceInterface500 **intf=NULL;
    SInt32 score;
    HRESULT res;

    // There is no documentation for IOCreatePlugInInterfaceForService or QueryInterface, you have to use sample code.

    err = IOCreatePlugInInterfaceForService(usbInterface,
                                            kIOUSBInterfaceUserClientTypeID,
                                            kIOCFPlugInInterfaceID,
                                            &plugInInterface,
                                            &score);
    (void)IOObjectRelease(usbInterface);				// done with the usbInterface object now that I have the plugin
    if ((kIOReturnSuccess != err) || (plugInInterface == nil) )
    {
        printInterpretedError("Unable to create plug in interface for USB interface", err);
        return(nil);
    }

    res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID500), (LPVOID)&intf);
    IODestroyPlugInInterface(plugInInterface);			// done with this

    if (res || !intf)
    {
        fprintf(stderr, "Unable to create interface with QueryInterface %X\n", (unsigned int)res);
        return(nil);
    }
    return(intf);
}

Boolean isThisTheInterfaceYoureLookingFor(IOUSBInterfaceInterface500 **intf)
{
    //	Check to see if this is the interface you're interested in
    //  This code is only expecting one interface, so returns true
    //  the first time.
    //  You code could check the nature and type of endpoints etc

    //    static Boolean foundOnce  = false;
    //    if(foundOnce)
    //    {
    //        fprintf(stderr, "Subsequent interface found, we're only intersted in 1 of them\n");
    //        return(false);
    //    }
    //    foundOnce = true;
    return true;
}

int iterateInterfaces(io_iterator_t interfaceIterator)
{
    io_service_t usbInterface;
    int err = 0;
    IOUSBInterfaceInterface500 **intf=NULL;

    usbInterface = IOIteratorNext(interfaceIterator);
    if (usbInterface == IO_OBJECT_NULL) {
        fprintf(stderr, "Unable to find an Interface\n");
        return(-1);
    }

    while (usbInterface != IO_OBJECT_NULL) {
        intf = getInterfaceInterface(usbInterface);

        if (intf != nil) {
            // Don't release the interface here. That's one too many releases and causes set alt interface to fail
            if (isThisTheInterfaceYoureLookingFor(intf)) {
                err = openInterface(intf);
                if (err == 0) {
                    useInterface(intf);
                    //                    ret = (*intf)->USBInterfaceClose(intf);
                }

                //                ret = (*intf)->Release(intf);
                // Not worth bothering with errors here
                return(err);
            }
        }
        usbInterface = IOIteratorNext(interfaceIterator);
    }

    fprintf(stderr, "No interesting interfaces found\n");
    IOObjectRelease(usbInterface);
    return(-1);
}

IOUSBDeviceInterface **getDevice(io_object_t usbDevice) {
    IOReturn err;
    IOCFPlugInInterface **plugInInterface = NULL;
    IOUSBDeviceInterface **device = NULL;
    SInt32 score;
    HRESULT res;

    // There is no documentation for IOCreatePlugInInterfaceForService or QueryInterface, you have to use sample code.

    err = IOCreatePlugInInterfaceForService(usbDevice,
                                            kIOUSBDeviceUserClientTypeID,
                                            kIOCFPlugInInterfaceID,
                                            &plugInInterface,
                                            &score);
    if ((kIOReturnSuccess != err) || (plugInInterface == nil) )
    {
        fprintf(stderr, "Unable to create plug in interface for USB device (err: %d)", err);
        return NULL;
    }

    res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID650), (LPVOID)&device);
    IODestroyPlugInInterface(plugInInterface);

    if (res || !device)
    {
        fprintf(stderr, "Unable to create USB device with QueryInterface\n");
        return NULL;
    }

    return device;
}

SInt32 openDevice(IOUSBDeviceInterface **dev)
{
    UInt8 numConfig;
    IOReturn err;
    IOUSBConfigurationDescriptorPtr descriptor;

    err = (*dev)->GetNumberOfConfigurations(dev, &numConfig);
    if (err != kIOReturnSuccess) {
        fprintf(stderr, "Could not number of configurations from device (err: %d)", err);
        return -1;
    }
    if (numConfig != 1) {
        fprintf(stderr, "This does not look like the right device, it has %d configurations (we want 1)\n", numConfig);
        return -1 ;
    }

    err = (*dev)->GetConfigurationDescriptorPtr(dev, 0, &descriptor);
    if (err != kIOReturnSuccess) {
        fprintf(stderr, "Could not get configuration descriptor from device (err: %d)", err);
        return -1;
    }

    // We should really try to do classic arbitration here
    err = (*dev)->USBDeviceOpen(dev);
    if (err != kIOReturnSuccess) {
        fprintf(stderr, "Could not open device (err: %d)", err);
        return -1;
    }

    return descriptor->bConfigurationValue;
}

void useDevice(IOUSBDeviceInterface **dev, UInt32 configuration)
{
    io_iterator_t interfaceIterator;
    IOUSBFindInterfaceRequest req;
    IOReturn err;

    err = (*dev)->SetConfiguration(dev, configuration);
    if (err != kIOReturnSuccess) {
        fprintf(stderr, "Could not set configuration on device (err: %d)", err);
        return;
    }

    req.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    req.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    req.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    req.bAlternateSetting = kIOUSBFindInterfaceDontCare;

    err = (*dev)->CreateInterfaceIterator(dev, &req, &interfaceIterator);
    if (err != kIOReturnSuccess) {
        fprintf(stderr, "Could not create interface iterator (err: %d)", err);
        return;
    }

    err = iterateInterfaces(interfaceIterator);

    IOObjectRelease(interfaceIterator);
}

#pragma mark - Notifications

void SignalHandler(int v) {
    // closing process, let's clean up anything we need to..
    fprintf(stderr, "\nInterrupted.\n");
}

void DeviceAdded(void *refCon, io_iterator_t addedIterator) {
    io_object_t deviceRef;
    IOUSBDeviceInterface **device = NULL;
    SInt32 config = 0;

    while ((deviceRef = IOIteratorNext(addedIterator))) {
        device = getDevice(deviceRef);

        if (device != NULL) {
            config = openDevice(device);
            
            if (config >= 0) {
                useDevice(device, config);
            }
            
            (*device)->Release(device);
        }
        
        IOObjectRelease(deviceRef);
    }
}

void USBDeviceHandlerInit(SInt32 vendorID, SInt32 productID, USBDeviceHandlerStatusCallback cb) {
    cb_ = cb;
    // uh oh
    sig_t oldHandler = signal(SIGINT, SignalHandler);
    if (oldHandler == SIG_ERR) {
        fprintf(stderr, "Could not establish new signal handler.");
    }

    // set up device listener
    CFMutableDictionaryRef matching = IOServiceMatching(kIOUSBDeviceClassName);
    if (matching == NULL) {
        fprintf(stderr, "Could create matching dictionary\n");
        return;
    }

    CFNumberRef numberRef;
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendorID);
    CFDictionarySetValue(matching,
                         CFSTR(kUSBVendorID),
                         numberRef);
    CFRelease(numberRef);

    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &productID);
    CFDictionarySetValue(matching,
                         CFSTR(kUSBProductID),
                         numberRef);
    CFRelease(numberRef);
    numberRef = NULL;

    notificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    runLoop = CFRunLoopGetCurrent();

    CFRunLoopAddSource(runLoop,
                       IONotificationPortGetRunLoopSource(notificationPort),
                       kCFRunLoopDefaultMode);

    // Now set up a notification to be called when a device is first matched by I/O Kit.
    kern_return_t kr = IOServiceAddMatchingNotification(notificationPort,
                                                        kIOFirstMatchNotification,
                                                        matching,
                                                        DeviceAdded,
                                                        NULL,
                                                        &iterator);

    if (kr != kIOReturnSuccess)
    {
        fprintf(stderr, "Could not register for matching notification (err: %d)", kr);
        return;
    }

    // Iterate once to get already-present devices and arm the notification
    DeviceAdded(NULL, iterator);
}
