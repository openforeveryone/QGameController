#ifndef QJOYSTICK_H
#define QJOYSTICK_H

#include <QObject>
#include <QMap>

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

class QJoystickEvent
{
public:
    bool joystickId() {return JoystickId;}
protected:
    uint JoystickId;
};

class QJoystickButtonEvent : public QJoystickEvent
{
public:
    QJoystickButtonEvent(uint joystickId, uint button, bool pressed);
    uint button() {return Button;}
    bool pressed() {return Pressed;}
private:
    uint Button;
    bool Pressed;
};

class QJoystickAxisEvent : public QJoystickEvent
{
public:
    QJoystickAxisEvent(uint joystickId, uint axis, float value);
    uint axis() {return Axis;}
    float value() {return Value;}
private:
    uint Axis;
    float Value;
};

class QJoystick : public QObject
{
    Q_OBJECT
public:
    explicit QJoystick(uint id = 0, QObject *parent = 0);
    uint axisCount() {return Axis;}
    uint buttonCount() {return Buttons;}
    float axisValue(uint axis) {return AxisValues.value(axis);}
    bool buttonValue(uint button) {return ButtonValues.value(button);}
    QString name() {return Name;}
    uint id() {return ID;}
    bool isValid() {return Valid;}
signals:

public slots:    
    void readJoystick();
protected:
#ifdef Q_OS_LINUX
    void process_event(js_event e);
#endif
private:
    int fd;
    QString Name;
    uint ID;
    bool Valid;
    QMap<uint, float> AxisValues;
    QMap<uint, bool> ButtonValues;
public:
    //These should be private but EnumObjectsCallback needs access to them.
#ifdef Q_OS_WIN
    LPDIRECTINPUTDEVICE8    g_pJoystick = nullptr;
    uint enumCounter;
    QList<GUID> DIaxisGIIDs;
#endif
    uint Axis;
    uint Buttons;
signals:
    void JoystickEvent(QJoystickEvent *event);
    void JoystickButtonEvent(QJoystickButtonEvent *event);
    void JoystickAxisEvent(QJoystickAxisEvent *event);
};

#endif // QJOYSTICK_H
