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
#include "qgamecontroller_win_p.h"

#include <QDebug>

QGameControllerPrivate *joysticktoenume; //TODO: Dispense with this global pointer.
LPDIRECTINPUT8          g_pDI = nullptr;

QGameControllerPrivate::QGameControllerPrivate(uint id, QGameController *q) :
    q_ptr(q)
{
    ID=id;
    Valid=false;
    Axis=0;
    Buttons=0;
//    qDebug("QGameController::QGameController(%i)", ID);

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
}

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

void QGameControllerPrivate::readGameController()
{
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
}
