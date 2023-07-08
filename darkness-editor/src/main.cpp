#include "mainwindow.h"

#include <QtWidgets/QApplication>
#include <QtGui/QPainterPath>
#include <QtGui/QPainter>
#include <QMap>
#include <QDebug>
#include <QResource>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QString>
#include <QSettings>
#include <QMessageBox>

QString parseArgs(const QStringList &arguments)
{
    const int argumentCount = arguments.size();
    for (int i = 1; i < argumentCount; ++i) {
        const QString &arg = arguments.at(i);
        if (arg.startsWith(QLatin1String("-project"))) {
            qDebug() << arguments.at(i+1);
            return arguments.at(i + 1);
        }
    }
    return "";
}

void setProjectAttributes()
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("Codenist");
    QCoreApplication::setOrganizationDomain("codenist.com");
    QCoreApplication::setApplicationName("Darkness");
}

QString getFromLauncherSettings()
{
    QSettings settings;
    settings.beginGroup("launcher");
    int size = settings.beginReadArray("recentProjects");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString projectName = settings.value("projectName").toString();
        QString projectPath = settings.value("projectPath").toString();
        QFileInfo info(projectPath);
        if (info.exists())
            return projectPath;
    }
    settings.endArray();
    settings.endGroup();
    return "";
}

#ifdef _WIN32
int CALLBACK WinMain(
    _In_ HINSTANCE, // hInstance,
    _In_ HINSTANCE,  // hPrevInstance,
    _In_ LPSTR, //     lpCmdLine,
    _In_ int   //       nCmdShow
)
{
#ifdef Q_OS_WIN
    SetProcessDPIAware(); // call before the main event loop
#endif // Q_OS_WIN 

    setProjectAttributes();

    QApplication app(__argc, __argv);
    MainWindow::CustomSizeHintMap customSizeHints;
    QString projectFile = parseArgs(QCoreApplication::arguments());
    if (projectFile.isEmpty())
        projectFile = getFromLauncherSettings();

    if (projectFile.isEmpty())
    {
        QString msg;
        msg += "No project file was provided as argument\n";
        msg += "and we did not find existing projects\n\n";
        msg += "Please either add arguments like this:\n";
        msg += "-project \"path_to_project_file\"\n\n";
        msg += "Or create a new project by using the launcher.";

        QMessageBox message;
        message.setText(msg);
        message.setWindowTitle("No project file found!");
        message.exec();
        return -1;
    }

    QResource::registerResource(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + "/DarknessEditor.rcc");

    MainWindow mainWin(customSizeHints, projectFile);
    mainWin.resize(1280, 1024);
    mainWin.show();

    return app.exec();
}
#else
int main(int argc, char **argv)
{
    setProjectAttributes();

    QApplication app(argc, argv);
    MainWindow::CustomSizeHintMap customSizeHints;
    QString projectFile = parseArgs(QCoreApplication::arguments());
    if (projectFile.isEmpty())
        projectFile = getFromLauncherSettings();

    if (projectFile.isEmpty())
    {
        QString msg;
        msg += "No project file was provided as argument\n";
        msg += "and we did not find existing projects\n\n";
        msg += "Please either add arguments like this:\n";
        msg += "-project \"path_to_project_file\"\n\n";
        msg += "Or create a new project by using the launcher.";

        QMessageBox message;
        message.setText(msg);
        message.setWindowTitle("No project file found!");
        message.exec();
        return -1;
    }

    QResource::registerResource(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + "/DarknessEditor.rcc");

    MainWindow mainWin(customSizeHints, projectFile);
    mainWin.resize(1280, 1024);
    mainWin.show();

    return app.exec();
}
#endif