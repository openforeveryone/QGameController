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
    uint Axis;
    uint Buttons;
    bool Valid;
    QMap<uint, float> AxisValues;
    QMap<uint, bool> ButtonValues;
signals:
    void JoystickEvent(QJoystickEvent *event);
    void JoystickButtonEvent(QJoystickButtonEvent *event);
    void JoystickAxisEvent(QJoystickAxisEvent *event);
};

#endif // QJOYSTICK_H
