#ifndef PROJECTOPERATIONHANDLER_H
#define PROJECTOPERATIONHANDLER_H

#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <QQmlApplicationEngine>

class ProjectOperationHandler : public QObject
{
    Q_OBJECT
public:
    ProjectOperationHandler(
            QQmlApplicationEngine* engine, QWidget* parent = 0);
public slots:
    // from main.cpp
    void createProject();
    void openProject();

    // from CreateProjectMain.qml
    void nameChanged(QString name);
    void locationChanged(QString location);
    void browseClicked();
    void cancelClicked();
    void createClicked();

    // from Page1Form.ui.qml
    void openProject(QString name, QString location);

signals:
    void createProject(const QString& name, const QString& location);
    void openProject(const QString& location);

private:
    QQmlApplicationEngine* m_engine;
    QWidget* m_parent;

    QString m_projectName;
    QString m_projectLocation;

    QObject* m_createProjectForm;
};

#endif // PROJECTOPERATIONHANDLER_H
