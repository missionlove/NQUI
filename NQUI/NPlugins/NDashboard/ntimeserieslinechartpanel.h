#ifndef NTIMESERIESLINECHARTPANEL_H
#define NTIMESERIESLINECHARTPANEL_H

#include "ndashboardwidgetbase.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>

#include <QStringList>
#include <QVector>

class NDASHBOARD_EXPORT NTimeSeriesLineChartPanel : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NTimeSeriesLineChartPanel(QWidget *parent = nullptr);

    void setPoints(const QVector<QPointF> &points);
    void setSecondaryPoints(const QVector<QPointF> &points);
    void clearSecondaryPoints();

    /// X 轴使用分类标签（如 12 个月）；点的 x 坐标应为 0..n-1。传入空列表恢复数值轴。
    void setXAxisCategories(const QStringList &categories);
    QStringList xAxisCategories() const;

    void setLineColor(const QColor &c);
    QColor lineColor() const;
    void setSecondaryLineColor(const QColor &c);
    QColor secondaryLineColor() const;

    void setPlotBackgroundColor(const QColor &c);
    QColor plotBackgroundColor() const;

    void setGridVisible(bool on);
    bool gridVisible() const;
    void setVerticalGridVisible(bool on);
    bool verticalGridVisible() const;
    void setHorizontalGridVisible(bool on);
    bool horizontalGridVisible() const;

    void setLineWidth(int w);

protected:
    void applyChromeStyle() override;

private:
    void syncAxes();
    void recreateXAxis();
    void styleXAxis();
    void showPointTip(const QString &seriesName, const QPointF &point, bool state);

    QtCharts::QChart *m_chart;
    QtCharts::QChartView *m_view;
    QtCharts::QLineSeries *m_series;
    QtCharts::QLineSeries *m_seriesSecondary;
    QtCharts::QAbstractAxis *m_axisX;
    QtCharts::QValueAxis *m_axisY;
    QStringList m_xCategories;
    QColor m_lineColor;
    QColor m_secondaryLineColor;
    QColor m_plotBg;
    bool m_gridVisible;
    bool m_verticalGridVisible;
    bool m_horizontalGridVisible;
    int m_lineWidth;
};

#endif // NTIMESERIESLINECHARTPANEL_H
