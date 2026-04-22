#include "nsidebarnavitem.h"
#include <QMetaObject>
#include <QPainter>
#include <QPaintEvent>

NSidebarNavItem::NSidebarNavItem(const QString &text, QWidget *parent)
    : NDashboardPushButtonBase(parent),
      m_badgeCount(0)
{
    setText(text);
    setCheckable(true);
    setFlat(true);
    setMinimumHeight(36);
}

void NSidebarNavItem::setBadgeCount(int count)
{
    count = qMax(0, count);
    if (m_badgeCount == count) {
        return;
    }
    m_badgeCount = count;
    update();
}

int NSidebarNavItem::badgeCount() const
{
    return m_badgeCount;
}

void NSidebarNavItem::applyChromeStyle()
{
    const QString cls = QString::fromLatin1(metaObject()->className());
    const QString bg = backgroundColor().name(QColor::HexArgb);
    const QString fg = foregroundColor().name(QColor::HexArgb);
    const QString ac = accentColor().name(QColor::HexArgb);
    const QString bgChecked = accentColor().darker(140).name(QColor::HexArgb);
    setStyleSheet(QStringLiteral(
                      "%1 { text-align: left; padding: 8px 28px 8px 12px; background-color: %2; color: %3; "
                      "border-radius: %4px; border: none; }"
                      "%1:checked { background-color: %5; border: 1px solid %6; }"
                      "%1:hover:!checked { background-color: rgba(255,255,255,0.06); }")
                      .arg(cls, bg, fg)
                      .arg(cornerRadius())
                      .arg(bgChecked, ac));
}

void NSidebarNavItem::paintEvent(QPaintEvent *event)
{
    NDashboardPushButtonBase::paintEvent(event);
    if (m_badgeCount <= 0) {
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QString txt = m_badgeCount > 99 ? QStringLiteral("99+") : QString::number(m_badgeCount);
    QFont f = font();
    f.setPixelSize(10);
    f.setBold(true);
    p.setFont(f);
    QRect r = rect().adjusted(0, 0, -6, 0);
    const QSize ts = p.fontMetrics().boundingRect(txt).size() + QSize(8, 4);
    QRect badge(r.right() - ts.width(), r.center().y() - ts.height() / 2, ts.width(), ts.height());
    p.setPen(Qt::NoPen);
    p.setBrush(accentColor());
    p.drawRoundedRect(badge, 8, 8);
    p.setPen(foregroundColor());
    p.drawText(badge, Qt::AlignCenter, txt);
}
