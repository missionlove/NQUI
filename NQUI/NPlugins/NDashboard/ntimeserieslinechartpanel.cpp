#include "ntimeserieslinechartpanel.h"

#include <QBrush>
#include <QToolTip>
#include <QVBoxLayout>
#include <QtCharts/QBarCategoryAxis>
#include <QtMath>

NTimeSeriesLineChartPanel::NTimeSeriesLineChartPanel(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_chart(new QtCharts::QChart()),
      m_view(new QtCharts::QChartView(m_chart, this)),
      m_series(new QtCharts::QLineSeries()),
      m_seriesSecondary(new QtCharts::QLineSeries()),
      m_axisX(new QtCharts::QValueAxis()),
      m_axisY(new QtCharts::QValueAxis()),
      m_lineColor(QColor(106, 149, 255)),
      m_secondaryLineColor(QColor(63, 211, 179)),
      m_plotBg(QColor(18, 20, 26)),
      m_gridVisible(true),
      m_verticalGridVisible(true),
      m_horizontalGridVisible(true),
      m_lineWidth(2)
{
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(8, 8, 8, 8));
    m_chart->addSeries(m_series);
    m_chart->addSeries(m_seriesSecondary);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);
    m_seriesSecondary->attachAxis(m_axisX);
    m_seriesSecondary->attachAxis(m_axisY);
    m_axisY->setLabelsBrush(QBrush(QColor(200, 200, 200)));
    styleXAxis();
    m_axisY->setGridLineVisible(m_gridVisible);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setMinimumHeight(180);

    QVBoxLayout *v = new QVBoxLayout(this);
    v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    v->addWidget(m_view);
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this, v]() {
        v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    });
    connect(m_series, &QtCharts::QLineSeries::hovered, this, [this](const QPointF &point, bool state) {
        showPointTip(QStringLiteral("Marketing Sales"), point, state);
    });
    connect(m_seriesSecondary, &QtCharts::QLineSeries::hovered, this, [this](const QPointF &point, bool state) {
        showPointTip(QStringLiteral("Case Sales"), point, state);
    });

    QVector<QPointF> seed;
    for (int i = 0; i < 12; ++i) {
        seed.append(QPointF(i, 10 + qSin(i / 2.0) * 4 + i * 0.4));
    }
    setPoints(seed);
    applyChromeStyle();
}

void NTimeSeriesLineChartPanel::setXAxisCategories(const QStringList &categories)
{
    if (m_xCategories == categories)
        return;
    m_xCategories = categories;
    recreateXAxis();
    syncAxes();
}

QStringList NTimeSeriesLineChartPanel::xAxisCategories() const
{
    return m_xCategories;
}

void NTimeSeriesLineChartPanel::recreateXAxis()
{
    m_chart->removeAxis(m_axisX);
    delete m_axisX;
    m_axisX = nullptr;

    if (m_xCategories.isEmpty()) {
        m_axisX = new QtCharts::QValueAxis();
    } else {
        auto *cat = new QtCharts::QBarCategoryAxis();
        for (const QString &c : m_xCategories)
            cat->append(c);
        m_axisX = cat;
    }

    styleXAxis();
    m_axisX->setLinePenColor(accentColor());
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);
}

void NTimeSeriesLineChartPanel::styleXAxis()
{
    m_axisX->setLabelsBrush(QBrush(QColor(200, 200, 200)));
    m_axisX->setGridLineVisible(m_verticalGridVisible);
}

void NTimeSeriesLineChartPanel::setPoints(const QVector<QPointF> &points)
{
    m_series->clear();
    for (const QPointF &p : points) {
        m_series->append(p.x(), p.y());
    }
    syncAxes();
}

void NTimeSeriesLineChartPanel::setSecondaryPoints(const QVector<QPointF> &points)
{
    m_seriesSecondary->clear();
    for (const QPointF &p : points) {
        m_seriesSecondary->append(p.x(), p.y());
    }
    syncAxes();
}

void NTimeSeriesLineChartPanel::clearSecondaryPoints()
{
    m_seriesSecondary->clear();
    syncAxes();
}

void NTimeSeriesLineChartPanel::setLineColor(const QColor &c)
{
    m_lineColor = c;
    m_series->setPen(QPen(m_lineColor, m_lineWidth));
}

QColor NTimeSeriesLineChartPanel::lineColor() const
{
    return m_lineColor;
}

void NTimeSeriesLineChartPanel::setSecondaryLineColor(const QColor &c)
{
    m_secondaryLineColor = c;
    m_seriesSecondary->setPen(QPen(m_secondaryLineColor, m_lineWidth));
}

QColor NTimeSeriesLineChartPanel::secondaryLineColor() const
{
    return m_secondaryLineColor;
}

void NTimeSeriesLineChartPanel::setPlotBackgroundColor(const QColor &c)
{
    m_plotBg = c;
    m_chart->setBackgroundBrush(QBrush(m_plotBg));
    m_chart->setPlotAreaBackgroundBrush(QBrush(m_plotBg.darker(108)));
    m_chart->setPlotAreaBackgroundVisible(true);
}

QColor NTimeSeriesLineChartPanel::plotBackgroundColor() const
{
    return m_plotBg;
}

void NTimeSeriesLineChartPanel::setGridVisible(bool on)
{
    m_gridVisible = on;
    m_verticalGridVisible = on;
    m_horizontalGridVisible = on;
    if (m_axisX)
        m_axisX->setGridLineVisible(on);
    m_axisY->setGridLineVisible(on);
}

bool NTimeSeriesLineChartPanel::gridVisible() const
{
    return m_gridVisible;
}

void NTimeSeriesLineChartPanel::setVerticalGridVisible(bool on)
{
    m_verticalGridVisible = on;
    if (m_axisX)
        m_axisX->setGridLineVisible(on);
}

bool NTimeSeriesLineChartPanel::verticalGridVisible() const
{
    return m_verticalGridVisible;
}

void NTimeSeriesLineChartPanel::setHorizontalGridVisible(bool on)
{
    m_horizontalGridVisible = on;
    m_axisY->setGridLineVisible(on);
}

bool NTimeSeriesLineChartPanel::horizontalGridVisible() const
{
    return m_horizontalGridVisible;
}

void NTimeSeriesLineChartPanel::setLineWidth(int w)
{
    w = qMax(1, w);
    m_lineWidth = w;
    m_series->setPen(QPen(m_lineColor, m_lineWidth));
    m_seriesSecondary->setPen(QPen(m_secondaryLineColor, m_lineWidth));
}

void NTimeSeriesLineChartPanel::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    setPlotBackgroundColor(m_plotBg);
    if (m_axisX)
        m_axisX->setLinePenColor(accentColor());
    m_axisY->setLinePenColor(accentColor());
    setLineColor(m_lineColor);
    setSecondaryLineColor(m_secondaryLineColor);
}

void NTimeSeriesLineChartPanel::syncAxes()
{
    const bool primaryEmpty = (m_series->count() == 0);
    const bool secondaryEmpty = (m_seriesSecondary->count() == 0);
    if (primaryEmpty && secondaryEmpty) {
        if (auto *vx = qobject_cast<QtCharts::QValueAxis *>(m_axisX))
            vx->setRange(0, 1);
        m_axisY->setRange(0, 1);
        return;
    }
    qreal minX = 0.0;
    qreal maxX = 0.0;
    qreal minY = 0.0;
    qreal maxY = 0.0;
    bool inited = false;
    auto accumulate = [&](QtCharts::QLineSeries *series) {
        for (int i = 0; i < series->count(); ++i) {
            const QPointF p = series->at(i);
            if (!inited) {
                minX = maxX = p.x();
                minY = maxY = p.y();
                inited = true;
            } else {
                minX = qMin(minX, p.x());
                maxX = qMax(maxX, p.x());
                minY = qMin(minY, p.y());
                maxY = qMax(maxY, p.y());
            }
        }
    };
    if (!primaryEmpty)
        accumulate(m_series);
    if (!secondaryEmpty)
        accumulate(m_seriesSecondary);
    if (qFuzzyCompare(minY, maxY)) {
        maxY = minY + 1;
    }
    const qreal padY = (maxY - minY) * 0.12 + 0.5;
    if (auto *vx = qobject_cast<QtCharts::QValueAxis *>(m_axisX)) {
        if (qFuzzyCompare(minX, maxX)) {
            maxX = minX + 1;
        }
        vx->setRange(minX, maxX);
    }
    m_axisY->setRange(minY - padY, maxY + padY);
}

void NTimeSeriesLineChartPanel::showPointTip(const QString &seriesName, const QPointF &point, bool state)
{
    if (!state) {
        QToolTip::hideText();
        return;
    }

    QString prefix;
    if (!m_xCategories.isEmpty()) {
        const int idx = qRound(point.x());
        if (idx >= 0 && idx < m_xCategories.size()) {
            prefix = m_xCategories.at(idx) + QStringLiteral(": ");
        }
    }

    const qreal rounded = qRound(point.y());
    const bool useInt = qAbs(point.y() - rounded) < 0.05;
    const QString valueText = useInt
        ? QString::number(static_cast<int>(rounded))
        : QString::number(point.y(), 'f', 1);
    const QString tipText = seriesName + QStringLiteral("  ") + prefix + valueText;

    const QPointF chartPos = m_chart->mapToPosition(point, m_series);
    const QPoint globalPos = m_view->mapToGlobal(chartPos.toPoint() + QPoint(10, -24));
    QToolTip::showText(globalPos, tipText, m_view);
}
