#include "nstockchartpanel.h"

#include <QBrush>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QCursor>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QWheelEvent>

#include <QtCharts/QAbstractAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QCandlestickSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

using QtCharts::QBarCategoryAxis;
using QtCharts::QCandlestickSeries;
using QtCharts::QCandlestickSet;
using QtCharts::QChart;
using QtCharts::QChartView;
using QtCharts::QLineSeries;
using QtCharts::QValueAxis;

namespace {

class WheelZoomChartView : public QChartView
{
public:
    explicit WheelZoomChartView(QChart *chart, QWidget *parent = nullptr)
        : QChartView(chart, parent)
    {
    }

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        if (!chart()) {
            QChartView::wheelEvent(event);
            return;
        }
        const int dy = event->angleDelta().y();
        if (dy == 0) {
            QChartView::wheelEvent(event);
            return;
        }
        const qreal factor = (dy > 0) ? 1.12 : (1.0 / 1.12);
        chart()->zoom(factor);
        event->accept();
    }
};

} // namespace

NStockChartPanel::NStockChartPanel(QWidget *parent)
    : QWidget(parent),
      m_statusLabel(new QLabel(this)),
      m_modeBar(new QWidget(this)),
      m_modeGroup(new QButtonGroup(this)),
      m_chart(new QChart()),
      m_view(new WheelZoomChartView(m_chart, this)),
      m_candles(new QCandlestickSeries()),
      m_line(new QLineSeries()),
      m_prevCloseLine(new QLineSeries()),
      m_axisXCat(nullptr),
      m_axisXVal(nullptr),
      m_axisY(nullptr),
      m_net(new QNetworkAccessManager(this)),
      m_modeDebounce(new QTimer(this)),
      m_mode(ChartMode::Intraday)
{
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(4, 4, 4, 4));
    m_chart->setAnimationOptions(QChart::NoAnimation);
    m_chart->setBackgroundBrush(QBrush(QColor(18, 20, 26)));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->setPlotAreaBackgroundBrush(QBrush(QColor(18, 20, 26)));

    m_candles->setIncreasingColor(QColor(220, 78, 96));
    m_candles->setDecreasingColor(QColor(72, 185, 119));
    m_candles->setBodyOutlineVisible(true);
    m_line->setPen(QPen(QColor(106, 149, 255), 2));
    m_prevCloseLine->setName(QStringLiteral("昨收"));
    m_prevCloseLine->setPen(QPen(QColor(180, 188, 210), 1, Qt::DashLine));

    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    m_view->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
    m_view->setMinimumHeight(160);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setMouseTracking(true);

    m_modeDebounce->setSingleShot(true);
    m_modeDebounce->setInterval(45);
    connect(m_modeDebounce, &QTimer::timeout, this, [this]() {
        if (m_symbol.size() == 6) {
            launchChartFetch();
        }
    });

    m_statusLabel->setStyleSheet(QStringLiteral("color: #8f9cb4; font-size: 11px;"));
    m_statusLabel->setText(QStringLiteral("图表"));

    auto *modeLayout = new QHBoxLayout(m_modeBar);
    modeLayout->setContentsMargins(0, 0, 0, 0);
    modeLayout->setSpacing(4);

    const struct {
        QString text;
        ChartMode mode;
    } modes[] = {{QStringLiteral("分时"), ChartMode::Intraday},
                 {QStringLiteral("日K"), ChartMode::Daily},
                 {QStringLiteral("周K"), ChartMode::Weekly},
                 {QStringLiteral("月K"), ChartMode::Monthly},
                 {QStringLiteral("5日"), ChartMode::FiveDay}};

    m_modeGroup->setExclusive(true);
    for (const auto &m : modes) {
        auto *b = new QToolButton(m_modeBar);
        b->setText(m.text);
        b->setCheckable(true);
        b->setAutoExclusive(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setProperty("chartMode", static_cast<int>(m.mode));
        b->setStyleSheet(QStringLiteral(
            "QToolButton { border: 1px solid #31384a; border-radius: 6px; background: #202636; color: #b6c3dc; "
            "padding: 4px 8px; font-size: 11px; }"
            "QToolButton:checked { background: #2b3550; color: #e7eeff; border-color: #4a6cff; }"
            "QToolButton:hover { background: #2a3144; color: #dce6ff; }"));
        m_modeGroup->addButton(b);
        modeLayout->addWidget(b);
        if (m.mode == ChartMode::Intraday) {
            b->setChecked(true);
        }
    }
    modeLayout->addStretch(1);

    connect(m_modeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &NStockChartPanel::onModeButtonClicked);
    connect(m_net, &QNetworkAccessManager::finished, this, &NStockChartPanel::onNetworkFinished);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(6);
    root->addWidget(m_modeBar);
    root->addWidget(m_view, 1);
    root->addWidget(m_statusLabel);

    setMinimumWidth(200);
}

void NStockChartPanel::setSymbol(const QString &normalizedSymbol)
{
    const QString s = normalizedSymbol.trimmed();
    if (s == m_symbol) {
        return;
    }
    m_symbol = s;
    if (m_symbol.size() != 6) {
        clearChartContent();
        setStatus(QStringLiteral("无效代码"));
        return;
    }
    launchChartFetch();
}

void NStockChartPanel::reloadIntradayIfNeeded()
{
    if (m_mode != ChartMode::Intraday || m_symbol.size() != 6) {
        return;
    }
    launchChartFetch();
}

void NStockChartPanel::onModeButtonClicked(QAbstractButton *button)
{
    if (!button) {
        return;
    }
    const int raw = button->property("chartMode").toInt();
    m_mode = static_cast<ChartMode>(raw);
    if (m_symbol.size() == 6) {
        scheduleModeFetch();
    }
}

void NStockChartPanel::scheduleModeFetch()
{
    m_modeDebounce->stop();
    m_modeDebounce->start();
}

QString NStockChartPanel::secIdForSymbol(const QString &symbol)
{
    if (symbol.size() != 6) {
        return QString();
    }
    bool ok = false;
    symbol.toInt(&ok);
    if (!ok) {
        return QString();
    }
    const bool isShanghai = symbol.startsWith(QLatin1Char('6')) || symbol.startsWith(QLatin1Char('9'));
    return QStringLiteral("%1.%2").arg(isShanghai ? QStringLiteral("1") : QStringLiteral("0"), symbol);
}

int NStockChartPanel::kltForMode(ChartMode m) const
{
    switch (m) {
    case ChartMode::Daily:
    case ChartMode::FiveDay:
        return 101;
    case ChartMode::Weekly:
        return 102;
    case ChartMode::Monthly:
        return 103;
    default:
        return 101;
    }
}

int NStockChartPanel::klineFetchCount(ChartMode m) const
{
    switch (m) {
    case ChartMode::FiveDay:
        return 32;
    case ChartMode::Daily:
        return 72;
    case ChartMode::Weekly:
        return 52;
    case ChartMode::Monthly:
        return 48;
    default:
        return 60;
    }
}

void NStockChartPanel::clearChartContent()
{
    disconnect(m_candles, &QCandlestickSeries::hovered, this, &NStockChartPanel::onCandleHovered);
    m_kHoverDates.clear();
    m_kHoverOpens.clear();
    m_kHoverHighs.clear();
    m_kHoverLows.clear();
    m_kHoverCloses.clear();

    m_line->clear();
    m_prevCloseLine->clear();
    m_candles->clear();
    if (m_chart->series().contains(m_prevCloseLine)) {
        m_chart->removeSeries(m_prevCloseLine);
    }
    if (m_chart->series().contains(m_line)) {
        m_chart->removeSeries(m_line);
    }
    if (m_chart->series().contains(m_candles)) {
        m_chart->removeSeries(m_candles);
    }
    const QList<QtCharts::QAbstractAxis *> axes = m_chart->axes();
    for (QtCharts::QAbstractAxis *a : axes) {
        m_chart->removeAxis(a);
        delete a;
    }
    m_axisXCat = nullptr;
    m_axisXVal = nullptr;
    m_axisY = nullptr;
}

void NStockChartPanel::setStatus(const QString &text)
{
    m_statusLabel->setText(text);
}

void NStockChartPanel::launchChartFetch()
{
    m_modeDebounce->stop();

    if (m_symbol.size() != 6) {
        return;
    }
    const QString secId = secIdForSymbol(m_symbol);
    if (secId.isEmpty()) {
        setStatus(QStringLiteral("无效代码"));
        return;
    }

    if (m_pendingReply) {
        m_pendingReply->abort();
        m_pendingReply.clear();
    }

    for (auto *btn : m_modeGroup->buttons()) {
        btn->setEnabled(false);
    }
    setStatus(QStringLiteral("加载中…"));

    QString url;
    if (m_mode == ChartMode::Intraday) {
        url = QStringLiteral("http://push2his.eastmoney.com/api/qt/stock/trends/get?"
                             "secid=%1&fields1=f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13")
                  .arg(secId);
    } else {
        const int klt = kltForMode(m_mode);
        const int lmt = klineFetchCount(m_mode);
        url = QStringLiteral("http://push2his.eastmoney.com/api/qt/stock/kline/get?"
                             "secid=%1&fields1=f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13"
                             "&fields2=f51,f52,f53,f54,f55,f56,f57&klt=%2&fqt=1&lmt=%3&end=20500101"
                             "&ut=fa5fd1943c7b386f172d6893dbfba10b&rtntype=6")
                  .arg(secId)
                  .arg(klt)
                  .arg(lmt);
    }

    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0"));
    QNetworkReply *reply = m_net->get(req);
    reply->setProperty("chartSym", m_symbol);
    reply->setProperty("chartMode", static_cast<int>(m_mode));
    m_pendingReply = reply;
}

void NStockChartPanel::showIntradayLine(const QVector<double> &prices, double prevClose)
{
    m_view->setUpdatesEnabled(false);
    clearChartContent();
    if (prices.isEmpty()) {
        m_view->setUpdatesEnabled(true);
        setStatus(QStringLiteral("分时数据为空"));
        return;
    }

    double yMin = prices.first();
    double yMax = prices.first();
    for (double p : prices) {
        yMin = qMin(yMin, p);
        yMax = qMax(yMax, p);
    }
    if (prevClose > 0.0) {
        yMin = qMin(yMin, prevClose);
        yMax = qMax(yMax, prevClose);
    }
    const double pad = (yMax - yMin) * 0.08 + 1e-4;
    yMin -= pad;
    yMax += pad;

    m_axisY = new QValueAxis();
    m_axisY->setLabelsBrush(QBrush(QColor(200, 200, 200)));
    m_axisY->setGridLineVisible(true);
    m_axisY->setRange(yMin, yMax);
    m_axisY->setLabelFormat(QStringLiteral("%.2f"));

    m_axisXVal = new QValueAxis();
    m_axisXVal->setLabelsBrush(QBrush(QColor(200, 200, 200)));
    m_axisXVal->setGridLineVisible(false);
    m_axisXVal->setRange(0, qMax(1, prices.size() - 1));
    m_axisXVal->setLabelFormat(QStringLiteral("%d"));

    m_chart->addAxis(m_axisXVal, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    for (int i = 0; i < prices.size(); ++i) {
        m_line->append(i, prices.at(i));
    }
    m_chart->addSeries(m_line);
    m_line->attachAxis(m_axisXVal);
    m_line->attachAxis(m_axisY);

    if (prevClose > 0.0) {
        const qreal x1 = static_cast<qreal>(qMax(0, prices.size() - 1));
        m_prevCloseLine->append(0.0, prevClose);
        m_prevCloseLine->append(x1, prevClose);
        m_chart->addSeries(m_prevCloseLine);
        m_prevCloseLine->attachAxis(m_axisXVal);
        m_prevCloseLine->attachAxis(m_axisY);
    }

    m_chart->setTitle(QString());
    setStatus(prevClose > 0.0 ? QStringLiteral("分时 · 昨收 %1 · 东财").arg(QLocale().toString(prevClose, 'f', 2))
                              : QStringLiteral("分时 · 东财"));

    m_view->setUpdatesEnabled(true);
}

void NStockChartPanel::showCandles(const QStringList &categoryLabels,
                                    const QVector<double> &opens,
                                    const QVector<double> &highs,
                                    const QVector<double> &lows,
                                    const QVector<double> &closes)
{
    const int n = qMin(categoryLabels.size(), opens.size());
    m_view->setUpdatesEnabled(false);
    clearChartContent();
    if (n <= 0) {
        m_view->setUpdatesEnabled(true);
        setStatus(QStringLiteral("K线数据为空"));
        return;
    }

    m_kHoverDates.reserve(n);
    for (int i = 0; i < n; ++i) {
        m_kHoverDates.append(categoryLabels.at(i));
    }
    m_kHoverOpens = opens;
    m_kHoverHighs = highs;
    m_kHoverLows = lows;
    m_kHoverCloses = closes;

    double yMin = lows.first();
    double yMax = highs.first();
    for (int i = 0; i < n; ++i) {
        yMin = qMin(yMin, lows.at(i));
        yMax = qMax(yMax, highs.at(i));
    }
    const double pad = (yMax - yMin) * 0.06 + 1e-4;
    yMin -= pad;
    yMax += pad;

    m_axisXCat = new QBarCategoryAxis();
    m_axisXCat->setLabelsBrush(QBrush(QColor(200, 200, 200)));
    m_axisXCat->setGridLineVisible(false);
    for (int i = 0; i < n; ++i) {
        const QString &full = categoryLabels.at(i);
        m_axisXCat->append(full.length() >= 10 ? full.mid(5) : full);
    }

    m_axisY = new QValueAxis();
    m_axisY->setLabelsBrush(QBrush(QColor(200, 200, 200)));
    m_axisY->setGridLineVisible(false);
    m_axisY->setRange(yMin, yMax);
    m_axisY->setLabelFormat(QStringLiteral("%.2f"));

    m_chart->addAxis(m_axisXCat, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    QList<QCandlestickSet *> batch;
    batch.reserve(n);
    for (int i = 0; i < n; ++i) {
        auto *set = new QCandlestickSet(static_cast<qreal>(i));
        set->setOpen(opens.at(i));
        set->setHigh(highs.at(i));
        set->setLow(lows.at(i));
        set->setClose(closes.at(i));
        batch.append(set);
    }
    m_candles->append(batch);
    connect(m_candles, &QCandlestickSeries::hovered, this, &NStockChartPanel::onCandleHovered,
            Qt::UniqueConnection);
    m_chart->addSeries(m_candles);
    m_candles->attachAxis(m_axisXCat);
    m_candles->attachAxis(m_axisY);

    QString modeName;
    switch (m_mode) {
    case ChartMode::Daily:
        modeName = QStringLiteral("日K");
        break;
    case ChartMode::Weekly:
        modeName = QStringLiteral("周K");
        break;
    case ChartMode::Monthly:
        modeName = QStringLiteral("月K");
        break;
    case ChartMode::FiveDay:
        modeName = QStringLiteral("5日");
        break;
    default:
        modeName = QStringLiteral("K线");
        break;
    }
    setStatus(QStringLiteral("%1 · 东财").arg(modeName));

    m_view->setUpdatesEnabled(true);
}

void NStockChartPanel::onNetworkFinished(QNetworkReply *reply)
{
    const bool active = (reply == m_pendingReply);
    if (active) {
        m_pendingReply.clear();
    }

    const auto err = reply->error();
    const QString errString = reply->errorString();
    const QString propSym = reply->property("chartSym").toString();
    const int propMode = reply->property("chartMode").toInt();
    const QByteArray raw = reply->readAll();

    reply->deleteLater();

    for (auto *btn : m_modeGroup->buttons()) {
        btn->setEnabled(m_pendingReply.isNull());
    }

    if (err == QNetworkReply::OperationCanceledError) {
        return;
    }

    if (!active) {
        return;
    }

    if (err != QNetworkReply::NoError) {
        if (propSym == m_symbol) {
            clearChartContent();
            setStatus(QStringLiteral("网络错误: %1").arg(errString));
        }
        return;
    }

    const QString sym = propSym;
    const int modeInt = propMode;
    if (sym != m_symbol || modeInt != static_cast<int>(m_mode)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(raw);
    if (!doc.isObject()) {
        clearChartContent();
        setStatus(QStringLiteral("数据解析失败"));
        return;
    }
    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("rc")).toInt() != 0) {
        clearChartContent();
        setStatus(QStringLiteral("接口返回错误"));
        return;
    }

    if (m_mode == ChartMode::Intraday) {
        const QJsonArray arr = root.value(QStringLiteral("data")).toArray();
        QVector<double> prices;
        prices.reserve(arr.size());
        double prevClose = 0.0;
        for (const QJsonValue &v : arr) {
            const QJsonObject o = v.toObject();
            if (prevClose <= 0.0) {
                const double f8 = o.value(QStringLiteral("f8")).toDouble();
                if (f8 > 0.0) {
                    prevClose = f8 / 1000.0;
                }
            }
            const double px = o.value(QStringLiteral("f6")).toDouble() / 100.0;
            if (px > 0.0) {
                prices.append(px);
            }
        }
        showIntradayLine(prices, prevClose);
        return;
    }

    const QJsonObject data = root.value(QStringLiteral("data")).toObject();
    const QJsonArray klines = data.value(QStringLiteral("klines")).toArray();
    QStringList labels;
    QVector<double> opens, highs, lows, closes;
    for (const QJsonValue &v : klines) {
        const QString line = v.toString();
        const QStringList p = line.split(QLatin1Char(','));
        if (p.size() < 6) {
            continue;
        }
        const QString date = p.at(0).trimmed();
        const double o = p.at(1).toDouble();
        const double c = p.at(2).toDouble();
        const double h = p.at(3).toDouble();
        const double l = p.at(4).toDouble();
        labels.append(date);
        opens.append(o);
        closes.append(c);
        highs.append(h);
        lows.append(l);
    }

    if (m_mode == ChartMode::FiveDay && labels.size() > 5) {
        const int skip = labels.size() - 5;
        labels = labels.mid(skip);
        opens = opens.mid(skip);
        closes = closes.mid(skip);
        highs = highs.mid(skip);
        lows = lows.mid(skip);
    }

    showCandles(labels, opens, highs, lows, closes);
}

void NStockChartPanel::onCandleHovered(bool status, QtCharts::QCandlestickSet *set)
{
    if (!status || !set) {
        QToolTip::hideText();
        return;
    }
    const int i = static_cast<int>(set->timestamp());
    if (i < 0 || i >= m_kHoverDates.size() || i >= m_kHoverCloses.size()) {
        return;
    }
    const QString tip = QStringLiteral("时间: %1\n开 %2  高 %3  低 %4  收 %5")
                            .arg(m_kHoverDates.at(i))
                            .arg(QLocale().toString(m_kHoverOpens.at(i), 'f', 2))
                            .arg(QLocale().toString(m_kHoverHighs.at(i), 'f', 2))
                            .arg(QLocale().toString(m_kHoverLows.at(i), 'f', 2))
                            .arg(QLocale().toString(m_kHoverCloses.at(i), 'f', 2));
    QToolTip::showText(QCursor::pos(), tip, m_view);
}
