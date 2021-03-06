#include "statusbar.h"

StatusBar::StatusBar(QWidget *parent) : QStatusBar(parent), elapsed(-1)
{
    sopb.minimum = 0;
    sopb.maximum = 100;
    sopb.rect = QRect(3, 3, 100, 16);

    connect(&tm, &QTimer::timeout, [this] {
        elapsed = t.elapsed();
        update();
    });
}

void StatusBar::start()
{
    frame = total = 1;
    sopb.progress = 0;

    tm.start(1000);

    elapsed = 0;
    t.start();
}

void StatusBar::stop()
{
    tm.stop();
}

#include <QStylePainter>

void StatusBar::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_PanelStatusBar, &opt, &p, this);

    int offset = 0;

    if (tm.isActive())
    {
        p.drawControl(QStyle::CE_ProgressBar, sopb);
        offset += 103;
    }

    p.setPen(palette().foreground().color());

    if (elapsed != -1)
    {
        p.drawText(QRect(offset + 6, 0, 50, height()), Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, QString("%1:%2:%3").arg(elapsed / 3600000).arg(elapsed / 60000 % 60, 2, 10, QLatin1Char('0')).arg(elapsed / 1000 % 60, 2, 10, QLatin1Char('0')));
        offset += 56;

        if (tm.isActive() && total > 1)
        {
            p.drawText(QRect(offset + 6, 0, 40, height()), Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, QString("%1/%2").arg(frame).arg(total));
            offset += 46;
        }
    }

    p.drawText(QRect(offset + 6, 0, width() - offset - 12, height()), Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, permanent.isEmpty() ? currentMessage() : permanent);
}

void StatusBar::clearPermanentMessage()
{
    permanent.clear();
    update();
}

void StatusBar::showPermanentMessage(const QString &text)
{
    permanent = text;
    update();
}

void StatusBar::setFrame(int frame, int total)
{
    this->frame = frame;
    this->total = total;
    update();
}

void StatusBar::setProgress(int progress)
{
    sopb.progress = (100 * (frame - 1) + progress) / total;
    update();
}
