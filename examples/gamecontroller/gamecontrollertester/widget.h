#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtGameController/QGameController>
#include <QSlider>
#include <QLabel>
#include <QMap>

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
private:
    QMap<uint, QList<QSlider*> > slidersMap;
    QMap<uint, QList<QLabel*> > buttonLabelsMap;
private slots:
    void handleQGameControllerAxisEvent(QGameControllerAxisEvent *event);
    void handleQGameControllerButtonEvent(QGameControllerButtonEvent *event);
};

#endif // WIDGET_H
