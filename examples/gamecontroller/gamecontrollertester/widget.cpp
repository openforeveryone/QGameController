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

#include "widget.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    this->setWindowTitle("Game Controller Tester");
    QVBoxLayout *layout = new QVBoxLayout(this);
    QGameController *gameController;
    for (int i = 0; i < 10; i++)
    {
        gameController = new QGameController(i, this);
        if (gameController->isValid())
        {
            connect(gameController, SIGNAL(gameControllerAxisEvent(QGameControllerAxisEvent*)), this, SLOT(handleQGameControllerAxisEvent(QGameControllerAxisEvent*)));
            connect(gameController, SIGNAL(gameControllerButtonEvent(QGameControllerButtonEvent*)), this, SLOT(handleQGameControllerButtonEvent(QGameControllerButtonEvent*)));
            QList<QSlider*> sliders;
            QList<QLabel*> buttonLabels;

            QLabel *label = new QLabel(gameController->description(), this);
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
                QSlider *slider = new QSlider(Qt::Horizontal, this);
                slider->setEnabled(false);
                slider->setTickPosition(QSlider::TicksBothSides);
                slider->setTickInterval(1000/4);
                slider->setMinimum(-1000);
                slider->setMaximum(1000);
                slider->setValue(0);
                sliders.append(slider);
                QHBoxLayout *hlayout = new QHBoxLayout();
                hlayout->addWidget(label);
                hlayout->addWidget(slider);
                layout->addItem(hlayout);
            }

            buttonLabelsMap.insert(i, buttonLabels);
            slidersMap.insert(i, sliders);

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
    QList<QSlider*> sliders = slidersMap.value(event->controllerId());
    Q_ASSERT(axis < sliders.count());
    QSlider *bar = sliders.at(axis);
    bar->setValue(event->value()*1000);
    delete event;   //QGameControllerEvents unlike QEvents are not deleted automatically.
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
    delete event;   //QGameControllerEvents unlike QEvents are not deleted automatically.
}


Widget::~Widget()
{

}
