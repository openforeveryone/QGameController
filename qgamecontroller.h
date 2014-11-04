#ifndef QGameController_H
#define QGameController_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QGameControllerPrivate;

class QGameControllerEvent
{
public:
    bool controllerId() {return ControllerId;}
protected:
    uint ControllerId;
};

class QGameControllerButtonEvent : public QGameControllerEvent
{
public:
    QGameControllerButtonEvent(uint controllerId, uint button, bool pressed);
    uint button() {return Button;}
    bool pressed() {return Pressed;}
private:
    uint Button;
    bool Pressed;
};

class QGameControllerAxisEvent : public QGameControllerEvent
{
public:
    QGameControllerAxisEvent(uint controllerId, uint axis, float value);
    uint axis() {return Axis;}
    float value() {return Value;}
private:
    uint Axis;
    float Value;
};

class QGameController : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGameController)
public:
    explicit QGameController(uint id = 0, QObject *parent = 0);
    uint axisCount();
    uint buttonCount();
    float axisValue(uint axis);
    bool buttonValue(uint button);
    QString name();
    uint id();
    bool isValid();
signals:

public slots:    
    void readGameController();
signals:
    void gameControllerEvent(QGameControllerEvent *event);
    void gameControllerButtonEvent(QGameControllerButtonEvent *event);
    void gameControllerAxisEvent(QGameControllerAxisEvent *event);
private:
    QGameControllerPrivate* const d_ptr;
    Q_DISABLE_COPY(QGameController)
};

QT_END_NAMESPACE

#endif // QGameController_H
