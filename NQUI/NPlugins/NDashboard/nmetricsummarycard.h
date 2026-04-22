#ifndef NMETRICSUMMARYCARD_H
#define NMETRICSUMMARYCARD_H

#include "ndashboardwidgetbase.h"

class QLabel;
class NTrendIndicator;

class NDASHBOARD_EXPORT NMetricSummaryCard : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NMetricSummaryCard(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setValueText(const QString &value);
    void setFootnote(const QString &text);

    NTrendIndicator *trendWidget();

protected:
    void applyChromeStyle() override;

private:
    QLabel *m_title;
    QLabel *m_value;
    QLabel *m_foot;
    NTrendIndicator *m_trend;
};

#endif // NMETRICSUMMARYCARD_H
