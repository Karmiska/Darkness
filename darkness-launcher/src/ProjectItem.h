#ifndef PROJECTITEM_H
#define PROJECTITEM_H

#include <QtCore/QObject>

class ProjectItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString projectName READ projectName WRITE setProjectName NOTIFY projectNameChanged)
    Q_PROPERTY(QString projectPath READ projectPath WRITE setProjectPath NOTIFY projectPathChanged)

public:
    ProjectItem(QObject *parent=0);
    ProjectItem(QString projectName, QString projectPath, QObject *parent=0);
    QString projectName() const;
    QString projectPath() const;
    void setProjectName(const QString& name);
    void setProjectPath(const QString& path);

signals:
    void projectNameChanged();
    void projectPathChanged();

private:
    QString m_projectName;
    QString m_projectPath;
};

#endif // PROJECTITEM_H
