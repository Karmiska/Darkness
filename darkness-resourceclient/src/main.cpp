#include "platform/Platform.h"
#include "ResourceProcessor.h"
#include "tools/ArgParser.h"
#include "UiHandler.h"

#include <QApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QResource>
#include <QFile>
#include <QFileInfo>
#include <QtCore/QDebug>

#include "LogItem.h"
#include "LogListModel.h"
#include "UiHandler.h"

using namespace resource_client;

int realmain(int argc, char** argv)
{
    engine::ArgParser argParser(argc, argv);
    bool headless = argParser.flag("headless");
    if (!headless)
    {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setOrganizationName("Codenist");
        QCoreApplication::setOrganizationDomain("codenist.com");
        QCoreApplication::setApplicationName("Darkness");
        QApplication app(argc, argv);

        QResource::registerResource(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + "/DarknessResourceClient.rcc");

        QQmlApplicationEngine engine;

        qmlRegisterType<LogListModel>("Darkness", 1, 0, "LogListModel");

        engine::unique_ptr<LogListModel> listModel = engine::make_unique<LogListModel>();
        engine.rootContext()->setContextProperty("logListModel", listModel.get());

        engine.load(QUrl(QLatin1String("qrc:/qml/main.qml")));

        QObject* rootObject = engine.rootObjects().first();
        QObject* mainPageForm = rootObject->findChild<QObject*>("mainPageForm");

        engine::unique_ptr<UiHandler> opHandler = engine::make_unique<UiHandler>(&engine, app.activeWindow()); 
        QObject::connect(mainPageForm, SIGNAL(createProject()), opHandler.get(), SLOT(createProject()));
        QObject::connect(mainPageForm, SIGNAL(openProject()), opHandler.get(), SLOT(openProject()));

        QObject::connect(opHandler.get(), SIGNAL(createProject(QString, QString)), listModel.get(), SLOT(createProject(QString, QString)));
        QObject::connect(opHandler.get(), SIGNAL(openProject(QString)), listModel.get(), SLOT(openProject(QString)));

        ResourceProcessor processor(argParser.value("identity"));
        processor.run(true);

        return app.exec();
    }
    else
    {
        ResourceProcessor processor(argParser.value("identity"));
        processor.run();
        return 0;
    }
}

#ifdef _WIN32
int APIENTRY WinMain(
    _In_ HINSTANCE /*hInstance*/,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int /*nCmdShow*/
)
{
    return realmain(__argc, __argv);
}

int main(int argc, char** argv)
{
    return realmain(argc, argv);
}
#endif
