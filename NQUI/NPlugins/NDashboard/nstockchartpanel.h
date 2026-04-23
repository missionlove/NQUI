#ifndef NSTOCKCHARTPANEL_H
#define NSTOCKCHARTPANEL_H

#include "NDashboard_global.h"

#include <QPointer>
#include <QString>
#include <QVector>
#include <QWidget>

class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QButtonGroup;
class QAbstractButton;
class QTimer;

namespace QtCharts {
class QChart;
class QChartView;
class QCandlestickSeries;
class QCandlestickSet;
class QLineSeries;
class QBarCategoryAxis;
class QValueAxis;
}

class NDASHBOARD_EXPORT NStockChartPanel : public QWidget
{
    Q_OBJECT
public:
    explicit NStockChartPanel(QWidget *parent = nullptr);

    void setSymbol(const QString &normalizedSymbol);
    /// 与行情刷新同步：仅在「分时」模式下重新拉取分时数据
    void reloadIntradayIfNeeded();

private slots:
    void onModeButtonClicked(QAbstractButton *button);
    void onNetworkFinished(QNetworkReply *reply);
    void onCandleHovered(bool status, QtCharts::QCandlestickSet *set);

private:
    enum class ChartMode { Intraday, Daily, Weekly, Monthly, FiveDay };

    static QString secIdForSymbol(const QString &symbol);
    void launchChartFetch();
    void scheduleModeFetch();
    void clearChartContent();
    /// categoryLabels：每根 K 对应的日期/时间字符串（与东财 klines 首字段一致，用于 X 轴简写与悬停）
    void showCandles(const QStringList &categoryLabels,
                     const QVector<double> &opens,
                     const QVector<double> &highs,
                     const QVector<double> &lows,
                     const QVector<double> &closes);
    void showIntradayLine(const QVector<double> &prices, double prevClose);
    void setStatus(const QString &text);
    int kltForMode(ChartMode m) const;
    int klineFetchCount(ChartMode m) const;

    QLabel *m_statusLabel;
    QWidget *m_modeBar;
    QButtonGroup *m_modeGroup;
    QtCharts::QChart *m_chart;
    QtCharts::QChartView *m_view;
    QtCharts::QCandlestickSeries *m_candles;
    QtCharts::QLineSeries *m_line;
    QtCharts::QLineSeries *m_prevCloseLine;
    QtCharts::QBarCategoryAxis *m_axisXCat;
    QtCharts::QValueAxis *m_axisXVal;
    QtCharts::QValueAxis *m_axisY;

    QNetworkAccessManager *m_net;
    QTimer *m_modeDebounce;
    QPointer<QNetworkReply> m_pendingReply;
    QString m_symbol;
    ChartMode m_mode;

    QVector<QString> m_kHoverDates;
    QVector<double> m_kHoverOpens;
    QVector<double> m_kHoverHighs;
    QVector<double> m_kHoverLows;
    QVector<double> m_kHoverCloses;
};

#endif // NSTOCKCHARTPANEL_H
