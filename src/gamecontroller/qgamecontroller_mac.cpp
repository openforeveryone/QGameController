/***************************************************************************
 *   Copyright (C) 2014 M Wellings                                         *
 *   info@openforeveryone.co.uk                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "qgamecontroller.h"
#include "qgamecontroller_mac_p.h"

#include <QDebug>

IOHIDManagerRef hidManager=NULL;


QGameControllerPrivate::QGameControllerPrivate(uint id, QGameController *q) :
    q_ptr(q)
{
    ID=id;
    Valid=false;
    Axis=0;
    Buttons=0;
//    qDebug("QGameController::QGameController(%i)", ID);
    // Create HID Manager reference
    if (hidManager==NULL)
    {
        hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        if (CFGetTypeID(hidManager) == IOHIDManagerGetTypeID()) {
            // this is a HID Manager reference!
//            qDebug() <<"HID Manager Created";
        }else return;
        // Create a matching dictionary

        CFDictionaryRef matchingCFDictRef = hu_CreateDeviceMatchingDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
        if (matchingCFDictRef) {
            //Set the HID device matching dictionary.
            IOHIDManagerSetDeviceMatching(hidManager, matchingCFDictRef);
        } else
            fprintf(stderr, "%s: hu_CreateDeviceMatchingDictionary failed.", __PRETTY_FUNCTION__);
        IOReturn tIOReturn = IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
        if(tIOReturn==kIOReturnSuccess){
//            qDebug() <<"IOHIDManagerOpen Success";
        }
            else return;
        CFRelease(matchingCFDictRef);
    }

    CFSetRef tCFSetRef = IOHIDManagerCopyDevices(hidManager);
    if (tCFSetRef==NULL) {
//        qDebug() << "No Joysticks Attached";
        return;
    }
    CFIndex count = CFSetGetCount(tCFSetRef);
//    qDebug() << count << "Joysticks" << ID;
    if (count<=(CFIndex)ID)
        return;
    const void *values[count];
    CFSetGetValues(tCFSetRef, (const void**)values);
    if (CFGetTypeID(values[ID]) == IOHIDDeviceGetTypeID()) {
//        qDebug() <<"Device found";
        device = (IOHIDDeviceRef)values[ID];
        CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));
        if (tCFTypeRef && CFStringGetTypeID() == CFGetTypeID(tCFTypeRef))
        {
            QString Property=CFStringRefToQString((CFStringRef)tCFTypeRef);
//            qDebug() <<"Manufacturer" << Property;
            Description.append(Property).append(" ");
        }
        CFTypeRef tCFTypeRef1 = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
        if (tCFTypeRef1 && CFStringGetTypeID() == CFGetTypeID(tCFTypeRef1))
        {
            QString Property=CFStringRefToQString((CFStringRef)tCFTypeRef1);
//            qDebug() <<"Product" << Property;
            Description.append(Property);
        }
    }else return;
    CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
    CFIndex elementcount = CFArrayGetCount(elementCFArrayRef);
//    qDebug() << elementcount ;

    for (int i = 0; i< elementcount; i++)
    {
        IOHIDElementRef element= (IOHIDElementRef)CFArrayGetValueAtIndex(elementCFArrayRef, i);
        if (CFGetTypeID(element) == IOHIDElementGetTypeID()) {
//            qDebug() << "Element found";
            IOHIDElementType type = IOHIDElementGetType(element);
            if (type==kIOHIDElementTypeInput_Axis || type==kIOHIDElementTypeInput_Misc)
            {
                int Max = IOHIDElementGetLogicalMax(element);
                int Min = IOHIDElementGetLogicalMin(element);
                if (IOHIDElementGetUsage(element)!=kHIDUsage_GD_Hatswitch)
                {
//                    qDebug() << "Axis found" << Min << Max;
                    Axis++;
                    axisElements.append(element);
                    axisMaxVals.append(Max);
                    axisMinVals.append(Min);
                }else
                {
//                    qDebug() << "HAT found";
                    Axis=Axis+2;
                    axisElements.append(element);
                    axisElements.append(element);
                    axisMaxVals.append(Max);
                    axisMinVals.append(Min);
                    axisMaxVals.append(Max);
                    axisMinVals.append(Min);
                }
            }
            else if(type==kIOHIDElementTypeInput_Button)
            {
//                qDebug() << "Button found";
                Buttons++;
                buttonElements.append(element);
            }
//            else
//                qDebug() << type<< " found";
        }
    }
    Valid =true;
}

QString CFStringRefToQString(CFStringRef str)
{
    if (!str)
        return QString();

    CFIndex length = CFStringGetLength(str);
    if (length == 0)
        return QString();

    QString string(length, Qt::Uninitialized);
    CFStringGetCharacters(str, CFRangeMake(0, length), reinterpret_cast<UniChar *>
        (const_cast<QChar *>(string.unicode())));
    return string;
}

// function to create matching dictionary
CFMutableDictionaryRef hu_CreateDeviceMatchingDictionary(UInt32 inUsagePage, UInt32 inUsage)
{
    // create a dictionary to add usage page/usages to
    CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (result)
    {
        if (inUsagePage)
        {
            // Add key for device type to refine the matching dictionary.
            CFNumberRef pageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
            if (pageCFNumberRef)
            {
                CFDictionarySetValue(result,CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
                CFRelease(pageCFNumberRef);
                // note: the usage is only valid if the usage page is also defined
                if (inUsage)
                {
                    CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
                    if (usageCFNumberRef)
                    {
                        CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
                        CFRelease(usageCFNumberRef);
                    }
                    else
                        fprintf(stderr, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
                }
            }
            else
                fprintf(stderr, "%s: CFNumberCreate(usage page) failed.", __PRETTY_FUNCTION__);
        }
    } else
        fprintf(stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
    return result;
}   // hu_CreateDeviceMatchingDictionary

void QGameControllerPrivate::readGameController()
{
    Q_Q(QGameController);
    IOReturn result;
    IOHIDValueRef HIDValue;
    for (uint axidID = 0; axidID<Axis; axidID++)
    {
        Q_ASSERT((int)axidID<axisElements.count());
        Q_ASSERT((int)axidID<axisMinVals.count());
        Q_ASSERT((int)axidID<axisMaxVals.count());
        IOHIDElementRef element = axisElements.at(axidID);
        result= IOHIDDeviceGetValue(device, element, &HIDValue);
        int min = axisMinVals.at(axidID);
        int max = axisMaxVals.at(axidID);
        if (CFGetTypeID(HIDValue) == IOHIDValueGetTypeID()) {
            int ivalue = IOHIDValueGetIntegerValue(HIDValue);
            if (IOHIDElementGetUsage(element)!=kHIDUsage_GD_Hatswitch)
            {
                //Treat as ordinary axis.
                if (ivalue!=AxisValues.value(axidID))
                {
                    AxisValues.insert(axidID,ivalue);
                    float value = ((float)(ivalue-min)/(float)(max-min) -.5 )*2.0;
//                    qDebug() << ID << "Axis value" << axidID << "changed to " << ivalue << value;
                    QGameControllerAxisEvent* event=new QGameControllerAxisEvent(ID, axidID, value);
                    emit(q->gameControllerAxisEvent((QGameControllerAxisEvent*)event));
                }
            }else{
                //This is a POV hat.
                float valueX=0;
                float valueY=0;
                if (ivalue<min || ivalue>max)
                {
//                    qDebug("POV is centred %i.", ivalue);
                }
                else
                {
                    float povrads = (float)(ivalue-1)/8*2*3.141593;
//                    qDebug("POV now %i (%f rads).", ivalue, povrads);
                    valueX=sin(povrads);
                    valueY=cos(povrads);
                    //Dissable these lines to get pov on circle.
                    valueX=qRound(valueX);
                    valueY=qRound(valueY);
                }
                if (valueX!=AxisValues.value(axidID))
                {
                    AxisValues.insert(axidID,valueX);
                    QGameControllerAxisEvent *event=new QGameControllerAxisEvent(ID, axidID, valueX);
//                    qDebug("Axis %i moved to %f.", axidID, valueX);
                    emit(q->gameControllerAxisEvent(event));
                }if (valueY!=AxisValues.value(axidID+1))
                {
                    AxisValues.insert(axidID+1,valueY);
                    QGameControllerAxisEvent *event=new QGameControllerAxisEvent(ID, axidID+1, valueY);
//                    qDebug("Axis %i moved to %f.", axidID+1, valueY);
                    emit(q->gameControllerAxisEvent(event));
                }
                axidID++; //We have dealt with 2 axis in one go.
            }
        }
    }
    for (uint i = 0; i<Buttons; i++)
    {
        Q_ASSERT((int)i<buttonElements.count());
        result= IOHIDDeviceGetValue(device, buttonElements.at(i), &HIDValue);
        if (CFGetTypeID(HIDValue) == IOHIDValueGetTypeID()) {
            bool pressed = IOHIDValueGetIntegerValue(HIDValue);
            if (ButtonValues.value(i)!=pressed)
            {
//                qDebug() << ID << "Button value" << i << "Changed to " << IOHIDValueGetIntegerValue(HIDValue);
                ButtonValues.insert(i,pressed);
                QGameControllerButtonEvent* event=new QGameControllerButtonEvent(ID, i, pressed);
                emit(q->gameControllerButtonEvent((QGameControllerButtonEvent*)event));
            }
        }
    }
}
