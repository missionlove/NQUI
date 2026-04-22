#include "ndashboardpushbuttonbase.h"
#include <QMetaObject>

NDashboardPushButtonBase::NDashboardPushButtonBase(QWidget *parent)
    : QPushButton(parent),
      m_backgroundColor(QColor(55, 60, 70)),
      m_foregroundColor(QColor(240, 240, 240)),
      m_accentColor(QColor(106, 149, 255)),
      m_cornerRadius(8)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setCursor(Qt::PointingHandCursor);
    applyChromeStyle();
}

QColor NDashboardPushButtonBase::backgroundColor() const
{
    return m_backgroundColor;
}

void NDashboardPushButtonBase::setBackgroundColor(const QColor &c)
{
    if (m_backgroundColor == c) {
        return;
    }
    m_backgroundColor = c;
    applyChromeStyle();
    emit backgroundColorChanged(m_backgroundColor);
}

QColor NDashboardPushButtonBase::foregroundColor() const
{
    return m_foregroundColor;
}

void NDashboardPushButtonBase::setForegroundColor(const QColor &c)
{
    if (m_foregroundColor == c) {
        return;
    }
    m_foregroundColor = c;
    applyChromeStyle();
    emit foregroundColorChanged(m_foregroundColor);
}

QColor NDashboardPushButtonBase::accentColor() const
{
    return m_accentColor;
}

void NDashboardPushButtonBase::setAccentColor(const QColor &c)
{
    if (m_accentColor == c) {
        return;
    }
    m_accentColor = c;
    applyChromeStyle();
    emit accentColorChanged(m_accentColor);
}

int NDashboardPushButtonBase::cornerRadius() const
{
    return m_cornerRadius;
}

void NDashboardPushButtonBase::setCornerRadius(int r)
{
    r = qMax(0, r);
    if (m_cornerRadius == r) {
        return;
    }
    m_cornerRadius = r;
    applyChromeStyle();
    emit cornerRadiusChanged(m_cornerRadius);
}

QString NDashboardPushButtonBase::chromeStyleSheet() const
{
    const QString cls = QString::fromLatin1(metaObject()->className());
    const QString bg = m_backgroundColor.name(QColor::HexArgb);
    const QString fg = m_foregroundColor.name(QColor::HexArgb);
    const QString ac = m_accentColor.name(QColor::HexArgb);
    QColor hover = m_backgroundColor.lighter(115);
    QColor pressed = m_backgroundColor.darker(115);
    return QStringLiteral(
               "%1 { background-color: %2; color: %3; border-radius: %4px; border: 1px solid %5; padding: 6px 14px; }"
               "%1:hover { background-color: %6; }"
               "%1:pressed { background-color: %7; }"
               "%1:disabled { color: rgba(255,255,255,0.35); border-color: rgba(255,255,255,0.12); }")
        .arg(cls, bg, fg)
        .arg(m_cornerRadius)
        .arg(ac)
        .arg(hover.name(QColor::HexArgb))
        .arg(pressed.name(QColor::HexArgb));
}

void NDashboardPushButtonBase::applyChromeStyle()
{
    setStyleSheet(chromeStyleSheet());
}
