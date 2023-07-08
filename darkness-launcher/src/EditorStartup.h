#pragma once

#include <QString>
#include <QObject>

class EditorStartup
{
public:
    EditorStartup(QObject* parent, const QString& projectPath);
private:
    QString getEditorPath() const;
};
