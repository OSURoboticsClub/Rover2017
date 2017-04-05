#include "threadarray.h"

#include <QDebug>

ThreadArray::ThreadArray(){
    numThreads = 0;
    clearing = false;
    threadhead = NULL;
}


void ThreadArray::push(QThread *m_thread, bool startImmediately)
{
    if (!(m_thread->isRunning())){
        threadnode *temp = new threadnode;
        temp->n_thread = m_thread;
        temp->next = new threadnode;
        temp->nextBlank = true;
        if (threadhead == NULL) {

            threadhead = temp;

        }
        else {
            threadnode* last = threadhead;
            for(int i=0;i<numThreads;i++) {
                last = last->next;
            }
            last->next = temp;
            last->nextBlank = false;
        }

        if (startImmediately){
            m_thread->start();
        }
        connect(this, SIGNAL(closeThreads()), m_thread, SLOT(stop()));
        numThreads++;

    }
}

void ThreadArray::m_push(QThread *m_thread, bool startImmediately){
    if (!(m_thread->isRunning())){
       threadList.append(m_thread);
       if (startImmediately)
           m_thread->start();
       connect(this, SIGNAL(closeThreads()), m_thread, SLOT(stop()));
       numThreads++;
    }
}

void ThreadArray::convertToArray()
{
    struct threadnode *temp = threadhead;
    threadArray = new QThread *[numThreads];
    for (int i = 0;i < numThreads;i++){
        threadArray[i] = temp->n_thread;
        if (!temp->nextBlank) {
            temp = temp->next;
        }
    }
}


bool ThreadArray :: clear()
{
    if (!clearing){
        clearing = true;
        emit closeThreads();
        bool allThreadsKilled = false;
        int numThreadsRunning = 0;
        bool terminateHasRun = false;

        QTime startTime;
        startTime.start();

        convertToArray();
        while (!allThreadsKilled)
        {
            for (int i = 0;i < numThreads;i++)
            {
                if (threadArray[i]->isRunning())
                {
                    numThreadsRunning++;
                }
            }
            if (numThreadsRunning > 0)
            {
                numThreadsRunning = 0;

                if (!terminateHasRun)
                {
                    if (startTime.elapsed() > 8000) //terminate time is 8 seconds. Change to appropriate amount later
                    {
                        for (int k = 0;k < numThreads;k++)
                        {
                            if (threadArray[k]->isRunning())
                            {
                                qDebug() << "Terminating thread: " + k;
                                threadArray[k]->terminate();
                            }
                        }
                        terminateHasRun = true;
                        allThreadsKilled = true;
                    }
                }
            }

            else
            {
                allThreadsKilled = true;
            }
        }
        threadnode *temp = threadhead;
        while (!temp->nextBlank){
            threadhead = temp->next;
            delete temp;
        }

        return true;
    }
}

bool ThreadArray::m_clear(){
    if (!clearing){
        clearing = true;
        emit closeThreads();
        bool allThreadsKilled = false;
        int numThreadsRunning = 0;
        bool terminateHasRun = false;

        QTime startTime;
        startTime.start();

        while (!allThreadsKilled)
        {
            for (int i = 0;i < numThreads;i++)
            {
                if (threadList[i]->isRunning())
                {
                    numThreadsRunning++;
                }
            }
            if (numThreadsRunning > 0)
            {
                numThreadsRunning = 0;

                if (!terminateHasRun)
                {
                    if (startTime.elapsed() > 8000) //terminate time is 8 seconds. Change to appropriate amount later
                    {
                        for (int k = 0;k < numThreads;k++)
                        {
                            if (threadList[k]->isRunning())
                            {
                                qDebug() << "Terminating thread: " + k;
                                threadArray[k]->terminate();
                            }
                        }
                        terminateHasRun = true;
                        allThreadsKilled = true;
                    }
                }
            }

            else
            {
                allThreadsKilled = true;
            }
        }

        return true;
    }

}

ThreadArray::~ThreadArray()
{
    clear();
}
