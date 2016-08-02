#include <QDebug>
#include <QTimer>

#include "mainwindow.h"
#include "seriallayer.h"

MainWindow::MainWindow(QString port, uint baud, QWidget *parent) :
    QMainWindow(parent),
    ser(new SerialLayer(port, baud, parent))
{
    //Simple test timer
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::checkCommand);
    timer->start(100);

    show();
};

void MainWindow::checkCommand()
{
    while (ser->commandAvailable())
        qDebug() << ser->popCommand();
    ser->pushCommand("ok0!\n");
    ser->add("ok1!\n");
    ser->add("ok2!\n");
    ser->push();
}

MainWindow::~MainWindow()
{
}