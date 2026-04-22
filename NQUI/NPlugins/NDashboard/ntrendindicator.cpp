#include "ntrendindicator.h"
#include <QHBoxLayout>
#include <QLabel>

NTrendIndicator::NTrendIndicator(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_arrow(new QLabel(this)),
      m_value(new QLabel(this)),
      m_dir(Neutral),
      m_pos(QColor(72, 199, 116)),
      m_neg(QColor(255, 107, 107))
{
    setMinimumHeight(28);
    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(8, 4, 8, 4);
    h->setSpacing(6);
    h->addWidget(m_value);
    h->addWidget(m_arrow);

    setDeltaText(QStringLiteral("0%"));
    setDirection(Neutral);
    applyChromeStyle();
}

void NTrendIndicator::setDirection(Direction d)
{
    m_dir = d;
    refreshGlyph();
}

NTrendIndicator::Direction NTrendIndicator::direction() const
{
    return m_dir;
}

void NTrendIndicator::setDeltaText(const QString &text)
{
    m_value->setText(text);
}

QString NTrendIndicator::deltaText() const
{
    return m_value->text();
}

void NTrendIndicator::setPositiveColor(const QColor &c)
{
    m_pos = c;
    refreshGlyph();
}

void NTrendIndicator::setNegativeColor(const QColor &c)
{
    m_neg = c;
    refreshGlyph();
}

QColor NTrendIndicator::positiveColor() const
{
    return m_pos;
}

QColor NTrendIndicator::negativeColor() const
{
    return m_neg;
}

void NTrendIndicator::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    refreshGlyph();
}

void NTrendIndicator::refreshGlyph()
{
    QColor c = foregroundColor();
    QString g = QStringLiteral("\u2014");
    if (m_dir == Up) {
        g = QStringLiteral("⬆");
        c = m_pos;
    } else if (m_dir == Down) {
        g = QStringLiteral("⬇");
        c = m_neg;
    }
    m_arrow->setText(g);
    m_arrow->setStyleSheet(QStringLiteral("color: %1; font-size: 12px; font-weight: 800;").arg(c.name(QColor::HexArgb)));
    m_value->setStyleSheet(QStringLiteral("color: %1; font-size: 12px; font-weight: 600;").arg(c.name(QColor::HexArgb)));
}
