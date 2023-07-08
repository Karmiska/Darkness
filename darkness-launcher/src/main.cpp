#include <QApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QResource>
#include <QFile>
#include <QFileInfo>

#include <QtCore/QDebug>
#include "projectitem.h"
#include "projectlistmodel.h"
#include "containers/memory.h"
#include "projectoperationhandler.h"

#ifdef _WIN32
#include "platform/Platform.h"
int CALLBACK WinMain(
    _In_ HINSTANCE, // hInstance,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int   //       nCmdShow
)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("Codenist");
    QCoreApplication::setOrganizationDomain("codenist.com");
    QCoreApplication::setApplicationName("Darkness");
    QApplication app(__argc, __argv);

    QResource::registerResource(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + "/DarknessLauncher.rcc");

    QQmlApplicationEngine engine;

    qmlRegisterType<ProjectListModel>("Darkness", 1, 0, "ProjectListModel");

    engine::unique_ptr<ProjectListModel> listModel = engine::make_unique<ProjectListModel>();
    engine.rootContext()->setContextProperty("projectListModel", listModel.get());

    engine.load(QUrl(QLatin1String("qrc:/qml/main.qml")));

    QObject* rootObject = engine.rootObjects().first();
    QObject* mainPageForm = rootObject->findChild<QObject*>("mainPageForm");

    engine::unique_ptr<ProjectOperationHandler> opHandler =
        engine::make_unique<ProjectOperationHandler>(&engine, app.activeWindow());
    QObject::connect(mainPageForm, SIGNAL(createProject()), opHandler.get(), SLOT(createProject()));
    QObject::connect(mainPageForm, SIGNAL(openProject()), opHandler.get(), SLOT(openProject()));

    QObject::connect(opHandler.get(), SIGNAL(createProject(QString, QString)), listModel.get(), SLOT(createProject(QString, QString)));
    QObject::connect(opHandler.get(), SIGNAL(openProject(QString)), listModel.get(), SLOT(openProject(QString)));

    return app.exec();
}
#else
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("Codenist");
    QCoreApplication::setOrganizationDomain("codenist.com");
    QCoreApplication::setApplicationName("Darkness");
    QApplication app(argc, argv);

    QResource::registerResource(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + "/DarknessLauncher.rcc");

    QQmlApplicationEngine engine;

    qmlRegisterType<ProjectListModel>("Darkness", 1, 0, "ProjectListModel");

    engine::unique_ptr<ProjectListModel> listModel = engine::make_unique<ProjectListModel>();
    engine.rootContext()->setContextProperty("projectListModel", listModel.get());

    engine.load(QUrl(QLatin1String("qrc:/qml/main.qml")));

    QObject* rootObject = engine.rootObjects().first();
    QObject* mainPageForm = rootObject->findChild<QObject*>("mainPageForm");

    engine::unique_ptr<ProjectOperationHandler> opHandler =
            engine::make_unique<ProjectOperationHandler>(&engine, app.activeWindow());
    QObject::connect(mainPageForm, SIGNAL(createProject()), opHandler.get(), SLOT(createProject()));
    QObject::connect(mainPageForm, SIGNAL(openProject()), opHandler.get(), SLOT(openProject()));

    QObject::connect(opHandler.get(), SIGNAL(createProject(QString, QString)), listModel.get(), SLOT(createProject(QString, QString)));
    QObject::connect(opHandler.get(), SIGNAL(openProject(QString)), listModel.get(), SLOT(openProject(QString)));

    return app.exec();
}
#endif
