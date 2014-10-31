#include "widget.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    this->setWindowTitle("QJoystick Test");
    QVBoxLayout *layout = new QVBoxLayout(this);
    QJoystick *joystick;
    for (int i = 0; i < 10; i++)
    {
        joystick = new QJoystick(i, this);
        if (joystick->isValid())
        {
            connect(joystick, SIGNAL(JoystickAxisEvent(QJoystickAxisEvent*)), this, SLOT(handleQJoystickAxisEvent(QJoystickAxisEvent*)));
            connect(joystick, SIGNAL(JoystickButtonEvent(QJoystickButtonEvent*)), this, SLOT(handleQJoystickButtonEvent(QJoystickButtonEvent*)));
            QList<QProgressBar*> bars;
            QList<QLabel*> buttonLabels;

            QLabel *label = new QLabel(joystick->name(), this);
            layout->addWidget(label);
            QHBoxLayout *buttonLayout = new QHBoxLayout();
            for (int i = 0; i < joystick->buttonCount(); i++)
            {
                QLabel *label = new QLabel(QString ("%1: <b><font color=grey>U</font></b>").arg(i), this);
                label->setMargin(2);
                buttonLabels.append(label);
                buttonLayout->addWidget(label);
            }
            layout->addItem(buttonLayout);
            for (int i = 0; i < joystick->axisCount(); i++)
            {
                QLabel *label = new QLabel(QString ("Axis %1: ").arg(i), this);
                QProgressBar *bar = new QProgressBar(this);
                bar->setMinimum(-1000);
                bar->setMaximum(1000);
                bar->setValue(0);
                bars.append(bar);
                QHBoxLayout *hlayout = new QHBoxLayout();
                hlayout->addWidget(label);
                hlayout->addWidget(bar);
                layout->addItem(hlayout);
            }

            buttonLabelsMap.insert(i, buttonLabels);
            barsMap.insert(i, bars);

            QTimer *timer = new QTimer(this);
            timer->setInterval(15);
            connect(timer, SIGNAL(timeout()), joystick, SLOT(readJoystick()));
            timer->start();
        }
        else
            delete joystick;
     }
}

void Widget::handleQJoystickAxisEvent(QJoystickAxisEvent* event)
{
//    qDebug("handleQJoystickAxisEvent");
    uint axis = event->axis();
    QList<QProgressBar*> bars = barsMap.value(event->joystickId());
    Q_ASSERT(axis < bars.count());
    QProgressBar *bar = bars.at(axis);
    bar->setValue(event->value()*1000);
}


void Widget::handleQJoystickButtonEvent(QJoystickButtonEvent* event)
{
//    qDebug("handleQJoystickButtonEvent");
    uint button = event->button();
    QList<QLabel*> buttonLabels = buttonLabelsMap.value(event->joystickId());
    Q_ASSERT(button < buttonLabels.count());
    QLabel *label = buttonLabels.at(button);
    if (event->pressed())
        label->setText(QString ("%1: <b><font color=green>D</font></b>").arg(button));
    else
        label->setText(QString ("%1: <b><font color=grey>U</font></b>").arg(button));
}


Widget::~Widget()
{

}
