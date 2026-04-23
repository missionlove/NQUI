#include "nstockquotecard.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTextCodec>
#include <QTimer>
#include <QToolButton>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QSplitter>
#include <QWidget>

#include "nstockchartpanel.h"

NStockQuoteCard::NStockQuoteCard(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_title(new QLabel(this)),
      m_price(new QLabel(this)),
      m_change(new QLabel(this)),
      m_meta(new QLabel(this)),
      m_status(new QLabel(this)),
      m_pinButton(new QToolButton(this)),
      m_codeInput(new QLineEdit(this)),
      m_intervalSpin(new QSpinBox(this)),
      m_sourceCombo(new QComboBox(this)),
      m_addButton(new QToolButton(this)),
      m_removeButton(new QToolButton(this)),
      m_watchList(new QListWidget(this)),
      m_symbol(QStringLiteral("601288")),
      m_stockName(QStringLiteral("Stock")),
      m_refreshIntervalMs(5000),
      m_autoRefresh(true),
      m_dataSource(QStringLiteral("eastmoney")),
      m_eastMoneyFailureStreak(0),
      m_syncingWatchList(false),
      m_timer(new QTimer(this)),
      m_net(new QNetworkAccessManager(this)),
      m_mainSplitter(nullptr),
      m_chartPanel(nullptr)
{
    setMinimumHeight(280);
    setCornerRadius(12);
    setBackgroundColor(QColor(24, 28, 36));
    setForegroundColor(QColor(226, 234, 248));
    setAccentColor(QColor(56, 125, 255));

    m_subscriptions << m_symbol;
    m_defaultSubscriptions = m_subscriptions;
    loadLocalState();
    ensureCurrentSymbolValid();

    m_title->setObjectName(QStringLiteral("NStockTitle"));
    m_price->setObjectName(QStringLiteral("NStockPrice"));
    m_change->setObjectName(QStringLiteral("NStockChange"));
    m_meta->setObjectName(QStringLiteral("NStockMeta"));
    m_status->setObjectName(QStringLiteral("NStockStatus"));
    m_meta->setWordWrap(true);
    m_pinButton->setObjectName(QStringLiteral("NStockPinButton"));
    m_pinButton->setText(QStringLiteral("Pin Top"));
    m_pinButton->setCursor(Qt::PointingHandCursor);
    connect(m_pinButton, &QToolButton::clicked, this, &NStockQuoteCard::onPinCurrentClicked);
    m_codeInput->setObjectName(QStringLiteral("NStockCodeInput"));
    m_codeInput->setPlaceholderText(QStringLiteral("输入股票代码，如 601288"));
    m_codeInput->setClearButtonEnabled(true);
    m_intervalSpin->setObjectName(QStringLiteral("NStockIntervalSpin"));
    m_intervalSpin->setRange(1000, 60000);
    m_intervalSpin->setSingleStep(500);
    m_intervalSpin->setSuffix(QStringLiteral(" ms"));
    m_intervalSpin->setValue(m_refreshIntervalMs);
    m_intervalSpin->setToolTip(QStringLiteral("股票查询周期（毫秒）"));
    connect(m_intervalSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &NStockQuoteCard::setRefreshIntervalMs);
    m_sourceCombo->setObjectName(QStringLiteral("NStockSourceCombo"));
    m_sourceCombo->addItem(QStringLiteral("EastMoney"), QStringLiteral("eastmoney"));
    m_sourceCombo->addItem(QStringLiteral("Sina"), QStringLiteral("sina"));
    const int sourceIdx = qMax(0, m_sourceCombo->findData(m_dataSource));
    m_sourceCombo->setCurrentIndex(sourceIdx);
    connect(m_sourceCombo, &QComboBox::currentTextChanged, this, [this]() {
        const QString source = m_sourceCombo->currentData().toString();
        setDataSource(source);
    });
    m_addButton->setObjectName(QStringLiteral("NStockSmallButton"));
    m_removeButton->setObjectName(QStringLiteral("NStockSmallButton"));
    m_addButton->setText(QStringLiteral("增加"));
    m_removeButton->setText(QStringLiteral("删除"));
    m_addButton->setCursor(Qt::PointingHandCursor);
    m_removeButton->setCursor(Qt::PointingHandCursor);
    connect(m_addButton, &QToolButton::clicked, this, &NStockQuoteCard::onAddCodeClicked);
    connect(m_removeButton, &QToolButton::clicked, this, &NStockQuoteCard::onRemoveCodeClicked);
    connect(m_codeInput, &QLineEdit::returnPressed, this, &NStockQuoteCard::onAddCodeClicked);

    m_watchList->setObjectName(QStringLiteral("NStockWatchList"));
    m_watchList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_watchList->setDragDropMode(QAbstractItemView::InternalMove);
    m_watchList->setDefaultDropAction(Qt::MoveAction);
    m_watchList->setAlternatingRowColors(false);
    m_watchList->setUniformItemSizes(true);
    m_watchList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_watchList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_watchList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_watchList, &QListWidget::currentItemChanged,
            this, &NStockQuoteCard::onWatchCurrentItemChanged);
    connect(m_watchList, &QListWidget::customContextMenuRequested,
            this, &NStockQuoteCard::onWatchContextMenu);
    connect(m_watchList->model(), &QAbstractItemModel::rowsMoved,
            this, &NStockQuoteCard::onWatchRowsMoved);

    auto *titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(0, 0, 0, 0);
    titleRow->setSpacing(8);
    titleRow->addWidget(m_title);
    titleRow->addStretch(1);
    titleRow->addWidget(m_pinButton);
    titleRow->addWidget(m_status);

    auto *priceRow = new QHBoxLayout;
    priceRow->setContentsMargins(0, 0, 0, 0);
    priceRow->setSpacing(10);
    priceRow->addWidget(m_price);
    priceRow->addWidget(m_change);
    priceRow->addStretch(1);

    auto *editRow = new QHBoxLayout;
    editRow->setContentsMargins(0, 0, 0, 0);
    editRow->setSpacing(6);
    editRow->addWidget(m_codeInput, 1);
    editRow->addWidget(m_addButton);
    editRow->addWidget(m_removeButton);
    editRow->addSpacing(8);
    editRow->addWidget(new QLabel(QStringLiteral("源"), this));
    editRow->addWidget(m_sourceCombo);
    editRow->addSpacing(8);
    editRow->addWidget(new QLabel(QStringLiteral("周期"), this));
    editRow->addWidget(m_intervalSpin);

    m_chartPanel = new NStockChartPanel(this);
    m_chartPanel->setMinimumWidth(200);

    auto *listColumn = new QWidget(this);
    auto *listLayout = new QVBoxLayout(listColumn);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(7);
    listLayout->addWidget(m_watchList, 1);
    listLayout->addLayout(editRow);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setObjectName(QStringLiteral("NStockMainSplitter"));
    m_mainSplitter->addWidget(listColumn);
    m_mainSplitter->addWidget(m_chartPanel);
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 2);
    m_mainSplitter->setChildrenCollapsible(false);
    m_mainSplitter->setHandleWidth(6);

    {
        QSettings splitterStore;
        splitterStore.beginGroup(QStringLiteral("NStockQuoteCard"));
        const QByteArray splitterState = splitterStore.value(QStringLiteral("stockChartSplitterState")).toByteArray();
        if (!splitterState.isEmpty()) {
            m_mainSplitter->restoreState(splitterState);
        } else {
            m_mainSplitter->setSizes(QList<int>() << 240 << 420);
        }
        splitterStore.endGroup();
    }

    connect(m_mainSplitter, &QSplitter::splitterMoved, this, [this](int, int) { saveLocalState(); });

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(7);
    layout->addLayout(titleRow);
    layout->addWidget(m_mainSplitter, 1);
    layout->addLayout(priceRow);
    layout->addWidget(m_meta);

    m_chartPanel->setSymbol(m_symbol);

    m_timer->setInterval(m_refreshIntervalMs);
    connect(m_timer, &QTimer::timeout, this, &NStockQuoteCard::refreshNow);
    connect(m_net, &QNetworkAccessManager::finished, this, &NStockQuoteCard::onReplyFinished);

    m_status->setText(QStringLiteral("Loading..."));
    applyChromeStyle();
    refreshSelectorButtons();
    if (m_autoRefresh) {
        m_timer->start();
    }
    refreshNow();
}

NStockQuoteCard::~NStockQuoteCard() = default;

QString NStockQuoteCard::symbol() const
{
    return m_symbol;
}

void NStockQuoteCard::setSymbol(const QString &symbol)
{
    setCurrentSymbol(symbol);
}

QString NStockQuoteCard::stockName() const
{
    return m_stockName;
}

void NStockQuoteCard::setStockName(const QString &name)
{
    if (m_stockName == name) {
        return;
    }
    m_stockName = name;
    setPreferredNameForSymbol(m_symbol, name);
    refreshDetail();
    refreshSelectorButtons();
    saveLocalState();
    emit stockNameChanged(m_stockName);
}

QStringList NStockQuoteCard::subscriptions() const
{
    return m_subscriptions;
}

void NStockQuoteCard::setSubscriptions(const QStringList &symbols)
{
    QStringList normalized;
    for (const QString &raw : symbols) {
        const QString s = normalizeSymbol(raw);
        if (!s.isEmpty() && !normalized.contains(s)) {
            normalized.append(s);
        }
    }
    if (normalized.isEmpty()) {
        normalized << m_symbol;
    }
    if (m_subscriptions == normalized) {
        return;
    }
    m_defaultSubscriptions = normalized;
    m_subscriptions = normalized;
    ensureCurrentSymbolValid();
    if (!m_pinnedSymbol.isEmpty() && !m_subscriptions.contains(m_pinnedSymbol)) {
        m_pinnedSymbol.clear();
        emit pinnedSymbolChanged(m_pinnedSymbol);
    }
    refreshSelectorButtons();
    emit subscriptionsChanged(m_subscriptions);
    saveLocalState();
    refreshNow();
}

void NStockQuoteCard::addSubscription(const QString &symbol)
{
    const QString s = normalizeSymbol(symbol);
    if (s.isEmpty()) {
        return;
    }

    if (secIdForSymbol(s).isEmpty()) {
        m_status->setText(QStringLiteral("Invalid Symbol"));
        emit requestFailed(QStringLiteral("invalid symbol: %1").arg(s));
        return;
    }

    // 已存在时直接切换到该股票并刷新，确保详情区实时显示
    if (m_subscriptions.contains(s)) {
        setCurrentSymbol(s);
        requestQuote(s);
        return;
    }

    m_subscriptions.append(s);
    m_symbol = s;
    m_stockName = preferredNameForSymbol(m_symbol, QString());
    refreshSelectorButtons();
    refreshDetail();
    m_status->setText(QStringLiteral("Added: %1").arg(s));
    emit symbolChanged(m_symbol);
    emit currentSymbolChanged(m_symbol);
    emit subscriptionsChanged(m_subscriptions);
    saveLocalState();
    requestQuote(s);
    if (m_chartPanel) {
        m_chartPanel->setSymbol(m_symbol);
    }
}

void NStockQuoteCard::removeSubscription(const QString &symbol)
{
    const QString s = normalizeSymbol(symbol);
    const int idx = m_subscriptions.indexOf(s);
    if (idx < 0 || m_subscriptions.size() <= 1) {
        return;
    }
    m_subscriptions.removeAt(idx);
    m_quotes.remove(s);
    if (m_pinnedSymbol == s) {
        m_pinnedSymbol.clear();
        emit pinnedSymbolChanged(m_pinnedSymbol);
    }
    ensureCurrentSymbolValid();
    refreshSelectorButtons();
    refreshDetail();
    emit subscriptionsChanged(m_subscriptions);
    saveLocalState();
}

QString NStockQuoteCard::currentSymbol() const
{
    return m_symbol;
}

void NStockQuoteCard::setCurrentSymbol(const QString &symbol)
{
    const QString normalized = normalizeSymbol(symbol);
    if (normalized.isEmpty()) {
        return;
    }
    if (!m_subscriptions.contains(normalized)) {
        m_subscriptions.append(normalized);
        emit subscriptionsChanged(m_subscriptions);
    }
    if (m_symbol == normalized) {
        return;
    }
    m_symbol = normalized;
    QString nextName = preferredNameForSymbol(m_symbol, QString());
    if (nextName.isEmpty() && m_quotes.contains(m_symbol)) {
        nextName = m_quotes.value(m_symbol).name;
    }
    if (m_stockName != nextName) {
        m_stockName = nextName;
        emit stockNameChanged(m_stockName);
    }
    refreshSelectorButtons();
    refreshDetail();
    emit symbolChanged(m_symbol);
    emit currentSymbolChanged(m_symbol);
    saveLocalState();
    requestQuote(m_symbol);
    if (m_chartPanel) {
        m_chartPanel->setSymbol(m_symbol);
    }
}

QString NStockQuoteCard::pinnedSymbol() const
{
    return m_pinnedSymbol;
}

void NStockQuoteCard::pinSymbol(const QString &symbol)
{
    const QString s = normalizeSymbol(symbol);
    const int idx = indexOfSymbol(s);
    if (idx < 0 || idx == 0) {
        m_pinnedSymbol = (idx == 0) ? s : m_pinnedSymbol;
        refreshSelectorButtons();
        return;
    }
    m_subscriptions.move(idx, 0);
    m_pinnedSymbol = s;
    refreshSelectorButtons();
    emit subscriptionsChanged(m_subscriptions);
    emit pinnedSymbolChanged(m_pinnedSymbol);
    saveLocalState();
}

void NStockQuoteCard::unpinSymbol()
{
    if (m_pinnedSymbol.isEmpty()) {
        return;
    }
    m_pinnedSymbol.clear();
    refreshSelectorButtons();
    emit pinnedSymbolChanged(m_pinnedSymbol);
    saveLocalState();
}

int NStockQuoteCard::refreshIntervalMs() const
{
    return m_refreshIntervalMs;
}

void NStockQuoteCard::setRefreshIntervalMs(int ms)
{
    ms = qMax(1000, ms);
    if (m_refreshIntervalMs == ms) {
        return;
    }
    m_refreshIntervalMs = ms;
    m_timer->setInterval(m_refreshIntervalMs);
    if (m_intervalSpin && m_intervalSpin->value() != ms) {
        const QSignalBlocker blocker(m_intervalSpin);
        m_intervalSpin->setValue(ms);
    }
    saveLocalState();
    emit refreshIntervalChanged(ms);
}

bool NStockQuoteCard::autoRefresh() const
{
    return m_autoRefresh;
}

void NStockQuoteCard::setAutoRefresh(bool on)
{
    if (m_autoRefresh == on) {
        return;
    }
    m_autoRefresh = on;
    if (m_autoRefresh) {
        m_timer->start();
        refreshNow();
    } else {
        m_timer->stop();
    }
    emit autoRefreshChanged(on);
}

QString NStockQuoteCard::dataSource() const
{
    return m_dataSource;
}

void NStockQuoteCard::setDataSource(const QString &source)
{
    const QString s = source.trimmed().toLower();
    const QString normalized = (s == QStringLiteral("eastmoney"))
        ? QStringLiteral("eastmoney")
        : QStringLiteral("sina");
    if (m_dataSource == normalized) {
        return;
    }
    resetEastMoneyFailureStreak();
    m_dataSource = normalized;
    if (m_sourceCombo) {
        const int idx = m_sourceCombo->findData(m_dataSource);
        if (idx >= 0 && m_sourceCombo->currentIndex() != idx) {
            const QSignalBlocker blocker(m_sourceCombo);
            m_sourceCombo->setCurrentIndex(idx);
        }
    }
    saveLocalState();
    emit dataSourceChanged(m_dataSource);
    refreshNow();
}

void NStockQuoteCard::refreshNow()
{
    if (m_subscriptions.isEmpty()) {
        m_status->setText(QStringLiteral("No Symbol"));
        return;
    }
    m_status->setText(QStringLiteral("Refreshing..."));
    for (const QString &s : m_subscriptions) {
        requestQuote(s);
    }
    if (m_chartPanel) {
        m_chartPanel->reloadIntradayIfNeeded();
    }
}

void NStockQuoteCard::requestQuote(const QString &symbol)
{
    const QString s = normalizeSymbol(symbol);
    if (secIdForSymbol(s).isEmpty()) {
        emit requestFailed(QStringLiteral("invalid symbol: %1").arg(symbol));
        return;
    }
    requestQuoteWithSource(s, m_dataSource, true);
}

void NStockQuoteCard::requestQuoteWithSource(const QString &s, const QString &source, bool allowFallback)
{
    const bool isShanghai = s.startsWith(QLatin1Char('6')) || s.startsWith(QLatin1Char('9'));
    QString url;
    if (source == QStringLiteral("eastmoney")) {
        const QString secId = QStringLiteral("%1.%2")
                                  .arg(isShanghai ? QStringLiteral("1") : QStringLiteral("0"), s);
        url = QStringLiteral("http://push2.eastmoney.com/api/qt/stock/get?"
                             "secid=%1&fields=f57,f58,f43,f46,f60,f44,f45,f31,f32,f169,f170,f47,f48,f124")
                  .arg(secId);
    } else {
        const QString sinaCode = QStringLiteral("%1%2").arg(isShanghai ? QStringLiteral("sh")
                                                                        : QStringLiteral("sz"),
                                                            s);
        url = QStringLiteral("http://hq.sinajs.cn/list=%1").arg(sinaCode);
    }
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0"));
    if (source == QStringLiteral("sina")) {
        req.setRawHeader("Referer", "https://finance.sina.com.cn");
    }
    QNetworkReply *reply = m_net->get(req);
    reply->setProperty("quoteSource", source);
    reply->setProperty("allowFallback", allowFallback);
    m_pendingReplies.insert(reply, s);
}

bool NStockQuoteCard::tryScheduleFallback(const QString &normalizedSymbol,
                                          const QString &usedSourceLower,
                                          bool allowFallback)
{
    if (!allowFallback) {
        return false;
    }
    const bool east = usedSourceLower == QStringLiteral("eastmoney");
    const bool sina = usedSourceLower == QStringLiteral("sina");
    if (!east && !sina) {
        return false;
    }
    const QString other = east ? QStringLiteral("sina") : QStringLiteral("eastmoney");
    const QString fromName = east ? QStringLiteral("EastMoney") : QStringLiteral("Sina");
    const QString toName = (other == QStringLiteral("eastmoney")) ? QStringLiteral("EastMoney") : QStringLiteral("Sina");
    if (normalizedSymbol == m_symbol) {
        m_status->setText(QStringLiteral("Fallback: %1 -> %2").arg(fromName, toName));
    }
    requestQuoteWithSource(normalizedSymbol, other, false);
    return true;
}

void NStockQuoteCard::resetEastMoneyFailureStreak()
{
    m_eastMoneyFailureStreak = 0;
}

void NStockQuoteCard::recordEastMoneyPrimaryFailed(const QString &replyQuoteSourceLower, bool allowFallback)
{
    if (m_dataSource != QStringLiteral("eastmoney")) {
        return;
    }
    const QString ps = replyQuoteSourceLower.trimmed().toLower();
    if (ps != QStringLiteral("eastmoney") && !(ps == QStringLiteral("sina") && !allowFallback)) {
        return;
    }
    m_eastMoneyFailureStreak++;
    static const int kEastMoneyFailThreshold = 5;
    if (m_eastMoneyFailureStreak >= kEastMoneyFailThreshold) {
        m_eastMoneyFailureStreak = 0;
        setDataSource(QStringLiteral("sina"));
        if (m_status) {
            m_status->setText(QStringLiteral("东财连续获取失败，已自动切换新浪"));
        }
    }
}

void NStockQuoteCard::onReplyFinished(QNetworkReply *reply)
{
    const QString replySymbol = m_pendingReplies.take(reply);
    const bool allowFallback = reply->property("allowFallback").toBool();
    const QString quoteSource = reply->property("quoteSource").toString().trimmed().toLower();
    if (replySymbol.isEmpty()) {
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        recordEastMoneyPrimaryFailed(quoteSource, allowFallback);
        if (!tryScheduleFallback(replySymbol, quoteSource, allowFallback)) {
            if (replySymbol == m_symbol) {
                m_status->setText(QStringLiteral("Offline"));
            }
            emit requestFailed(reply->errorString());
        }
        reply->deleteLater();
        return;
    }

    const QByteArray body = reply->readAll();
    const QString source = quoteSource;
    reply->deleteLater();

    QuoteSnapshot quote;
    quote.symbol = replySymbol;
    if (source == QStringLiteral("eastmoney")) {
        const QJsonDocument doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            recordEastMoneyPrimaryFailed(source, allowFallback);
            if (!tryScheduleFallback(replySymbol, source, allowFallback)) {
                if (replySymbol == m_symbol) {
                    m_status->setText(QStringLiteral("Data Error"));
                }
                emit requestFailed(QStringLiteral("invalid eastmoney payload"));
            }
            return;
        }
        const QJsonObject data = doc.object().value(QStringLiteral("data")).toObject();
        if (data.isEmpty()) {
            recordEastMoneyPrimaryFailed(source, allowFallback);
            if (!tryScheduleFallback(replySymbol, source, allowFallback)) {
                if (replySymbol == m_symbol) {
                    m_status->setText(QStringLiteral("No Data"));
                }
                emit requestFailed(QStringLiteral("empty eastmoney data"));
            }
            return;
        }
        const QString fetchedName = data.value(QStringLiteral("f58")).toVariant().toString().trimmed();
        quote.name = preferredNameForSymbol(replySymbol, fetchedName);
        quote.price = data.value(QStringLiteral("f43")).toDouble() / 100.0;
        quote.openPrice = data.value(QStringLiteral("f46")).toDouble() / 100.0;
        quote.prevClose = data.value(QStringLiteral("f60")).toDouble() / 100.0;
        quote.highPrice = data.value(QStringLiteral("f44")).toDouble() / 100.0;
        quote.lowPrice = data.value(QStringLiteral("f45")).toDouble() / 100.0;
        quote.bidPrice = data.value(QStringLiteral("f31")).toDouble() / 100.0;
        quote.askPrice = data.value(QStringLiteral("f32")).toDouble() / 100.0;
        quote.change = data.value(QStringLiteral("f169")).toDouble() / 100.0;
        quote.changePct = data.value(QStringLiteral("f170")).toDouble() / 100.0;
        if (qFuzzyIsNull(quote.change) && quote.prevClose > 0.0) {
            quote.change = quote.price - quote.prevClose;
        }
        if (qFuzzyIsNull(quote.changePct) && quote.prevClose > 0.0) {
            quote.changePct = quote.change / quote.prevClose * 100.0;
        }
        quote.volume = data.value(QStringLiteral("f47")).toVariant().toLongLong();
        quote.turnover = data.value(QStringLiteral("f48")).toVariant().toLongLong();
        const qint64 ts = data.value(QStringLiteral("f124")).toVariant().toLongLong();
        quote.updateAt = (ts > 0) ? QDateTime::fromSecsSinceEpoch(ts) : QDateTime::currentDateTime();
        quote.detailText = QStringLiteral("今开: %1  昨收: %2  现价: %3  最高: %4  最低: %5  买一: %6  卖一: %7\n"
                                          "成交量: %8  成交额: %9\n"
                                          "数据源: EastMoney\n"
                                          "更新时间: %10")
                               .arg(QLocale().toString(quote.openPrice, 'f', 2))
                               .arg(QLocale().toString(quote.prevClose, 'f', 2))
                               .arg(QLocale().toString(quote.price, 'f', 2))
                               .arg(QLocale().toString(quote.highPrice, 'f', 2))
                               .arg(QLocale().toString(quote.lowPrice, 'f', 2))
                               .arg(QLocale().toString(quote.bidPrice, 'f', 2))
                               .arg(QLocale().toString(quote.askPrice, 'f', 2))
                               .arg(compactNumber(quote.volume))
                               .arg(compactNumber(quote.turnover))
                               .arg(quote.updateAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    } else {
        const int firstQuote = body.indexOf('\"');
        const int lastQuote = body.lastIndexOf('\"');
        if (firstQuote < 0 || lastQuote <= firstQuote) {
            recordEastMoneyPrimaryFailed(source, allowFallback);
            if (!tryScheduleFallback(replySymbol, source, allowFallback)) {
                if (replySymbol == m_symbol) {
                    m_status->setText(QStringLiteral("Data Error"));
                }
                emit requestFailed(QStringLiteral("invalid sina payload"));
            }
            return;
        }
        const QByteArray rawQuote = body.mid(firstQuote + 1, lastQuote - firstQuote - 1);
        QTextCodec *codec = QTextCodec::codecForName("GBK");
        const QString quoteText = codec ? codec->toUnicode(rawQuote) : QString::fromLocal8Bit(rawQuote);
        const QStringList fields = quoteText.split(QLatin1Char(','));
        if (fields.size() < 10 || fields.at(0).trimmed().isEmpty()) {
            recordEastMoneyPrimaryFailed(source, allowFallback);
            if (!tryScheduleFallback(replySymbol, source, allowFallback)) {
                if (replySymbol == m_symbol) {
                    m_status->setText(QStringLiteral("No Data"));
                }
                emit requestFailed(QStringLiteral("empty sina payload"));
            }
            return;
        }
        const QString fetchedName = fields.value(0).trimmed();
        quote.name = preferredNameForSymbol(replySymbol, fetchedName);
        quote.openPrice = fields.value(1).toDouble();
        quote.prevClose = fields.value(2).toDouble();
        quote.price = fields.value(3).toDouble();
        quote.highPrice = fields.value(4).toDouble();
        quote.lowPrice = fields.value(5).toDouble();
        quote.bidPrice = fields.value(6).toDouble();
        quote.askPrice = fields.value(7).toDouble();
        quote.change = quote.price - quote.prevClose;
        if (quote.prevClose > 0.0) {
            quote.changePct = quote.change / quote.prevClose * 100.0;
        }
        quote.volume = fields.value(8).toDouble();
        quote.turnover = fields.value(9).toDouble();
        const QString datePart = fields.value(30).trimmed();
        const QString timePart = fields.value(31).trimmed();
        const QDateTime dt = QDateTime::fromString(datePart + QLatin1Char(' ') + timePart,
                                                   QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        quote.updateAt = dt.isValid() ? dt : QDateTime::currentDateTime();

        const QString priceLine = QStringLiteral(
            "今开: %1  昨收: %2  现价: %3  最高: %4  最低: %5  买一: %6  卖一: %7")
                                      .arg(QLocale().toString(quote.openPrice, 'f', 2))
                                      .arg(QLocale().toString(quote.prevClose, 'f', 2))
                                      .arg(QLocale().toString(quote.price, 'f', 2))
                                      .arg(QLocale().toString(quote.highPrice, 'f', 2))
                                      .arg(QLocale().toString(quote.lowPrice, 'f', 2))
                                      .arg(QLocale().toString(quote.bidPrice, 'f', 2))
                                      .arg(QLocale().toString(quote.askPrice, 'f', 2));
        const QString volumeLine = QStringLiteral("成交量: %1  成交额: %2")
                                       .arg(compactNumber(quote.volume))
                                       .arg(compactNumber(quote.turnover));
        QString bidLine = QStringLiteral("买盘: ");
        QString askLine = QStringLiteral("卖盘: ");
        for (int level = 1; level <= 5; ++level) {
            const int bidVolIdx = 10 + (level - 1) * 2;
            const int bidPriceIdx = bidVolIdx + 1;
            const int askVolIdx = 20 + (level - 1) * 2;
            const int askPriceIdx = askVolIdx + 1;
            const QString bidVol = fields.value(bidVolIdx).trimmed();
            const QString bidPx = fields.value(bidPriceIdx).trimmed();
            const QString askVol = fields.value(askVolIdx).trimmed();
            const QString askPx = fields.value(askPriceIdx).trimmed();
            if (level > 1) {
                bidLine += QStringLiteral("  ");
                askLine += QStringLiteral("  ");
            }
            bidLine += QStringLiteral("B%1:%2@%3").arg(level).arg(bidVol, bidPx);
            askLine += QStringLiteral("S%1:%2@%3").arg(level).arg(askVol, askPx);
        }
        quote.detailText = QStringLiteral("%1\n%2\n%3\n%4\n数据源: Sina\n更新时间: %5")
                               .arg(priceLine)
                               .arg(volumeLine)
                               .arg(bidLine)
                               .arg(askLine)
                               .arg(quote.updateAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    }
    quote.valid = true;
    if (source == QStringLiteral("eastmoney")) {
        resetEastMoneyFailureStreak();
    }
    m_quotes.insert(replySymbol, quote);

    if (replySymbol == m_symbol) {
        if (m_stockName != quote.name) {
            m_stockName = quote.name;
            emit stockNameChanged(m_stockName);
        }
        refreshDetail();
        m_status->setText(QStringLiteral("Live"));
        emit quoteUpdated();
    }
    refreshSelectorButtons();
}

void NStockQuoteCard::onPinCurrentClicked()
{
    if (m_subscriptions.isEmpty()) {
        return;
    }
    if (m_pinnedSymbol == m_symbol) {
        unpinSymbol();
    } else {
        pinSymbol(m_symbol);
    }
}

void NStockQuoteCard::onWatchCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current || m_syncingWatchList) {
        return;
    }
    const QString selected = current->data(Qt::UserRole).toString();
    if (!selected.isEmpty()) {
        setCurrentSymbol(selected);
    }
}

void NStockQuoteCard::onWatchRowsMoved(const QModelIndex &parent, int start, int end,
                                       const QModelIndex &destination, int row)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(destination);
    Q_UNUSED(row);
    if (m_syncingWatchList) {
        return;
    }
    syncSubscriptionsFromWatchList();
}

void NStockQuoteCard::onWatchContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_watchList->itemAt(pos);

    QMenu menu(this);
    QAction *addAction = menu.addAction(QStringLiteral("新增订阅"));
    QAction *clearLocalAction = menu.addAction(QStringLiteral("清空本地配置"));
    menu.addSeparator();
    QAction *pinAction = nullptr;
    QAction *renameAction = nullptr;
    QAction *removeAction = nullptr;
    QString symbol;
    if (item) {
        symbol = item->data(Qt::UserRole).toString();
    }
    if (!symbol.isEmpty()) {
        if (m_pinnedSymbol == symbol) {
            pinAction = menu.addAction(QStringLiteral("取消置顶"));
        } else {
            pinAction = menu.addAction(QStringLiteral("置顶"));
        }
        renameAction = menu.addAction(QStringLiteral("重命名"));
        removeAction = menu.addAction(QStringLiteral("移除"));
        removeAction->setEnabled(m_subscriptions.size() > 1);
    }

    QAction *chosen = menu.exec(m_watchList->viewport()->mapToGlobal(pos));
    if (!chosen) {
        return;
    }

    if (chosen == addAction) {
        bool ok = false;
        const QString code = QInputDialog::getText(this,
                                                   QStringLiteral("新增订阅"),
                                                   QStringLiteral("股票代码:"),
                                                   QLineEdit::Normal,
                                                   QString(),
                                                   &ok).trimmed();
        if (ok && !code.isEmpty()) {
            addSubscription(code);
            m_codeInput->setText(normalizeSymbol(code));
        }
        return;
    }
    if (chosen == clearLocalAction) {
        QSettings settings;
        settings.remove(QStringLiteral("NStockQuoteCard"));
        resetToDefaultSubscriptions();
        return;
    }

    if (chosen == pinAction) {
        if (m_pinnedSymbol == symbol) {
            unpinSymbol();
        } else {
            pinSymbol(symbol);
        }
        return;
    }
    if (chosen == removeAction) {
        removeSubscription(symbol);
        return;
    }
    if (chosen == renameAction) {
        const QString currentName = preferredNameForSymbol(symbol, symbol);
        bool ok = false;
        const QString newName = QInputDialog::getText(this,
                                                      QStringLiteral("重命名股票"),
                                                      QStringLiteral("名称:"),
                                                      QLineEdit::Normal,
                                                      currentName,
                                                      &ok).trimmed();
        if (!ok || newName.isEmpty()) {
            return;
        }
        setPreferredNameForSymbol(symbol, newName);
        if (symbol == m_symbol) {
            m_stockName = newName;
            emit stockNameChanged(m_stockName);
        }
        if (m_quotes.contains(symbol)) {
            QuoteSnapshot q = m_quotes.value(symbol);
            q.name = newName;
            m_quotes.insert(symbol, q);
        }
        saveLocalState();
        refreshDetail();
        refreshSelectorButtons();
    }
}

void NStockQuoteCard::onAddCodeClicked()
{
    const QString code = m_codeInput->text().trimmed();
    if (code.isEmpty()) {
        return;
    }
    addSubscription(code);
    m_codeInput->clear();
}

void NStockQuoteCard::onRemoveCodeClicked()
{
    QString code = m_codeInput->text().trimmed();
    if (code.isEmpty()) {
        code = m_symbol;
    }
    removeSubscription(code);
    if (!m_subscriptions.isEmpty()) {
        setCurrentSymbol(m_symbol);
    }
    m_codeInput->clear();
}

void NStockQuoteCard::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    setStyleSheet(surfaceStyleSheet() +
                  QStringLiteral(
                      "QLabel#NStockTitle { color: #e8edf9; font-size: 15px; font-weight: 700; }"
                      "QLabel#NStockPrice { color: #f2f5ff; font-size: 28px; font-weight: 800; }"
                      "QLabel#NStockChange { font-size: 15px; font-weight: 700; }"
                      "QLabel#NStockMeta { color: #8f9cb4; font-size: 12px; }"
                      "QLabel#NStockStatus { color: #8f9cb4; font-size: 12px; }"
                      "QToolButton#NStockPinButton { border: none; border-radius: 8px; background: #2d3445; color: #b6c3dc; padding: 4px 8px; font-size: 12px; }"
                      "QToolButton#NStockPinButton:hover { background: #39425a; color: #dce6ff; }"
                      "QLineEdit#NStockCodeInput { border: 1px solid #31384a; border-radius: 7px; background: #202636; color: #c8d4ea; padding: 4px 8px; }"
                      "QLineEdit#NStockCodeInput:focus { border: 1px solid #4a6cff; }"
                      "QComboBox#NStockSourceCombo { border: 1px solid #31384a; border-radius: 7px; background: #202636; color: #c8d4ea; padding: 3px 8px; }"
                      "QComboBox#NStockSourceCombo:focus { border: 1px solid #4a6cff; }"
                      "QComboBox#NStockSourceCombo QAbstractItemView { background: #202636; color: #c8d4ea; selection-background-color: #2b3550; }"
                      "QSpinBox#NStockIntervalSpin { border: 1px solid #31384a; border-radius: 7px; background: #202636; color: #c8d4ea; padding: 2px 6px; }"
                      "QSpinBox#NStockIntervalSpin:focus { border: 1px solid #4a6cff; }"
                      "QToolButton#NStockSmallButton { border: 1px solid #36405a; border-radius: 7px; background: #2a3144; color: #c2cde1; padding: 4px 8px; }"
                      "QToolButton#NStockSmallButton:hover { background: #36405a; color: #e8efff; }"
                      "QListWidget#NStockWatchList { border: 1px solid #31384a; border-radius: 8px; background: #1d2332; outline: none; padding: 4px; }"
                      "QListWidget#NStockWatchList::item { border: none; padding: 0; margin: 2px 0; }"
                      "QWidget#NStockRow { border: 1px solid #31384a; border-radius: 7px; background: #202636; }"
                      "QWidget#NStockRow[active='true'] { border: 1px solid #4a6cff; background: #2b3550; }"
                      "QLabel#NStockRowLabel { color: #9eabc1; font-size: 12px; }"
                      "QWidget#NStockRow[active='true'] QLabel#NStockRowLabel { color: #e7eeff; font-weight: 700; }"
                      "QToolButton#NStockRowDelete { border: none; border-radius: 6px; background: transparent; color: #8f9cb4; padding: 1px 6px; font-size: 12px; }"
                      "QToolButton#NStockRowDelete:hover { background: #3a2030; color: #ffb7c4; }"
                      "QSplitter#NStockMainSplitter::handle { background: #39425a; }"));
}

QString NStockQuoteCard::normalizeSymbol(const QString &symbol)
{
    QString s = symbol.trimmed();
    if (s.startsWith(QStringLiteral("sh"), Qt::CaseInsensitive) ||
        s.startsWith(QStringLiteral("sz"), Qt::CaseInsensitive)) {
        s = s.mid(2);
    }
    return s;
}

QString NStockQuoteCard::secIdForSymbol(const QString &symbol)
{
    const QString s = normalizeSymbol(symbol);
    if (s.size() != 6) {
        return QString();
    }
    bool ok = false;
    s.toInt(&ok);
    if (!ok) {
        return QString();
    }
    const bool isShanghai = s.startsWith(QLatin1Char('6')) || s.startsWith(QLatin1Char('9'));
    return QStringLiteral("%1.%2").arg(isShanghai ? QStringLiteral("1") : QStringLiteral("0"), s);
}

double NStockQuoteCard::scalePriceField(double raw)
{
    return raw / 100.0;
}

QString NStockQuoteCard::compactNumber(qint64 value)
{
    if (value >= 100000000) {
        return QStringLiteral("%1亿").arg(QLocale().toString(value / 100000000.0, 'f', 2));
    }
    if (value >= 10000) {
        return QStringLiteral("%1万").arg(QLocale().toString(value / 10000.0, 'f', 2));
    }
    return QLocale().toString(value);
}

QString NStockQuoteCard::makeSelectorText(const QuoteSnapshot &quote)
{
    if (!quote.valid) {
        return quote.symbol;
    }
    const QString namePart = quote.name.trimmed().isEmpty()
        ? QStringLiteral("--")
        : quote.name.trimmed();
    const QString pct = QLocale().toString(quote.changePct, 'f', 2) + QStringLiteral("%");
    return QStringLiteral("%1 %2  %3 (%4)")
        .arg(namePart)
        .arg(quote.symbol)
        .arg(QLocale().toString(quote.price, 'f', 2))
        .arg(pct);
}

void NStockQuoteCard::refreshDetail()
{
    const QuoteSnapshot quote = m_quotes.value(m_symbol);
    if (!quote.valid) {
        const QString maybeName = preferredNameForSymbol(m_symbol, QStringLiteral("Stock"));
        m_title->setText(QStringLiteral("%1 (%2)").arg(maybeName, m_symbol));
        m_price->setText(QStringLiteral("--"));
        m_change->setText(QStringLiteral("--"));
        m_meta->setText(QStringLiteral("Waiting for live quote..."));
        updateTrendStyle(0.0);
        return;
    }

    m_title->setText(QStringLiteral("%1 (%2)").arg(quote.name, quote.symbol));
    m_price->setText(QLocale().toString(quote.price, 'f', 2));
    const QString arrow = (quote.change >= 0.0) ? QStringLiteral("▲") : QStringLiteral("▼");
    m_change->setText(QStringLiteral("%1 %2 (%3%)")
                          .arg(arrow)
                          .arg(QLocale().toString(quote.change, 'f', 2))
                          .arg(QLocale().toString(quote.changePct, 'f', 2)));
    m_meta->setText(QStringLiteral("Vol: %1    Amount: %2    %3")
                        .arg(compactNumber(quote.volume))
                        .arg(compactNumber(quote.turnover))
                        .arg(quote.updateAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))));
    if (!quote.detailText.isEmpty()) {
        m_meta->setText(quote.detailText);
    }
    updateTrendStyle(quote.change);
}

void NStockQuoteCard::refreshSelectorButtons()
{
    m_syncingWatchList = true;
    const QSignalBlocker blocker(m_watchList);
    m_watchList->clear();

    for (const QString &symbol : m_subscriptions) {
        QListWidgetItem *item = new QListWidgetItem(m_watchList);
        item->setData(Qt::UserRole, symbol);
        QString text = makeSelectorText(m_quotes.value(symbol));
        auto *row = new QWidget(m_watchList);
        row->setObjectName(QStringLiteral("NStockRow"));
        row->setProperty("active", symbol == m_symbol);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(8, 2, 4, 2);
        rowLayout->setSpacing(6);
        auto *label = new QLabel(text, row);
        label->setObjectName(QStringLiteral("NStockRowLabel"));
        auto *removeBtn = new QToolButton(row);
        removeBtn->setObjectName(QStringLiteral("NStockRowDelete"));
        removeBtn->setText(QStringLiteral("x"));
        removeBtn->setCursor(Qt::PointingHandCursor);
        removeBtn->setToolTip(QStringLiteral("删除该股票"));
        connect(removeBtn, &QToolButton::clicked, this, [this, symbol]() {
            removeSubscription(symbol);
        });
        rowLayout->addWidget(label, 1);
        rowLayout->addWidget(removeBtn, 0, Qt::AlignRight);
        item->setSizeHint(QSize(0, 28));
        m_watchList->setItemWidget(item, row);
        if (symbol == m_symbol) {
            m_watchList->setCurrentItem(item);
        }
    }
    if (!m_subscriptions.isEmpty() && !m_watchList->currentItem()) {
        m_watchList->setCurrentRow(0);
    }
    m_pinButton->setText((m_pinnedSymbol == m_symbol) ? QStringLiteral("Unpin") : QStringLiteral("Pin Top"));
    m_syncingWatchList = false;
}

void NStockQuoteCard::resetToDefaultSubscriptions()
{
    if (m_defaultSubscriptions.isEmpty()) {
        m_defaultSubscriptions << QStringLiteral("601288");
    }
    m_subscriptions = m_defaultSubscriptions;

    if (!m_subscriptions.contains(m_symbol)) {
        m_symbol = m_subscriptions.first();
        emit symbolChanged(m_symbol);
        emit currentSymbolChanged(m_symbol);
    }

    if (!m_pinnedSymbol.isEmpty() && !m_subscriptions.contains(m_pinnedSymbol)) {
        m_pinnedSymbol.clear();
        emit pinnedSymbolChanged(m_pinnedSymbol);
    }

    m_nameOverrides.clear();
    refreshSelectorButtons();
    refreshDetail();
    emit subscriptionsChanged(m_subscriptions);
}

void NStockQuoteCard::loadLocalState()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("NStockQuoteCard"));

    const QStringList storedSubs = settings.value(QStringLiteral("subscriptions")).toStringList();
    if (!storedSubs.isEmpty()) {
        QStringList normalized;
        for (const QString &raw : storedSubs) {
            const QString s = normalizeSymbol(raw);
            if (!s.isEmpty() && !normalized.contains(s)) {
                normalized.append(s);
            }
        }
        if (!normalized.isEmpty()) {
            m_subscriptions = normalized;
        }
    }

    const QString storedCurrent = normalizeSymbol(settings.value(QStringLiteral("currentSymbol")).toString());
    if (!storedCurrent.isEmpty() && m_subscriptions.contains(storedCurrent)) {
        m_symbol = storedCurrent;
    } else if (!m_subscriptions.isEmpty()) {
        m_symbol = m_subscriptions.first();
    }

    const QString storedPinned = normalizeSymbol(settings.value(QStringLiteral("pinnedSymbol")).toString());
    if (!storedPinned.isEmpty() && m_subscriptions.contains(storedPinned)) {
        m_pinnedSymbol = storedPinned;
    } else {
        m_pinnedSymbol.clear();
    }

    const QVariantMap aliases = settings.value(QStringLiteral("nameOverrides")).toMap();
    for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it) {
        const QString sym = normalizeSymbol(it.key());
        const QString alias = it.value().toString().trimmed();
        if (!sym.isEmpty() && !alias.isEmpty()) {
            m_nameOverrides.insert(sym, alias);
        }
    }

    if (m_nameOverrides.contains(m_symbol) && !m_nameOverrides.value(m_symbol).isEmpty()) {
        m_stockName = m_nameOverrides.value(m_symbol);
    }

    const int storedInterval = settings.value(QStringLiteral("refreshIntervalMs"), m_refreshIntervalMs).toInt();
    if (storedInterval > 0) {
        m_refreshIntervalMs = storedInterval;
    }
    m_autoRefresh = settings.value(QStringLiteral("autoRefresh"), m_autoRefresh).toBool();
    const QString source = settings.value(QStringLiteral("dataSource"), m_dataSource).toString();
    if (!source.trimmed().isEmpty()) {
        m_dataSource = source.trimmed().toLower();
    }
    if (m_dataSource != QStringLiteral("sina") && m_dataSource != QStringLiteral("eastmoney")) {
        m_dataSource = QStringLiteral("eastmoney");
    }

    settings.endGroup();
}

void NStockQuoteCard::saveLocalState() const
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("NStockQuoteCard"));
    settings.setValue(QStringLiteral("subscriptions"), m_subscriptions);
    settings.setValue(QStringLiteral("currentSymbol"), m_symbol);
    settings.setValue(QStringLiteral("pinnedSymbol"), m_pinnedSymbol);
    settings.setValue(QStringLiteral("refreshIntervalMs"), m_refreshIntervalMs);
    settings.setValue(QStringLiteral("autoRefresh"), m_autoRefresh);
    settings.setValue(QStringLiteral("dataSource"), m_dataSource);
    if (m_mainSplitter) {
        settings.setValue(QStringLiteral("stockChartSplitterState"), m_mainSplitter->saveState());
    }
    QVariantMap aliases;
    for (auto it = m_nameOverrides.constBegin(); it != m_nameOverrides.constEnd(); ++it) {
        aliases.insert(it.key(), it.value());
    }
    settings.setValue(QStringLiteral("nameOverrides"), aliases);
    settings.endGroup();
}

void NStockQuoteCard::syncSubscriptionsFromWatchList()
{
    QStringList ordered;
    for (int i = 0; i < m_watchList->count(); ++i) {
        QListWidgetItem *item = m_watchList->item(i);
        if (!item) {
            continue;
        }
        const QString symbol = item->data(Qt::UserRole).toString();
        if (!symbol.isEmpty() && !ordered.contains(symbol)) {
            ordered.append(symbol);
        }
    }
    if (ordered.isEmpty()) {
        return;
    }
    if (ordered == m_subscriptions) {
        return;
    }
    m_subscriptions = ordered;
    if (!m_pinnedSymbol.isEmpty() && m_subscriptions.first() != m_pinnedSymbol) {
        m_pinnedSymbol.clear();
        emit pinnedSymbolChanged(m_pinnedSymbol);
    }
    ensureCurrentSymbolValid();
    emit subscriptionsChanged(m_subscriptions);
    refreshSelectorButtons();
    saveLocalState();
}

void NStockQuoteCard::ensureCurrentSymbolValid()
{
    if (m_subscriptions.contains(m_symbol)) {
        return;
    }
    if (!m_subscriptions.isEmpty()) {
        m_symbol = m_subscriptions.first();
        emit symbolChanged(m_symbol);
        emit currentSymbolChanged(m_symbol);
        if (m_chartPanel) {
            m_chartPanel->setSymbol(m_symbol);
        }
    }
}

int NStockQuoteCard::indexOfSymbol(const QString &symbol) const
{
    return m_subscriptions.indexOf(symbol);
}

QString NStockQuoteCard::preferredNameForSymbol(const QString &symbol, const QString &fallback) const
{
    if (m_nameOverrides.contains(symbol) && !m_nameOverrides.value(symbol).isEmpty()) {
        return m_nameOverrides.value(symbol);
    }
    return fallback;
}

void NStockQuoteCard::setPreferredNameForSymbol(const QString &symbol, const QString &name)
{
    if (symbol.isEmpty()) {
        return;
    }
    if (name.isEmpty()) {
        m_nameOverrides.remove(symbol);
    } else {
        m_nameOverrides.insert(symbol, name);
    }
}

void NStockQuoteCard::updateTrendStyle(double change)
{
    if (qFuzzyIsNull(change)) {
        m_change->setStyleSheet(QStringLiteral("color: #b3bfd6;"));
        return;
    }
    const QString color = (change >= 0.0) ? QStringLiteral("#36c66b") : QStringLiteral("#e5636f");
    m_change->setStyleSheet(QStringLiteral("color: %1;").arg(color));
}
