#ifndef QGAMECONTROLLER_P_H
#define QGAMECONTROLLER_P_H

#include <QMap>

#include "qgamecontroller.h"

#ifdef Q_OS_LINUX
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
//#include <sstream>
#include <errno.h>
#include <linux/joystick.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <dinputd.h>
#include <QList>
struct DI_ENUM_CONTEXT
{
    DIJOYCONFIG* pPreferredJoyCfg;
    bool bPreferredJoyCfgValid;
};
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
BOOL CALLBACK    EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
#endif
#ifdef Q_OS_MAC
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/HID/IOHIDKeys.h>
CFMutableDictionaryRef hu_CreateDeviceMatchingDictionary(UInt32 inUsagePage, UInt32 inUsage);
QString CFStringRefToQString(CFStringRef str);
#endif

QT_BEGIN_NAMESPACE

class QGameControllerPrivate
{
    Q_DECLARE_PUBLIC(QGameController)
public:
    explicit QGameControllerPrivate(uint id, QGameController *q);
    QGameController * const q_ptr;
protected:
#ifdef Q_OS_LINUX
    void process_event(js_event e);
#endif
public:
    int fd;
    QString Name;
    uint ID;
    bool Valid;
    QMap<uint, float> AxisValues;
    QMap<uint, bool> ButtonValues;

    QGameControllerPrivate* d;
#ifdef Q_OS_MAC
    IOHIDDeviceRef device;
    QList<IOHIDElementRef> axisElements;
    QList<uint> axisMaxVals;
    QList<uint> axisMinVals;
    QList<IOHIDElementRef> buttonElements;
#endif

#ifdef Q_OS_WIN
    LPDIRECTINPUTDEVICE8    g_pJoystick = nullptr;
    uint enumCounter;
    QList<GUID> DIaxisGIIDs;
#endif
    uint Axis;
    uint Buttons;

    void readGameController();
};

QT_END_NAMESPACE

#endif // QGAMECONTROLLER_P_H
