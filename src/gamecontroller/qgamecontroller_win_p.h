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

#ifndef QGAMECONTROLLER_WIN_P_H
#define QGAMECONTROLLER_WIN_P_H

#include <QMap>

#include "qgamecontroller.h"
#include "qgamecontroller_p.h"

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

class QGameControllerPrivate
{
    Q_DECLARE_PUBLIC(QGameController)
public:
    explicit QGameControllerPrivate(uint id, QGameController *q);
    QGameController * const q_ptr;
protected:
public:
    int fd;
    QString Description;
    uint ID;
    bool Valid;
    QMap<uint, float> AxisValues;
    QMap<uint, bool> ButtonValues;
    LPDIRECTINPUTDEVICE8    g_pJoystick;
    uint enumCounter;
    QList<GUID> DIaxisGIIDs;
    uint Axis;
    uint Buttons;

    void readGameController();
};

QT_END_NAMESPACE

#endif // QGAMECONTROLLER_WIN_P_H
