#include "nmetricsummarycard.h"
#include "ntrendindicator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

NMetricSummaryCard::NMetricSummaryCard(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_title(new QLabel(this)),
      m_value(new QLabel(this)),
      m_foot(new QLabel(this)),
      m_trend(new NTrendIndicator(this))
{
    m_title->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.75;"));
    m_value->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: 800;"));
    m_foot->setStyleSheet(QStringLiteral("font-size: 11px; opacity: 0.55;"));
    QVBoxLayout *v = new QVBoxLayout(this);
    v->setSpacing(verticalSpacing());
    v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    v->addWidget(m_title);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(m_value);
    hlayout->addSpacing(20);
    hlayout->addWidget(m_trend);
    hlayout->addStretch(1);

    v->addLayout(hlayout);

    v->addWidget(m_foot);
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this, v]() {
        v->setSpacing(verticalSpacing());
        v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    });
    applyChromeStyle();
}

void NMetricSummaryCard::setTitle(const QString &title)
{
    m_title->setText(title);
}

void NMetricSummaryCard::setValueText(const QString &value)
{
    m_value->setText(value);
}

void NMetricSummaryCard::setFootnote(const QString &text)
{
    m_foot->setText(text);
}

NTrendIndicator *NMetricSummaryCard::trendWidget()
{
    return m_trend;
}

void NMetricSummaryCard::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    const QString fg = foregroundColor().name(QColor::HexArgb);
    m_title->setStyleSheet(QStringLiteral("font-size: 12px; color: %1; opacity: 0.75;").arg(fg));
    m_value->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: 800; color: %1;").arg(fg));
    m_foot->setStyleSheet(QStringLiteral("font-size: 11px; color: %1; opacity: 0.55;").arg(fg));
    m_trend->setBackgroundColor(backgroundColor());
    m_trend->setForegroundColor(foregroundColor());
    m_trend->setAccentColor(accentColor());
    m_trend->setCornerRadius(qMax(0, cornerRadius() - 4));
}
