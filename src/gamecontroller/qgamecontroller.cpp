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
#include "qgamecontroller_p.h"

#include <QDebug>

#ifdef Q_OS_WIN
QGameControllerPrivate *joysticktoenume; //TODO: Dispense with this global pointer.
LPDIRECTINPUT8          g_pDI = nullptr;
#endif
#ifdef Q_OS_MAC
IOHIDManagerRef hidManager=NULL;
#endif

QGameControllerEvent::QGameControllerEvent(uint controllerId)
    :d_ptr(new QGameControllerEventPrivate(this))
{
    Q_D(QGameControllerEvent);
    d->ControllerId=controllerId;
}

QGameControllerEvent::QGameControllerEvent(uint controllerId, QGameControllerEventPrivate &d)
    :d_ptr(&d)
{
    d.ControllerId=controllerId;
}

bool QGameControllerEvent::controllerId()
{
    Q_D(QGameControllerEvent);
    return d->ControllerId;
}

QGameControllerEvent::~QGameControllerEvent()
{
    delete d_ptr;
}

QGameControllerButtonEvent::QGameControllerButtonEvent(uint controllerId, uint button, bool pressed)
    : QGameControllerEvent(controllerId, *new QGameControllerButtonEventPrivate(this))
{
    Q_D(QGameControllerButtonEvent);
    d->Button=button;
    d->Pressed=pressed;
}

uint QGameControllerButtonEvent::button()
{
    Q_D(QGameControllerButtonEvent);
    return d->Button;
}

bool QGameControllerButtonEvent::pressed()
{
    Q_D(QGameControllerButtonEvent);
    return d->Pressed;
}

QGameControllerAxisEvent::QGameControllerAxisEvent(uint controllerId, uint axis, float value)
    : QGameControllerEvent(controllerId, *new QGameControllerAxisEventPrivate(this))
{
    Q_D(QGameControllerAxisEvent);
    d->Axis=axis;
    d->Value=value;
}

uint QGameControllerAxisEvent::axis()
{
    Q_D(QGameControllerAxisEvent);
    return d->Axis;
}

float QGameControllerAxisEvent::value()
{
    Q_D(QGameControllerAxisEvent);
    return d->Value;
}

QGameController::QGameController(uint id, QObject *parent) :
    QObject(parent), d_ptr(new QGameControllerPrivate(id, this))
{

}

uint QGameController::axisCount()
{
    Q_D(QGameController);
    return d->Axis;
}

uint QGameController::buttonCount()
{
    Q_D(QGameController);
    return d->Buttons;
}

float QGameController::axisValue(uint axis)
{
    Q_D(QGameController);
    return d->AxisValues.value(axis);
}

bool QGameController::buttonValue(uint button)
{
    Q_D(QGameController);
    return d->ButtonValues.value(button);
}

QString QGameController::description()
{
    Q_D(QGameController);
    return d->Description;
}

uint QGameController::id()
{
    Q_D(QGameController);
    return d->ID;
}

bool QGameController::isValid()
{
    Q_D(QGameController);
    return d->Valid;
}

void QGameController::readGameController()
{
    Q_D(QGameController);
    d->readGameController();
}


QGameControllerPrivate::QGameControllerPrivate(uint id, QGameController *q) :
    q_ptr(q)
{
    ID=id;
    Valid=false;
    Axis=0;
    Buttons=0;
//    qDebug("QGameController::QGameController(%i)", ID);
#ifdef Q_OS_LINUX
    char number_of_axes=0;
    char number_of_buttons=0;
    QString filename = QString("/dev/input/js%1").arg(QString::number(id));
    qDebug() << "Opening" << filename.toUtf8().data();
    if( ( fd = open( filename.toUtf8().data() , O_NONBLOCK)) == -1 )
    {
        qDebug( "Couldn't open joystick\n" );
        return;
    }
    Valid=true;
    ioctl (fd, JSIOCGAXES, &number_of_axes);
    ioctl (fd, JSIOCGBUTTONS, &number_of_buttons);
    Axis=number_of_axes;
    Buttons=number_of_buttons;
    char name_of_stick[80];
    ioctl (fd, JSIOCGNAME(80), &name_of_stick);
    Description=name_of_stick;
    qDebug("Joystick: \"%s\" has %i axis and %i buttons", name_of_stick, number_of_axes, number_of_buttons);
    readGameController();
#endif

#ifdef Q_OS_WIN
    HRESULT hr;
    g_pJoystick = nullptr;

    // Register with DirectInput to get a pointer to an IDirectInput
    if (g_pDI==nullptr)
    {
        qDebug() << "Setting up directinput";
        if( FAILED( hr = DirectInput8Create( GetModuleHandle( nullptr ), DIRECTINPUT_VERSION,
                                             IID_IDirectInput8, ( VOID** )&g_pDI, nullptr ) ) )
            //            return hr;
            qDebug() << "Error 1";
    }

    DIJOYCONFIG PreferredJoyCfg = {0};
    DI_ENUM_CONTEXT enumContext;
    enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
    enumContext.bPreferredJoyCfgValid = false;

    IDirectInputJoyConfig8* pJoyConfig = nullptr;
    if( FAILED( hr = g_pDI->QueryInterface( IID_IDirectInputJoyConfig8, ( void** )&pJoyConfig ) ) )
        //            return hr;
        qDebug() << "Error 2";
    PreferredJoyCfg.dwSize = sizeof( PreferredJoyCfg );
    if( SUCCEEDED( pJoyConfig->GetConfig( 0, &PreferredJoyCfg, DIJC_GUIDINSTANCE ) ) ) // This function is expected to fail if no joystick is attached
        enumContext.bPreferredJoyCfgValid = true;
    else
        qDebug() << "bPreferredJoyCfgValid == false";
    SAFE_RELEASE( pJoyConfig );

    joysticktoenume=this;
    enumCounter=0;
    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                         EnumJoysticksCallback,
                                         &enumContext, DIEDFL_ATTACHEDONLY ) ) )
        qDebug() << "Error 3";
    // Make sure we got a joystick
    if( !g_pJoystick )
    {
//        qDebug() << "Joystick not found";
        return;
    }

//    qDebug() << "Setting data format";
    if( FAILED( hr = g_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
        qDebug() << "Error 4";

    if( FAILED( hr = g_pJoystick->EnumObjects( EnumObjectsCallback,
                                               ( VOID* )NULL, DIDFT_ALL ) ) )
    {
        qDebug() << "Error 6";
        return;
    }

    DIDEVICEINSTANCE joystickinfo;
    joystickinfo.dwSize=sizeof(joystickinfo);
    if( FAILED( hr = g_pJoystick->GetDeviceInfo(&joystickinfo)))
        qDebug() << "Error 6";
    if (hr==DIERR_INVALIDPARAM)
        qDebug() << "DIERR_INVALIDPARAM";

    Description = QString::fromUtf16((ushort*)&(joystickinfo.tszProductName));
//    qDebug() << description() << QString::fromUtf16((ushort*)&(joystickinfo.tszInstaxnceName));
    Valid=true;

#endif

#ifdef Q_OS_MAC
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
#endif
}


#ifdef Q_OS_MAC
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
#endif

#ifdef Q_OS_WIN
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
//    qDebug() << "EnumJoysticksCallback()" << joysticktoenume->enumCounter;
    if (joysticktoenume->enumCounter!=joysticktoenume->ID)
    {
//        qDebug() << "Skiping" << joysticktoenume->enumCounter;
        joysticktoenume->enumCounter++;
        return DIENUM_CONTINUE;
    }

//    auto pEnumContext = reinterpret_cast<DI_ENUM_CONTEXT*>( pContext );

//    if( pEnumContext->bPreferredJoyCfgValid &&
//        !IsEqualGUID( pdidInstance->guidInstance, pEnumContext->pPreferredJoyCfg->guidInstance ) )
//    {
//            //Maybe we should mark this as prefered in QGameController
//    }

    // Get an interface to this joystick.
    HRESULT hr;
    hr = g_pDI->CreateDevice( pdidInstance->guidInstance, &(joysticktoenume->g_pJoystick), nullptr );

    if( FAILED( hr ) )
    {
        qDebug() << "CreateDevice failed";
        return DIENUM_CONTINUE;
}
//    qDebug() << "EnumJoysticksCallback() Joystick found";
    return DIENUM_STOP;
}

BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext )
{
    //Set the range for axis
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg;
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin = -1000;
        diprg.lMax = +1000;
        if( FAILED( joysticktoenume->g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
            return DIENUM_STOP;
    }
//    qDebug() << "JSObject Found";

    // Set the UI to reflect what objects the joystick supports
    if( pdidoi->guidType == GUID_XAxis )
    {
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
//        qDebug() << "Axis found";
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_YAxis )
    {
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
//        qDebug() << "Axis found";
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_ZAxis )
    {
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
//        qDebug() << "Axis found";
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_RxAxis )
    {
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
//        qDebug() << "Axis found";
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_RyAxis )
    {
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
//        qDebug() << "Axis found";
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_RzAxis )
    {
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
//        qDebug() << "Axis found";
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_Slider )
    {
//        qDebug() << "Slider found";
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_POV )
    {
//        qDebug() << "POV found";
        //We add two axis to represent this pov
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
        joysticktoenume->Axis++;
        joysticktoenume->DIaxisGIIDs.push_back(pdidoi->guidType);
        joysticktoenume->Axis++;
    }
    if( pdidoi->guidType == GUID_Button )
    {
//        qDebug() << "Button found";
        joysticktoenume->Buttons++;
    }
    if( pdidoi->guidType == GUID_Key )
    {
//        qDebug() << "Key found";
    }
    if( pdidoi->guidType == GUID_Unknown )
    {
//        qDebug() << "Unknown object found";
    }

    return DIENUM_CONTINUE;
}
#endif


void QGameControllerPrivate::readGameController()
{

#ifdef Q_OS_LINUX
    if (!Valid)
        return;
//    qDebug() << "readJoystick";
    struct js_event e;
    while (read (fd, &e, sizeof(e)) > 0) {
        process_event (e);
    }
    /* EAGAIN is returned when the queue is empty */
    if (errno != EAGAIN) {
        qDebug() << "Error";
        Valid=false;
    }
//    else
//        qDebug() << "No event";
#endif

#ifdef Q_OS_WIN    
    Q_Q(QGameController);
    HRESULT hr;
    DIJOYSTATE2 js;           // DInput joystick state

    if( !g_pJoystick )
        return;

    hr = g_pJoystick->Poll();
    if( FAILED( hr ) )
    {
        hr = g_pJoystick->Acquire();
        while( hr == DIERR_INPUTLOST )
            hr = g_pJoystick->Acquire();
        return;
    }

    if( FAILED( hr = g_pJoystick->GetDeviceState( sizeof( DIJOYSTATE2 ), &js ) ) )
        return;

    // Axes
    float value = 0;
    int slider = 0;
    int pov = 0;
    for (uint axisid = 0; axisid<Axis; axisid++)
    {
//        qDebug("Reading joystick %i axis %i of %i, %i", id(), axisid, Axis, axisGIIDs.count());
        Q_ASSERT(axisid<DIaxisGIIDs.count());
        GUID guidForThisAxis = DIaxisGIIDs.at(axisid);
        if (guidForThisAxis==GUID_XAxis)
            value = (float)(js.lX);
        else if(guidForThisAxis==GUID_YAxis)
            value = (float)(js.lY);
        else if(guidForThisAxis==GUID_ZAxis)
            value = (float)(js.lZ);
        else if(guidForThisAxis==GUID_RxAxis)
            value = (float)(js.lRx);
        else if(guidForThisAxis==GUID_RyAxis)
            value = (float)(js.lRy);
        else if(guidForThisAxis==GUID_RzAxis)
            value = (float)(js.lRz);
        else if(guidForThisAxis==GUID_Slider)
        {
            value = (float)(js.rglSlider[slider]);
            slider++;
        }
        if(guidForThisAxis==GUID_POV)
        {
            DWORD povvalue = js.rgdwPOV[pov];
            float valueX=0;
            float valueY=0;
            if (povvalue==-1 || (LOWORD(povvalue) == 0xFFFF))
            {
//                qDebug("POV %i is centred %i.", pov, povvalue);
            }
            else
            {
                int povdegs = (int)povvalue/100;
                float povrads = (float)povdegs/360.0*2*3.141593;
//                qDebug("POV %i now %i (%i deg, %f rads).", pov, povvalue, povdegs, povrads);
                valueX=sin(povrads);
                valueY=cos(povrads);
                //Dissable these lines to get pov on circle.
                valueX=qRound(valueX);
                valueY=qRound(valueY);
            }
            if (valueX!=AxisValues.value(axisid))
            {
                AxisValues.insert(axisid,valueX);
                QGameControllerAxisEvent *event=new QGameControllerAxisEvent(ID, axisid, valueX);
//                qDebug("Axis %i moved to %f.", axisid, valueX);
                emit(q->gameControllerAxisEvent(event));
            }if (valueY!=AxisValues.value(axisid+1))
            {
                AxisValues.insert(axisid+1,valueY);
                QGameControllerAxisEvent *event=new QGameControllerAxisEvent(ID, axisid+1, valueY);
//                qDebug("Axis %i moved to %f.", axisid+1, valueY);
                emit(q->gameControllerAxisEvent(event));
            }
            axisid++; //We have dealt with 2 axis in one go.
            pov++;
        }else
        {
            value=value/1000.0;
            if (value!=AxisValues.value(axisid))
            {
                AxisValues.insert(axisid,value);
                QGameControllerAxisEvent *event=new QGameControllerAxisEvent(ID, axisid, value);
                //            qDebug("Axis %i moved to %f.", axisid, value);
                emit(q->gameControllerAxisEvent(event));
            }
        }
    }
    //Buttons
    for( int i = 0; i < Buttons; i++ )
    {
        if( js.rgbButtons[i] & 0x80 )
        {
           if (ButtonValues.value(i)!=true)
           {
               ButtonValues.insert(i,true);
//               qDebug("Button %i pressed.", i);
               QGameControllerButtonEvent* event=new QGameControllerButtonEvent(ID, i, true);
               emit(q->gameControllerButtonEvent((QGameControllerButtonEvent*)event));
           }
        }else
        {
            if (ButtonValues.value(i)!=false)
            {
                ButtonValues.insert(i,false);
//                qDebug("Button %i released.", i);
                QGameControllerButtonEvent* event=new QGameControllerButtonEvent(ID, i, false);
                emit(q->gameControllerButtonEvent((QGameControllerButtonEvent*)event));
            }
        }
    }
#endif

#ifdef Q_OS_MAC
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
#endif
}

#ifdef Q_OS_LINUX
void QGameControllerPrivate::process_event(js_event e)
{
    Q_Q(QGameController);
    QGameControllerEvent *event = NULL;
    if (e.type & JS_EVENT_INIT)
    {
//        qDebug() << "process_event" << "event was a JS_EVENT_INIT" << e.number << e.value << e.type;
    }
    qint16 value = e.value;
//    qDebug() << "process_event" << e.number << value << e.type;
    if (e.type & JS_EVENT_BUTTON)
    {
        if (e.value==1)
        {
//            qDebug("Button %i pressed.", e.number);
        }
        else
        {
//            qDebug("Button %i released.", e.number);
        }
        event=new QGameControllerButtonEvent(ID, e.number, value);
        ButtonValues.insert(e.number, value);
        emit(q->gameControllerButtonEvent((QGameControllerButtonEvent*)event));
    } else if (e.type & JS_EVENT_AXIS) {
        float Value;
        if (value<0)
            Value = (float)value/32768.0;
        else
            Value = (float)value/32767.0;
        AxisValues.insert(e.number, Value);
        event=new QGameControllerAxisEvent(ID, e.number, Value);
//        qDebug("Axis %i moved to %f.", e.number , Value);
        emit(q->gameControllerAxisEvent((QGameControllerAxisEvent*)event));
    }
    emit(q->gameControllerEvent(event));
    return;
}
#endif

