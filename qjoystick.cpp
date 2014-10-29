#include "qjoystick.h"
#include <QDebug>

QJoystick::QJoystick(uint id, QObject *parent) :
    QObject(parent)
{
    ID=id;
    Valid=false;
    Axis=0;
    Buttons=0;
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
    Name=name_of_stick;
    qDebug("Joystick: \"%s\" has %i axis and %i buttons", name_of_stick, number_of_axes, number_of_buttons);
    readJoystick();
#endif
}

void QJoystick::readJoystick()
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
}

#ifdef Q_OS_LINUX
void QJoystick::process_event(js_event e)
{
    QJoystickEvent *event = NULL;
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
        event=new QJoystickButtonEvent(ID, e.number, value);
        ButtonValues.insert(e.number, value);
        emit(JoystickButtonEvent((QJoystickButtonEvent*)event));
    } else if (e.type & JS_EVENT_AXIS) {
        float Value;
        if (value<0)
            Value = (float)value/32768.0;
        else
            Value = (float)value/32767.0;
        AxisValues.insert(e.number, Value);
        event=new QJoystickAxisEvent(ID, e.number, Value);
//        qDebug("Axis %i moved to %f.", e.number , Value);
        emit(JoystickAxisEvent((QJoystickAxisEvent*)event));
    }
    emit(JoystickEvent(event));
    return;
}
#endif

QJoystickButtonEvent::QJoystickButtonEvent(uint joystickId, uint button, bool pressed)
{
    JoystickId=joystickId;
    Button=button;
    Pressed=pressed;
}

QJoystickAxisEvent::QJoystickAxisEvent(uint joystickId, uint axis, float value)
{
    JoystickId=joystickId;
    Axis=axis;
    Value=value;
}
