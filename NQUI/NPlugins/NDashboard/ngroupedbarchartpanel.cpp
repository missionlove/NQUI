#include "ngroupedbarchartpanel.h"

#include <QVBoxLayout>

NGroupedBarChartPanel::NGroupedBarChartPanel(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_chart(new QtCharts::QChart()),
      m_view(new QtCharts::QChartView(m_chart, this)),
      m_series(new QtCharts::QBarSeries()),
      m_catAxis(new QtCharts::QBarCategoryAxis()),
      m_valAxis(new QtCharts::QValueAxis())
{
    m_palette << QColor(106, 149, 255) << QColor(120, 200, 180) << QColor(230, 170, 90) << QColor(200, 120, 220);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->addSeries(m_series);
    m_chart->addAxis(m_catAxis, Qt::AlignBottom);
    m_chart->addAxis(m_valAxis, Qt::AlignLeft);
    m_series->attachAxis(m_catAxis);
    m_series->attachAxis(m_valAxis);
    m_valAxis->setLabelsBrush(QBrush(QColor(200, 200, 200)));
    m_catAxis->setLabelsColor(QColor(200, 200, 200));
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setMinimumHeight(200);

    QVBoxLayout *v = new QVBoxLayout(this);
    v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    v->addWidget(m_view);
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this, v]() {
        v->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    });

    setCategories(QStringList() << QStringLiteral("Q1") << QStringLiteral("Q2") << QStringLiteral("Q3"));
    addGroup(QStringLiteral("产品 A"), QVector<qreal>() << 4 << 7 << 5);
    addGroup(QStringLiteral("产品 B"), QVector<qreal>() << 3 << 5 << 9);
    applyChromeStyle();
}

void NGroupedBarChartPanel::setCategories(const QStringList &categories)
{
    m_categories = categories;
    m_catAxis->clear();
    m_catAxis->append(m_categories);
}

void NGroupedBarChartPanel::clearGroups()
{
    while (!m_series->barSets().isEmpty()) {
        QtCharts::QBarSet *set = m_series->barSets().first();
        m_series->remove(set);
        delete set;
    }
}

void NGroupedBarChartPanel::addGroup(const QString &label, const QVector<qreal> &values)
{
    auto *set = new QtCharts::QBarSet(label);
    for (qreal v : values) {
        *set << v;
    }
    m_series->append(set);
    restyleBars();
    qreal maxV = 0;
    for (QtCharts::QBarSet *st : m_series->barSets()) {
        for (int i = 0; i < st->count(); ++i) {
            maxV = qMax(maxV, st->at(i));
        }
    }
    m_valAxis->setRange(0, qMax(1.0, maxV * 1.15));
}

void NGroupedBarChartPanel::setBarColors(const QVector<QColor> &colors)
{
    m_palette = colors;
    restyleBars();
}

void NGroupedBarChartPanel::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    const QColor bg = backgroundColor().darker(108);
    m_chart->setBackgroundBrush(QBrush(bg));
    m_chart->setPlotAreaBackgroundBrush(QBrush(bg));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_valAxis->setGridLineVisible(true);
    m_valAxis->setGridLineColor(accentColor().darker(140));
    m_catAxis->setLinePenColor(accentColor());
    m_valAxis->setLinePenColor(accentColor());
    restyleBars();
}

void NGroupedBarChartPanel::restyleBars()
{
    int idx = 0;
    for (QtCharts::QBarSet *set : m_series->barSets()) {
        const QColor c = m_palette.isEmpty() ? accentColor() : m_palette.at(idx % m_palette.size());
        set->setBrush(QBrush(c));
        set->setPen(QPen(c.darker(120)));
        ++idx;
    }
}
