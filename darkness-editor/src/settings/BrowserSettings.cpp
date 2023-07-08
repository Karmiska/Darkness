#include "BrowserSettings.h"
#include <QDebug>

BrowserSettings::BrowserSettings()
{
}

BrowserSettings::BrowserSettings(int thumbnailSize)
    : m_thumbnailSize{ thumbnailSize }
{
}

QString BrowserSettings::settingsCategory() const
{
    return "browserSettings";
}

int BrowserSettings::thumbnailSize() const
{
    return m_thumbnailSize;
}

int BrowserSettings::splitterPosition() const
{
    return m_splitterPosition;
}

void BrowserSettings::thumbnailSize(int size)
{
    m_thumbnailSize = size;
}

void BrowserSettings::splitterPosition(int position)
{
    m_splitterPosition = position;
}

void BrowserSettings::read(const QJsonObject& json)
{
    m_thumbnailSize = json["thumbnailSize"].toInt();
    m_splitterPosition = json["splitterPosition"].toInt();
}

void BrowserSettings::write(QJsonObject& json) const
{
    json["thumbnailSize"] = m_thumbnailSize;
    json["splitterPosition"] = m_splitterPosition;
}

