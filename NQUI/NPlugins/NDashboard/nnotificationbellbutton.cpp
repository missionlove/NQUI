#include "nnotificationbellbutton.h"
#include <QMetaObject>
#include <QPainter>
#include <QPaintEvent>

NNotificationBellButton::NNotificationBellButton(QWidget *parent)
    : NDashboardPushButtonBase(parent),
      m_badgeCount(0)
{
    setBellGlyph(QStringLiteral("\xF0\x9F\x94\x94")); // UTF-8 bell
    setFixedSize(40, 36);
}

void NNotificationBellButton::setBadgeCount(int count)
{
    count = qMax(0, count);
    if (m_badgeCount == count) {
        return;
    }
    m_badgeCount = count;
    update();
}

int NNotificationBellButton::badgeCount() const
{
    return m_badgeCount;
}

void NNotificationBellButton::setBellGlyph(const QString &glyph)
{
    setText(glyph);
}

void NNotificationBellButton::applyChromeStyle()
{
    const QString cls = QString::fromLatin1(metaObject()->className());
    const QString bg = backgroundColor().name(QColor::HexArgb);
    const QString fg = foregroundColor().name(QColor::HexArgb);
    const QString ac = accentColor().name(QColor::HexArgb);
    setStyleSheet(QStringLiteral(
                      "%1 { background-color: %2; color: %3; border-radius: %4px; border: 1px solid rgba(255,255,255,0.08); padding: 4px; font-size: 16px; }"
                      "%1:hover { border-color: %5; }")
                      .arg(cls, bg, fg)
                      .arg(cornerRadius())
                      .arg(ac));
}

void NNotificationBellButton::paintEvent(QPaintEvent *event)
{
    NDashboardPushButtonBase::paintEvent(event);
    if (m_badgeCount <= 0) {
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int d = 8;
    QRect dot(rect().right() - d - 4, rect().top() + 4, d, d);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 82, 82));
    p.drawEllipse(dot);
}
