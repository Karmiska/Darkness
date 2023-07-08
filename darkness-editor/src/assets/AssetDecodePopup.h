#pragma once

#include <atomic>
#include <QWidget>
#include <QMap>
#include <QQueue>
#include <QMutex>

static const int SingleItemHeight = 42;
static const int MaxItemCount = 10;
static const int MaximumHeight = 420;

class AssetDecodePopup : public QWidget
{
    Q_OBJECT
public:
    AssetDecodePopup(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    void addItem(const QString& absoluteFilename);
    void setItemStatus(const QString& absoluteFilename, const QString& status);
    void setItemProgress(const QString& absoluteFilename, float progress);
    void removeItem(const QString& absoluteFilename);

protected:
    void paintEvent(QPaintEvent *);
    void drawItem(const QString& displayFilename, const QString& status, const QRect& target, float progress);
    void resizeEvent(QResizeEvent *event);

private:
    struct DecodeItem
    {
        QString absoluteFilename;
        QString status;
        float progress;
        QString displayFilename;
    };
    QMap<QString, DecodeItem> m_items;
    QQueue<DecodeItem> m_queue;
    QMutex itemMutex;
    std::atomic<int> m_itemCount;
};
