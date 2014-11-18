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

#ifndef QGAMECONTROLLER_MAC_P_H
#define QGAMECONTROLLER_MAC_P_H

#include <QMap>

#include "qgamecontroller.h"
#include "qgamecontroller_p.h"

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/HID/IOHIDKeys.h>
CFMutableDictionaryRef hu_CreateDeviceMatchingDictionary(UInt32 inUsagePage, UInt32 inUsage);
QString CFStringRefToQString(CFStringRef str);

QT_BEGIN_NAMESPACE

class QGameControllerPrivate
{
    Q_DECLARE_PUBLIC(QGameController)
public:
    explicit QGameControllerPrivate(uint id, QGameController *q);
    QGameController * const q_ptr;
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

QGAMECONTROLLER_MAC_P_H

#endif // QGAMECONTROLLER_P_H
