#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "customdata.h"
int main(int argc, char *argv[])
{
    /* Initialize GStreamer */
      gst_init (&argc, &argv);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
 
    QGuiApplication app(argc, argv);
 
    QQmlApplicationEngine engine;
 
    MyCppClass myCppClass;
 
    engine.rootContext()->setContextProperty("myCppClass", &myCppClass);
 
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);
 
    return app.exec();
}