#include "miniboardupdater.h"

#include <QDebug>


MiniBoardUpdater::MiniBoardUpdater(QObject *parent)
    : QThread(parent)
{
    connect(this, SIGNAL(update()), SerialHandler::instance(), SLOT(queryStatus()));
    //runTime.start();

}

void MiniBoardUpdater::run()
{

    qDebug() << "Update timer on";
    eventLoop();
    qDebug() << "Update timer off";
    m_run = true;
}

void MiniBoardUpdater::eventLoop()
{
    emit changeButtonColor("#169d06", true);
    while(m_run){
        //emit update();
        //if runtime has gone 1000 ms more
        emit update();
        msleep(1000);

    }
    emit changeButtonColor("#9d0606", false);

}


void MiniBoardUpdater::stop()
{
    m_run = false;
}

int MiniBoardUpdater::timeRunning()
{
    return runTime.elapsed();
}

