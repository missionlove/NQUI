#ifndef NSTOCKQUOTECARD_H
#define NSTOCKQUOTECARD_H

#include "ndashboardwidgetbase.h"

#include <QDateTime>
#include <QHash>
#include <QModelIndex>
#include <QPoint>
#include <QStringList>

class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
class QToolButton;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QListWidget;
class QListWidgetItem;
class QSplitter;
class NStockChartPanel;

class NDASHBOARD_EXPORT NStockQuoteCard : public NDashboardWidgetBase
{
    Q_OBJECT
    Q_PROPERTY(QString symbol READ symbol WRITE setSymbol NOTIFY symbolChanged)
    Q_PROPERTY(QString stockName READ stockName WRITE setStockName NOTIFY stockNameChanged)
    Q_PROPERTY(QStringList subscriptions READ subscriptions WRITE setSubscriptions NOTIFY subscriptionsChanged)
    Q_PROPERTY(int refreshIntervalMs READ refreshIntervalMs WRITE setRefreshIntervalMs NOTIFY refreshIntervalChanged)
    Q_PROPERTY(bool autoRefresh READ autoRefresh WRITE setAutoRefresh NOTIFY autoRefreshChanged)
    Q_PROPERTY(QString dataSource READ dataSource WRITE setDataSource NOTIFY dataSourceChanged)

public:
    explicit NStockQuoteCard(QWidget *parent = nullptr);
    ~NStockQuoteCard() override;

    QString symbol() const;
    void setSymbol(const QString &symbol);

    QString stockName() const;
    void setStockName(const QString &name);

    QStringList subscriptions() const;
    void setSubscriptions(const QStringList &symbols);
    void addSubscription(const QString &symbol);
    void removeSubscription(const QString &symbol);

    QString currentSymbol() const;
    void setCurrentSymbol(const QString &symbol);

    QString pinnedSymbol() const;
    void pinSymbol(const QString &symbol);
    void unpinSymbol();

    int refreshIntervalMs() const;
    void setRefreshIntervalMs(int ms);

    bool autoRefresh() const;
    void setAutoRefresh(bool on);
    QString dataSource() const;
    void setDataSource(const QString &source);

public slots:
    void refreshNow();

signals:
    void symbolChanged(const QString &symbol);
    void stockNameChanged(const QString &name);
    void subscriptionsChanged(const QStringList &symbols);
    void currentSymbolChanged(const QString &symbol);
    void pinnedSymbolChanged(const QString &symbol);
    void refreshIntervalChanged(int ms);
    void autoRefreshChanged(bool on);
    void dataSourceChanged(const QString &source);
    void quoteUpdated();
    void requestFailed(const QString &reason);

protected:
    void applyChromeStyle() override;

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onPinCurrentClicked();
    void onWatchCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onWatchRowsMoved(const QModelIndex &parent, int start, int end,
                          const QModelIndex &destination, int row);
    void onWatchContextMenu(const QPoint &pos);
    void onAddCodeClicked();
    void onRemoveCodeClicked();

private:
    struct QuoteSnapshot {
        QString symbol;
        QString name;
        double price = 0.0;
        double openPrice = 0.0;
        double prevClose = 0.0;
        double highPrice = 0.0;
        double lowPrice = 0.0;
        double bidPrice = 0.0;
        double askPrice = 0.0;
        double change = 0.0;
        double changePct = 0.0;
        qint64 volume = 0;
        qint64 turnover = 0;
        QDateTime updateAt;
        QString detailText;
        bool valid = false;
    };

    static QString normalizeSymbol(const QString &symbol);
    static QString secIdForSymbol(const QString &symbol);
    static double scalePriceField(double raw);
    static QString compactNumber(qint64 value);
    static QString makeSelectorText(const QuoteSnapshot &quote);
    void requestQuote(const QString &symbol);
    void requestQuoteWithSource(const QString &normalizedSymbol, const QString &source, bool allowFallback);
    bool tryScheduleFallback(const QString &normalizedSymbol, const QString &usedSourceLower, bool allowFallback);
    void recordEastMoneyPrimaryFailed(const QString &replyQuoteSourceLower, bool allowFallback);
    void resetEastMoneyFailureStreak();
    void refreshDetail();
    void refreshSelectorButtons();
    void syncSubscriptionsFromWatchList();
    void resetToDefaultSubscriptions();
    void loadLocalState();
    void saveLocalState() const;
    void ensureCurrentSymbolValid();
    int indexOfSymbol(const QString &symbol) const;
    QString preferredNameForSymbol(const QString &symbol, const QString &fallback) const;
    void setPreferredNameForSymbol(const QString &symbol, const QString &name);
    void updateTrendStyle(double change);

    QLabel *m_title;
    QLabel *m_price;
    QLabel *m_change;
    QLabel *m_meta;
    QLabel *m_status;
    QToolButton *m_pinButton;
    QLineEdit *m_codeInput;
    QSpinBox *m_intervalSpin;
    QComboBox *m_sourceCombo;
    QToolButton *m_addButton;
    QToolButton *m_removeButton;
    QListWidget *m_watchList;
    QSplitter *m_mainSplitter;
    NStockChartPanel *m_chartPanel;

    QString m_symbol;
    QString m_stockName;
    QStringList m_subscriptions;
    QStringList m_defaultSubscriptions;
    QString m_pinnedSymbol;
    int m_refreshIntervalMs;
    bool m_autoRefresh;
    QString m_dataSource;
    int m_eastMoneyFailureStreak;

    QHash<QString, QuoteSnapshot> m_quotes;
    QHash<QString, QString> m_nameOverrides;
    QHash<QNetworkReply *, QString> m_pendingReplies;
    bool m_syncingWatchList;
    QTimer *m_timer;
    QNetworkAccessManager *m_net;
};

#endif // NSTOCKQUOTECARD_H
