#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "CaptureVideoFilter.h"
#include "EnumInfo.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<CaptureVideoFilter>("QtCaptureVideoApp", 1, 0, "CaptureVideoFilter");
    qmlRegisterType<EnumInfo>("QtCaptureVideoApp", 1, 0, "EnumInfo");

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
