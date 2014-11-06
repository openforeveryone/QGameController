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

struct QGameControllerEventPrivate
{    
    Q_DECLARE_PUBLIC(QGameControllerEvent)
    QGameControllerEventPrivate(QGameControllerEvent *q) : q_ptr(q) { }
    QGameControllerEvent * const q_ptr;
    uint ControllerId;
};


struct QGameControllerButtonEventPrivate : public QGameControllerEventPrivate
{
    Q_DECLARE_PUBLIC(QGameControllerButtonEvent)
    QGameControllerButtonEventPrivate(QGameControllerEvent *q) : QGameControllerEventPrivate(q) { }
    uint Button;
    bool Pressed;
};

struct QGameControllerAxisEventPrivate : public QGameControllerEventPrivate
{
    Q_DECLARE_PUBLIC(QGameControllerAxisEvent)
    QGameControllerAxisEventPrivate(QGameControllerEvent *q) : QGameControllerEventPrivate(q) { }
    uint Axis;
    float Value;
};

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
    QString Description;
    uint ID;
    bool Valid;
    QMap<uint, float> AxisValues;
    QMap<uint, bool> ButtonValues;
#ifdef Q_OS_MAC
    IOHIDDeviceRef device;
    QList<IOHIDElementRef> axisElements;
    QList<uint> axisMaxVals;
    QList<uint> axisMinVals;
    QList<IOHIDElementRef> buttonElements;
#endif

#ifdef Q_OS_WIN
    LPDIRECTINPUTDEVICE8    g_pJoystick;
    uint enumCounter;
    QList<GUID> DIaxisGIIDs;
#endif
    uint Axis;
    uint Buttons;

    void readGameController();
};

QT_END_NAMESPACE

#endif // QGAMECONTROLLER_P_H
