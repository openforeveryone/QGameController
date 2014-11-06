#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtGameController/QGameController>
#include <QProgressBar>
#include <QLabel>
#include <QMap>

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
private:
    QMap<uint, QList<QProgressBar*> > barsMap;
    QMap<uint, QList<QLabel*> > buttonLabelsMap;
private slots:
    void handleQGameControllerAxisEvent(QGameControllerAxisEvent *event);
    void handleQGameControllerButtonEvent(QGameControllerButtonEvent *event);
};

#endif // WIDGET_H
