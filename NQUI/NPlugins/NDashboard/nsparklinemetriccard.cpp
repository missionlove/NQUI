#include "nsparklinemetriccard.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <QtMath>

NSparklineMetricCard::NSparklineMetricCard(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_title(new QLabel(this)),
      m_value(new QLabel(this)),
      m_chart(new QtCharts::QChart()),
      m_view(new QtCharts::QChartView(m_chart, this)),
      m_series(new QtCharts::QLineSeries()),
      m_axX(new QtCharts::QValueAxis()),
      m_axY(new QtCharts::QValueAxis()),
      m_lineColor(QColor(106, 149, 255))
{
    m_title->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.75;"));
    m_value->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 800;"));
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(0, 0, 0, 0));
    m_chart->addSeries(m_series);
    m_chart->addAxis(m_axX, Qt::AlignBottom);
    m_chart->addAxis(m_axY, Qt::AlignLeft);
    m_series->attachAxis(m_axX);
    m_series->attachAxis(m_axY);
    m_axX->setVisible(false);
    m_axY->setVisible(false);
    m_axX->setGridLineVisible(false);
    m_axY->setGridLineVisible(false);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setFixedHeight(52);
    m_view->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *v = new QVBoxLayout(this);
    v->setSpacing(verticalSpacing() / 2);
    v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    v->addWidget(m_title);
    v->addWidget(m_value);
    v->addWidget(m_view);
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this, v]() {
        v->setSpacing(verticalSpacing() / 2);
        v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    });

    QVector<double> seed;
    for (int i = 0; i < 14; ++i) {
        seed.append(3 + qSin(i / 2.0));
    }
    setSparkline(seed);
    applyChromeStyle();
}

void NSparklineMetricCard::setTitle(const QString &title)
{
    m_title->setText(title);
}

void NSparklineMetricCard::setValueText(const QString &value)
{
    m_value->setText(value);
}

void NSparklineMetricCard::setSparkline(const QVector<double> &y)
{
    m_series->clear();
    for (int i = 0; i < y.size(); ++i) {
        m_series->append(i, y.at(i));
    }
    syncSparkAxes();
}

void NSparklineMetricCard::setLineColor(const QColor &c)
{
    m_lineColor = c;
    m_series->setPen(QPen(m_lineColor, 2));
}

void NSparklineMetricCard::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    const QString fg = foregroundColor().name(QColor::HexArgb);
    m_title->setStyleSheet(QStringLiteral("font-size: 12px; color: %1;").arg(fg));
    m_value->setStyleSheet(QStringLiteral("font-size: 22px; font-weight: 800; color: %1;").arg(fg));
    const QColor bg = backgroundColor().darker(108);
    m_chart->setBackgroundBrush(QBrush(bg));
    m_chart->setPlotAreaBackgroundBrush(QBrush(bg));
    m_chart->setPlotAreaBackgroundVisible(true);
    setLineColor(m_lineColor);
}

void NSparklineMetricCard::syncSparkAxes()
{
    if (m_series->count() == 0) {
        m_axX->setRange(0, 1);
        m_axY->setRange(0, 1);
        return;
    }
    qreal minY = m_series->at(0).y();
    qreal maxY = minY;
    for (int i = 0; i < m_series->count(); ++i) {
        const qreal yy = m_series->at(i).y();
        minY = qMin(minY, yy);
        maxY = qMax(maxY, yy);
    }
    const qreal pad = (maxY - minY) * 0.25 + 0.1;
    m_axX->setRange(0, qMax(1.0, m_series->count() - 1.0));
    m_axY->setRange(minY - pad, maxY + pad);
}
