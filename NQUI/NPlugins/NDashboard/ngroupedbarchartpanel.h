#ifndef NGROUPEDBARCHARTPANEL_H
#define NGROUPEDBARCHARTPANEL_H

#include "ndashboardwidgetbase.h"

#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

#include <QVector>

class NDASHBOARD_EXPORT NGroupedBarChartPanel : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NGroupedBarChartPanel(QWidget *parent = nullptr);

    void setCategories(const QStringList &categories);
    void clearGroups();
    void addGroup(const QString &label, const QVector<qreal> &values);

    void setBarColors(const QVector<QColor> &colors);

protected:
    void applyChromeStyle() override;

private:
    void restyleBars();

    QtCharts::QChart *m_chart;
    QtCharts::QChartView *m_view;
    QtCharts::QBarSeries *m_series;
    QtCharts::QBarCategoryAxis *m_catAxis;
    QtCharts::QValueAxis *m_valAxis;
    QStringList m_categories;
    QVector<QColor> m_palette;
};

#endif // NGROUPEDBARCHARTPANEL_H
