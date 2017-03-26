#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtWebEngine>

#include "messagehandler.h"
#include "backend.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //qInstallMessageHandler(myMessageHandler);


//    QQmlEngine engine;
//    QQmlComponent component(&engine,QUrl(QStringLiteral("qrc:/panel.qml")));
//    QObject *object = component.create();

 //   QQuickItem *item = qobject_cast<QQuickItem*>(object);
//    item->setWidth(500);
    QtWebEngine::initialize();

    QQuickView view;
    view.setSource(QUrl("qrc:/qml/main.qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);

//    QObject *item = view.rootObject();



    //QObject::connect(item, SIGNAL(closeMainWindow()), &w, SLOT(close()));
    //QObject::connect(item, SIGNAL(closeMainWindow()), item, SLOT(deleteLater()));

    //w.show();
    view.show();

    QObject *rootView = view.rootObject();
    Backend b(rootView);

    //   qDebug() << "main";

    return app.exec();
}
