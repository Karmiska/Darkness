#include "colordock.h"

#include <QAction>
#include <QtEvents>
#include <QFrame>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSpinBox>
#include <QLabel>
#include <QPainterPath>
#include <QPushButton>
#include <QHBoxLayout>
#include <QBitmap>
#include <QtDebug>

QColor bgColorForName2(const QString &name)
{
    if (name == "Black")
        return QColor("#3F3F41");
    if (name == "White")
        return QColor("#3F3F41");
    if (name == "Red")
        return QColor("#3F3F41");
    if (name == "Green")
        return QColor("#3F3F41");
    if (name == "Blue")
        return QColor("#3F3F41");
    if (name == "Yellow")
        return QColor("#3F3F41");
    return QColor(name).light(110);
}

QColor fgColorForName2(const QString &name)
{
    if (name == "Black")
        return QColor("#373737");
    if (name == "White")
        return QColor("#373737");
    if (name == "Red")
        return QColor("#373737");
    if (name == "Green")
        return QColor("#373737");
    if (name == "Blue")
        return QColor("#373737");
    if (name == "Yellow")
        return QColor("#373737");
    return QColor(name);
}

ColorDock::ColorDock(const QString &c, QWidget *parent)
    : QFrame(parent)
    , color(c)
    , szHint(-1, -1)
    , minSzHint(125, 25)
{
    QFont font = this->font();
    font.setPointSize(8);
    setFont(font);
}

void ColorDock::paintEvent(QPaintEvent *)
{
    return;
#if 0
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), bgColorForName2(color));

    p.save();

    //extern void render_qt_text(QPainter *, int, int, const QColor &);
    //render_qt_text(&p, width(), height(), fgColorForName2(color));

    p.restore();

#ifdef DEBUG_SIZEHINTS
    p.setRenderHint(QPainter::Antialiasing, false);

    QSize sz = size();
    QSize szHint = sizeHint();
    QSize minSzHint = minimumSizeHint();
    QSize maxSz = maximumSize();
    QString text = QString::fromLatin1("sz: %1x%2\nszHint: %3x%4\nminSzHint: %5x%6\n"
        "maxSz: %8x%9")
        .arg(sz.width()).arg(sz.height())
        .arg(szHint.width()).arg(szHint.height())
        .arg(minSzHint.width()).arg(minSzHint.height())
        .arg(maxSz.width()).arg(maxSz.height());

    QRect r = fontMetrics().boundingRect(rect(), Qt::AlignLeft | Qt::AlignTop, text);
    r.adjust(-2, -2, 1, 1);
    p.translate(4, 4);
    QColor bg = Qt::yellow;
    bg.setAlpha(120);
    p.setBrush(bg);
    p.setPen(Qt::black);
    p.drawRect(r);
    p.drawText(rect(), Qt::AlignLeft | Qt::AlignTop, text);
#endif // DEBUG_SIZEHINTS
#endif
}

QSpinBox *createSpinBox2(int value, QWidget *parent, int max = 1000)
{
    QSpinBox *result = new QSpinBox(parent);
    result->setMinimum(-1);
    result->setMaximum(max);
    result->setValue(value);
    return result;
}

void ColorDock::changeSizeHints()
{
    QDialog dialog(this);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setWindowTitle(color);

    QVBoxLayout *topLayout = new QVBoxLayout(&dialog);

    QGridLayout *inputLayout = new QGridLayout();
    topLayout->addLayout(inputLayout);

    inputLayout->addWidget(new QLabel(tr("Size Hint:"), &dialog), 0, 0);
    inputLayout->addWidget(new QLabel(tr("Min Size Hint:"), &dialog), 1, 0);
    inputLayout->addWidget(new QLabel(tr("Max Size:"), &dialog), 2, 0);
    inputLayout->addWidget(new QLabel(tr("Dock Widget Max Size:"), &dialog), 3, 0);

    QSpinBox *szHintW = createSpinBox2(szHint.width(), &dialog);
    inputLayout->addWidget(szHintW, 0, 1);
    QSpinBox *szHintH = createSpinBox2(szHint.height(), &dialog);
    inputLayout->addWidget(szHintH, 0, 2);

    QSpinBox *minSzHintW = createSpinBox2(minSzHint.width(), &dialog);
    inputLayout->addWidget(minSzHintW, 1, 1);
    QSpinBox *minSzHintH = createSpinBox2(minSzHint.height(), &dialog);
    inputLayout->addWidget(minSzHintH, 1, 2);

    QSize maxSz = maximumSize();
    QSpinBox *maxSzW = createSpinBox2(maxSz.width(), &dialog, QWIDGETSIZE_MAX);
    inputLayout->addWidget(maxSzW, 2, 1);
    QSpinBox *maxSzH = createSpinBox2(maxSz.height(), &dialog, QWIDGETSIZE_MAX);
    inputLayout->addWidget(maxSzH, 2, 2);

    QSize dwMaxSz = parentWidget()->maximumSize();
    QSpinBox *dwMaxSzW = createSpinBox2(dwMaxSz.width(), &dialog, QWIDGETSIZE_MAX);
    inputLayout->addWidget(dwMaxSzW, 3, 1);
    QSpinBox *dwMaxSzH = createSpinBox2(dwMaxSz.height(), &dialog, QWIDGETSIZE_MAX);
    inputLayout->addWidget(dwMaxSzH, 3, 2);

    inputLayout->setColumnStretch(1, 1);
    inputLayout->setColumnStretch(2, 1);

    topLayout->addStretch();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::reject);

    topLayout->addWidget(buttonBox);

    if (dialog.exec() != QDialog::Accepted)
        return;

    szHint = QSize(szHintW->value(), szHintH->value());
    minSzHint = QSize(minSzHintW->value(), minSzHintH->value());
    maxSz = QSize(maxSzW->value(), maxSzH->value());
    setMaximumSize(maxSz);
    dwMaxSz = QSize(dwMaxSzW->value(), dwMaxSzH->value());
    parentWidget()->setMaximumSize(dwMaxSz);
    updateGeometry();
    update();
}

void ColorDock::setCustomSizeHint(const QSize &size)
{
    if (szHint != size) {
        szHint = size;
        updateGeometry();
    }
}
