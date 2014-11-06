#ifndef QGameController_H
#define QGameController_H

#include <QObject>
#include "qgamecontroller-global.h"

QT_BEGIN_NAMESPACE
class QGameControllerPrivate;
class QGameControllerEventPrivate;
class QGameControllerAxisEventPrivate;
class QGameControllerButtonEventPrivate;

class QGAMECONTROLLER_EXPORT QGameControllerEvent
{
    Q_DECLARE_PRIVATE(QGameControllerEvent)
public:
    QGameControllerEvent(uint controllerId);
    bool controllerId();
    ~QGameControllerEvent();
protected:
    QGameControllerEvent(uint controllerId, QGameControllerEventPrivate &d);
    QGameControllerEventPrivate* const d_ptr;
};

class QGAMECONTROLLER_EXPORT QGameControllerButtonEvent : public QGameControllerEvent
{
    Q_DECLARE_PRIVATE(QGameControllerButtonEvent)
public:
    QGameControllerButtonEvent(uint controllerId, uint button, bool pressed);
    uint button();
    bool pressed();
};

class QGAMECONTROLLER_EXPORT QGameControllerAxisEvent : public QGameControllerEvent
{
    Q_DECLARE_PRIVATE(QGameControllerAxisEvent)
public:
    QGameControllerAxisEvent(uint controllerId, uint axis, float value);
    uint axis();
    float value();
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
    QString description();
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
