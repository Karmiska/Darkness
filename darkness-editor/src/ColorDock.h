#pragma once

#include <QFrame>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QMenu)

class ColorDock : public QFrame
{
    Q_OBJECT
public:
    explicit ColorDock(const QString &c, QWidget *parent);

    QSize sizeHint() const Q_DECL_OVERRIDE { return szHint; }
    QSize minimumSizeHint() const Q_DECL_OVERRIDE { return minSzHint; }

    void setCustomSizeHint(const QSize &size);

    public slots:
    void changeSizeHints();

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    const QString color;
    QSize szHint;
    QSize minSzHint;
};
