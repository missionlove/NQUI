#ifndef NSPARKLINEMETRICCARD_H
#define NSPARKLINEMETRICCARD_H

#include "ndashboardwidgetbase.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

#include <QVector>

class QLabel;

class NDASHBOARD_EXPORT NSparklineMetricCard : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NSparklineMetricCard(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setValueText(const QString &value);
    void setSparkline(const QVector<double> &y);

    void setLineColor(const QColor &c);

protected:
    void applyChromeStyle() override;

private:
    void syncSparkAxes();

    QLabel *m_title;
    QLabel *m_value;
    QtCharts::QChart *m_chart;
    QtCharts::QChartView *m_view;
    QtCharts::QLineSeries *m_series;
    QtCharts::QValueAxis *m_axX;
    QtCharts::QValueAxis *m_axY;
    QColor m_lineColor;
};

#endif // NSPARKLINEMETRICCARD_H
