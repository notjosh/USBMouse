//
//  USBMouseIOController.h
//  USBMouse
//
//  Created by joshua may on 15/6/16.
//  Copyright © 2016 joshua may. All rights reserved.
//

#ifndef USBMouseIOController_h
#define USBMouseIOController_h

#include <CoreFoundation/CoreFoundation.h>

typedef void (*USBMouseIOStatusCallback)(void *);

void USBMouseIOInit(USBMouseIOStatusCallback, void *);

void USBMouseIOHandshake();
void USBMouseIOPoke(UInt32 command);

#endif
