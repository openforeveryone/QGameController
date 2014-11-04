#ifndef QGameController_H
#define QGameController_H

#include <QObject>
#include "qgamecontroller-global.h"

QT_BEGIN_NAMESPACE
class QGameControllerPrivate;

class QGAMECONTROLLER_EXPORT QGameControllerEvent
{
public:
    bool controllerId();
protected:
    uint ControllerId;
};

class QGAMECONTROLLER_EXPORT QGameControllerButtonEvent : public QGameControllerEvent
{
public:
    QGameControllerButtonEvent(uint controllerId, uint button, bool pressed);
    uint button();
    bool pressed();
private:
    uint Button;
    bool Pressed;
};

class QGAMECONTROLLER_EXPORT QGameControllerAxisEvent : public QGameControllerEvent
{
public:
    QGameControllerAxisEvent(uint controllerId, uint axis, float value);
    uint axis();
    float value();
private:
    uint Axis;
    float Value;
};

class QGAMECONTROLLER_EXPORT QGameController : public QObject
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
