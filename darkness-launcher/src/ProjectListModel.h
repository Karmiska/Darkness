#ifndef PROJECTLISTMODEL_H
#define PROJECTLISTMODEL_H

#include <QAbstractListModel>
#include "projectitem.h"

class ProjectListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ProjectListRoles
    {
        NameRole = Qt::UserRole + 1,
        PathRole,
        LineRole,
        LoadRole
    };

    Q_ENUMS(ProjectListRoles)

    ProjectListModel(QObject* parent = 0);
    virtual ~ProjectListModel();

    int rowCount(const QModelIndex& parentP = QModelIndex()) const;
    QVariant data(const QModelIndex& indexP, int roleP = Qt::DisplayRole) const;
    bool setData ( const QModelIndex &indexP, const QVariant& valueP, int roleP = Qt::EditRole );
    void refreshData();

protected:
    QHash<int, QByteArray> roleNames() const;

public slots:
    void setData(const int rowP, const QVariant& valueP, int roleP);
    void remove(const int rowP);
    void createProject(const QString& name, const QString& location);
    void openProject(const QString& location);

private:
    QList<ProjectItem*> projectItems;
    void loadRecentProjects();
    void saveRecentProjects();
};
Q_DECLARE_METATYPE(ProjectListModel*)

#endif // PROJECTLISTMODEL_H
