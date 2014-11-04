#include "widget.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    this->setWindowTitle("QGameController Test");
    QVBoxLayout *layout = new QVBoxLayout(this);
    QGameController *gameController;
    for (int i = 0; i < 10; i++)
    {
        gameController = new QGameController(i, this);
        if (gameController->isValid())
        {
            connect(gameController, SIGNAL(gameControllerAxisEvent(QGameControllerAxisEvent*)), this, SLOT(handleQGameControllerAxisEvent(QGameControllerAxisEvent*)));
            connect(gameController, SIGNAL(gameControllerButtonEvent(QGameControllerButtonEvent*)), this, SLOT(handleQGameControllerButtonEvent(QGameControllerButtonEvent*)));
            QList<QProgressBar*> bars;
            QList<QLabel*> buttonLabels;

            QLabel *label = new QLabel(gameController->name(), this);
            layout->addWidget(label);
            QHBoxLayout *buttonLayout = new QHBoxLayout();
            for (int i = 0; i < gameController->buttonCount(); i++)
            {
                QLabel *label = new QLabel(QString ("%1: <b><font color=grey>U</font></b>").arg(i), this);
                label->setMargin(2);
                buttonLabels.append(label);
                buttonLayout->addWidget(label);
            }
            layout->addItem(buttonLayout);
            for (int i = 0; i < gameController->axisCount(); i++)
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
            connect(timer, SIGNAL(timeout()), gameController, SLOT(readGameController()));
            timer->start();
        }
        else
        {
            delete gameController;
            break;
        }
     }
}

void Widget::handleQGameControllerAxisEvent(QGameControllerAxisEvent* event)
{
//    qDebug("handleQGameControllerAxisEvent");
    uint axis = event->axis();
    QList<QProgressBar*> bars = barsMap.value(event->controllerId());
    Q_ASSERT(axis < bars.count());
    QProgressBar *bar = bars.at(axis);
    bar->setValue(event->value()*1000);
}


void Widget::handleQGameControllerButtonEvent(QGameControllerButtonEvent* event)
{
//    qDebug("handleQGameControllerButtonEvent");
    uint button = event->button();
    QList<QLabel*> buttonLabels = buttonLabelsMap.value(event->controllerId());
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
