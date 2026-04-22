#include "ndashboardwidgetbase.h"
#include <QMetaObject>

NDashboardWidgetBase::NDashboardWidgetBase(QWidget *parent)
    : QWidget(parent),
      m_backgroundColor(QColor(40, 44, 52)),
      m_foregroundColor(QColor(240, 240, 240)),
      m_accentColor(QColor(106, 149, 255)),
      m_cornerRadius(10),
      m_hSpacing(8),
      m_vSpacing(8)
{
    setAttribute(Qt::WA_StyledBackground, true);
    applyChromeStyle();
}

QColor NDashboardWidgetBase::backgroundColor() const
{
    return m_backgroundColor;
}

void NDashboardWidgetBase::setBackgroundColor(const QColor &c)
{
    if (m_backgroundColor == c) {
        return;
    }
    m_backgroundColor = c;
    applyChromeStyle();
    emit backgroundColorChanged(m_backgroundColor);
}

QColor NDashboardWidgetBase::foregroundColor() const
{
    return m_foregroundColor;
}

void NDashboardWidgetBase::setForegroundColor(const QColor &c)
{
    if (m_foregroundColor == c) {
        return;
    }
    m_foregroundColor = c;
    applyChromeStyle();
    emit foregroundColorChanged(m_foregroundColor);
}

QColor NDashboardWidgetBase::accentColor() const
{
    return m_accentColor;
}

void NDashboardWidgetBase::setAccentColor(const QColor &c)
{
    if (m_accentColor == c) {
        return;
    }
    m_accentColor = c;
    applyChromeStyle();
    emit accentColorChanged(m_accentColor);
}

int NDashboardWidgetBase::cornerRadius() const
{
    return m_cornerRadius;
}

void NDashboardWidgetBase::setCornerRadius(int r)
{
    r = qMax(0, r);
    if (m_cornerRadius == r) {
        return;
    }
    m_cornerRadius = r;
    applyChromeStyle();
    emit cornerRadiusChanged(m_cornerRadius);
}

int NDashboardWidgetBase::horizontalSpacing() const
{
    return m_hSpacing;
}

void NDashboardWidgetBase::setHorizontalSpacing(int s)
{
    s = qMax(0, s);
    if (m_hSpacing == s) {
        return;
    }
    m_hSpacing = s;
    emit spacingChanged();
}

int NDashboardWidgetBase::verticalSpacing() const
{
    return m_vSpacing;
}

void NDashboardWidgetBase::setVerticalSpacing(int s)
{
    s = qMax(0, s);
    if (m_vSpacing == s) {
        return;
    }
    m_vSpacing = s;
    emit spacingChanged();
}

QString NDashboardWidgetBase::surfaceStyleSheet() const
{
    const QString cls = QString::fromLatin1(metaObject()->className());
    const QString bg = m_backgroundColor.name(QColor::HexArgb);
    const QString fg = m_foregroundColor.name(QColor::HexArgb);
    const QString ac = m_accentColor.name(QColor::HexArgb);
    return QStringLiteral("%1 { background-color: %2; color: %3; border-radius: %4px; }"
                          "%1:focus { border: 1px solid %5; }")
        .arg(cls, bg, fg)
        .arg(m_cornerRadius)
        .arg(ac);
}

void NDashboardWidgetBase::applyChromeStyle()
{
    setStyleSheet(surfaceStyleSheet());
}
