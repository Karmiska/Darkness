#include "AssetDecodePopup.h"
#include <QStyleOption>
#include <QPainter>
#include <QFileInfo>

AssetDecodePopup::AssetDecodePopup(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent)
    , m_itemCount{ 0 }
{
    setGeometry(0, 0, 0, 0);
    show();
}

void AssetDecodePopup::resizeEvent(QResizeEvent *event)
{
    setGeometry(
        parentWidget()->width() - 300,
        parentWidget()->height() - SingleItemHeight * m_items.count(),
        300,
        SingleItemHeight * m_itemCount);
}

void AssetDecodePopup::addItem(const QString& absoluteFilename)
{
    itemMutex.lock();
    bool alreadyHaveIt = m_items.contains(absoluteFilename);
    for (auto& item : m_queue)
    {
        if (item.absoluteFilename == absoluteFilename)
        {
            alreadyHaveIt = true;
            break;
        }
    }
    if (alreadyHaveIt)
    {
        itemMutex.unlock();
        return;
    }

    if (m_items.count() >= MaxItemCount)
    {
        m_queue.enqueue({ absoluteFilename, "", 0.0f, QFileInfo(absoluteFilename).fileName() });
    }
    else
    {
        m_items[absoluteFilename] = { absoluteFilename, "", 0.0f, QFileInfo(absoluteFilename).fileName() };
        ++m_itemCount;
    }
    itemMutex.unlock();

    setGeometry(
        parentWidget()->width() - 300,
        parentWidget()->height() - SingleItemHeight * m_items.count(),
        300,
        SingleItemHeight * m_itemCount);

    update();
}

void AssetDecodePopup::setItemStatus(const QString& absoluteFilename, const QString& status)
{
    itemMutex.lock();
    if (m_items.contains(absoluteFilename))
    {
        m_items[absoluteFilename].status = status;
    }
    else
    {
        for (auto& item : m_queue)
        {
            if (item.absoluteFilename == absoluteFilename)
            {
                item.status = status;
                break;
            }
        }
    }
    itemMutex.unlock();

    update();
}

void AssetDecodePopup::setItemProgress(const QString& absoluteFilename, float progress)
{
    itemMutex.lock();
    if (m_items.contains(absoluteFilename))
    {
        m_items[absoluteFilename].progress = progress;
    }
    else
    {
        for (auto& item : m_queue)
        {
            if (item.absoluteFilename == absoluteFilename)
            {
                item.progress = progress;
                break;
            }
        }
    }
    itemMutex.unlock();

    update();
}

void AssetDecodePopup::removeItem(const QString& absoluteFilename)
{
    itemMutex.lock();
    if (m_items.contains(absoluteFilename))
    {
        m_items.remove(absoluteFilename);
        --m_itemCount;
        if (m_queue.size() > 0)
        {
            DecodeItem newItem = m_queue.dequeue();
            m_items[newItem.absoluteFilename] = newItem;
            ++m_itemCount;
        }
    }
    else
    {
        int index = 0;
        for (auto& item : m_queue)
        {
            if (item.absoluteFilename == absoluteFilename)
            {
                m_queue.removeAt(index);
                break;
            }
            ++index;
        }
    }
    itemMutex.unlock();

    auto oldGeometry = geometry();
    setGeometry(
        parentWidget()->width() - 300,
        parentWidget()->height() - SingleItemHeight * m_items.count(),
        300,
        SingleItemHeight * m_itemCount);
    auto newGeometry = geometry();

    //updateGeometry();
    //repaint(oldGeometry.united(newGeometry));
    repaint();
    parentWidget()->repaint();
}

void AssetDecodePopup::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    //style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    itemMutex.lock();

    QBrush brush;
    brush.setColor(QColor(51, 51, 51));
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    p.fillRect(0, 0, width(), height(), brush);
    
    QPen pen;
    pen.setColor(QColor(41, 41, 41));
    p.setPen(pen);
    p.drawRect(0, 0, width()-1, height()-1);

    
    int yPos = 2;
    for (const auto& item : m_items)
    {
        drawItem(item.displayFilename, item.status, QRect(2, yPos, width() - 5, SingleItemHeight), item.progress);
        yPos += SingleItemHeight;
    }
    itemMutex.unlock();
}

void AssetDecodePopup::drawItem(const QString& displayFilename, const QString& status, const QRect& target, float progress)
{
    QPainter p(this);

    // item background
    QBrush brush;
    brush.setColor(QColor(60, 60, 60));
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    p.fillRect(target.left(), target.top(), target.width(), target.height(), brush);

    // item borders
    QPen pen;
    pen.setColor(QColor(41, 41, 41));
    p.setPen(pen);
    p.drawRect(target.left(), target.top(), target.width(), target.height());

    // absoluteFilename and status text
    pen.setColor(QColor(220, 220, 220));
    p.setPen(pen);
    p.drawText(QRect(target.left() + 1, target.top(), target.width() - 2, 15), displayFilename);
    p.drawText(QRect(target.left() + 1, target.top() + 15, target.width() - 2, 15), status);

    // progress background
    brush.setColor(QColor(60, 60, 60));
    p.fillRect(target.left() + 2, target.top() + 30, target.width() - 4, 10, brush);

    // progress borders
    pen.setColor(QColor(41, 41, 41));
    p.setPen(pen);
    p.drawRect(target.left() + 2, target.top() + 30, target.width() - 4, 10);

    // progress bar
    QLinearGradient gradient(QPoint(target.left() + 4, target.top() + 32), QPoint(target.width() - 8, target.top() + 32));
    gradient.setColorAt(0, QColor(43, 155, 207));
    gradient.setColorAt(1, QColor(195, 236, 255));
    float progWidth = (target.width() - 7) * progress;
    float progHeight = 6;
    p.fillRect(QRect(target.left() + 4, target.top() + 32, progWidth, 6), gradient);
}
