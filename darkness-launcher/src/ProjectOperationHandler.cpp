#include "projectoperationhandler.h"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>
#include <QtCore/QDebug>

ProjectOperationHandler::ProjectOperationHandler(QQmlApplicationEngine* engine, QWidget* parent)
    : m_engine{ engine }
    , m_parent{ parent }
    , m_projectName{ "" }
    , m_projectLocation{ "" }
{
    QObject* rootObject = m_engine->rootObjects().first();
    QObject* createProjectSlide = rootObject->findChild<QObject*>("createProjectMainIdName");
    m_createProjectForm = rootObject->findChild<QObject*>("createProjectForm");

    QObject::connect(
        m_createProjectForm, SIGNAL(nameChanged(QString)),
        this, SLOT(nameChanged(QString)));
    QObject::connect(
        m_createProjectForm, SIGNAL(locationChanged(QString)),
        this, SLOT(locationChanged(QString)));
    QObject::connect(
        m_createProjectForm, SIGNAL(browseClicked()),
        this, SLOT(browseClicked()));
    QObject::connect(
        m_createProjectForm, SIGNAL(cancelClicked()),
        this, SLOT(cancelClicked()));
    QObject::connect(
        m_createProjectForm, SIGNAL(createClicked()),
        this, SLOT(createClicked()));

    QObject* form = m_createProjectForm->findChild<QObject*>("projectLocationInput"); 
    form->setProperty("text", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

    QObject* mainPageForm = rootObject->findChild<QObject*>("mainPageForm", Qt::FindChildrenRecursively);
    QObject::connect(mainPageForm, SIGNAL(openProjectLocation(QString, QString)), this, SLOT(openProject(QString, QString)));
}

void ProjectOperationHandler::openProject(QString name, QString location)
{
    emit openProject(location);
}

void ProjectOperationHandler::createProject()
{
    QObject* rootObject = m_engine->rootObjects().first();
    QObject* createProjectSlide = rootObject->findChild<QObject*>("createProjectMainIdName");
    m_createProjectForm = rootObject->findChild<QObject*>("createProjectForm");

    createProjectSlide->setProperty("visible", true);
    createProjectSlide->setProperty("y", 56.0f);

    QObject* form = m_createProjectForm->findChild<QObject*>("projectNameInput");
    m_projectName = form->property("text").toString();
    form = m_createProjectForm->findChild<QObject*>("projectLocationInput");
    m_projectLocation = form->property("text").toString();

    form = rootObject->findChild<QObject*>("goDark", Qt::FindChildrenRecursively);
    form->setProperty("running", true);
    form = rootObject->findChild<QObject*>("goOpenDark", Qt::FindChildrenRecursively);
    form->setProperty("running", true);

    form = rootObject->findChild<QObject*>("goDarkMouseArea", Qt::FindChildrenRecursively);
    form->setProperty("enabled", false);
    form = rootObject->findChild<QObject*>("goOpenDarkMouseArea", Qt::FindChildrenRecursively);
    form->setProperty("enabled", false);

}

void ProjectOperationHandler::nameChanged(QString name)
{
    m_projectName = name;
    QObject* form = m_createProjectForm->findChild<QObject*>("projectNameInput");
    form->setProperty("text", name);
}

void ProjectOperationHandler::locationChanged(QString location)
{
    m_projectLocation = location;
    QObject* form = m_createProjectForm->findChild<QObject*>("projectLocationInput");
    form->setProperty("text", location);
}

void ProjectOperationHandler::browseClicked()
{
    QString filename = QFileDialog::getSaveFileName(
                    m_parent,
                    tr("Open Project"),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                    tr("Project Files (*.darkproj)")
                    );
    m_projectLocation = filename;
    QObject* form = m_createProjectForm->findChild<QObject*>("projectLocationInput");
    form->setProperty("text", m_projectLocation);
}

void ProjectOperationHandler::cancelClicked()
{
    QObject* rootObject = m_engine->rootObjects().first();
    QObject* form = rootObject->findChild<QObject*>("goLight", Qt::FindChildrenRecursively);
    form->setProperty("running", true);
    form = rootObject->findChild<QObject*>("goOpenLight", Qt::FindChildrenRecursively);
    form->setProperty("running", true);

    form = rootObject->findChild<QObject*>("goDarkMouseArea", Qt::FindChildrenRecursively);
    form->setProperty("enabled", true);
    form = rootObject->findChild<QObject*>("goOpenDarkMouseArea", Qt::FindChildrenRecursively);
    form->setProperty("enabled", true);
}

void ProjectOperationHandler::createClicked()
{
    if(m_projectName != "" && m_projectLocation != "")
    {
        QObject* rootObject = m_engine->rootObjects().first();
        QObject* form = rootObject->findChild<QObject*>("goLight", Qt::FindChildrenRecursively);
        form->setProperty("running", true);
        form = rootObject->findChild<QObject*>("goOpenLight", Qt::FindChildrenRecursively);
        form->setProperty("running", true);

        form = rootObject->findChild<QObject*>("goDarkMouseArea", Qt::FindChildrenRecursively);
        form->setProperty("enabled", true);
        form = rootObject->findChild<QObject*>("goOpenDarkMouseArea", Qt::FindChildrenRecursively);
        form->setProperty("enabled", true);

        form = m_createProjectForm->findChild<QObject*>("projectNameInput");
        m_projectName = form->property("text").toString();
        form = m_createProjectForm->findChild<QObject*>("projectLocationInput");
        m_projectLocation = form->property("text").toString();

        qDebug() << m_projectName << m_projectLocation;

        emit createProject(m_projectName, m_projectLocation);
    }
}

void ProjectOperationHandler::openProject()
{
    QString filename = QFileDialog::getOpenFileName(
                m_parent,
                tr("Open Project"),
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                tr("Project Files (*.darkproj)")
                );
    if(filename != "")
        emit openProject(filename);
}

