#pragma once

#include <QString>
#include <QJsonObject>

class BrowserSettings
{
public:
    BrowserSettings();
    BrowserSettings(int thumbnailSize);

    QString settingsCategory() const;

    int thumbnailSize() const;
    int splitterPosition() const;

    void thumbnailSize(int size);
    void splitterPosition(int position);

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;

private:
    int m_thumbnailSize;
    int m_splitterPosition;
};
