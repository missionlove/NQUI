#include "nappwindow.h"

#include "nappshell.h"
#include "ndashboardcontentlayout.h"
#include "nmetricsummarycard.h"
#include "nnotificationbellbutton.h"
#include "nstockquotecard.h"
#include "nsparklinemetriccard.h"
#include "ntimeserieslinechartpanel.h"
#include "ntrendindicator.h"
#include "nuserprofilecompact.h"
#include "ncirculargaugecard.h"

#include <QCoreApplication>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QStringList>
#include <QStackedWidget>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtMath>

NAppWindow::NAppWindow(QWidget *parent)
    : NFramelessWidget(parent)
{
    setWindowTitle(QStringLiteral("NApp"));
    setMinimumSize(1080, 720);
    setMoveAreaHeight(44);
    setResizeBorderWidth(2);
    setResizeSensitivity(10);
    setShadowStyle(NFramelessWidget::ShadowPlatformNative);
    setCornerRadius(12);
    setShadowPadding(2);
#ifdef Q_OS_WIN
    setAttribute(Qt::WA_TranslucentBackground, false);
#else
    setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    setAttribute(Qt::WA_StyledBackground, true);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *titleBar = new QFrame(this);
    titleBar->setObjectName(QStringLiteral("NAppTitleBar"));
    titleBar->setFixedHeight(44);
    auto *th = new QHBoxLayout(titleBar);
    th->setContentsMargins(14, 0, 10, 0);
    th->setSpacing(10);

    const QString appDir = QCoreApplication::applicationDirPath();
    auto *title = new QLabel(titleBar);
    title->setObjectName(QStringLiteral("NAppTitleIcon"));
    title->setFixedSize(20, 20);
    title->setAlignment(Qt::AlignCenter);
    const QStringList appIconCandidates = {
        QDir(appDir).filePath(QStringLiteral("images/app.png")),
        QDir(appDir).filePath(QStringLiteral("../images/app.png")),
        QDir(appDir).filePath(QStringLiteral("../../images/app.png"))
    };
    QPixmap appIconPixmap;
    for (const QString &path : appIconCandidates) {
        QPixmap icon(path);
        if (!icon.isNull()) {
            appIconPixmap = icon;
            break;
        }
    }
    if (!appIconPixmap.isNull()) {
        title->setPixmap(appIconPixmap.scaled(title->size(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));
    } else {
        title->setText(QStringLiteral("NApp"));
    }
    auto *sub = new QLabel(tr("@CSDN 键盘会跳舞"), titleBar);
    sub->setObjectName(QStringLiteral("NAppSubTitle"));

    auto *bellBtn = new NNotificationBellButton();
    bellBtn->setBadgeCount(1);
    bellBtn->setBackgroundColor(QColor(32, 36, 44));
    bellBtn->setFixedSize(26, 26);
    bellBtn->setStyleSheet(QStringLiteral(
        "NNotificationBellButton { border: none; background: transparent; padding: 0; }"
        "NNotificationBellButton:hover { border: none; background: rgba(255,255,255,0.08); }"
        "NNotificationBellButton:pressed { border: none; background: rgba(255,255,255,0.14); }"));

    auto *profile = new NUserProfileCompact;
    profile->setFixedHeight(30);
    profile->setAvatarSize(28);
    profile->setDisplayName(tr("Mr Durain"));
    profile->setSubtitle(tr("R&D Engineer"));
    profile->setAvatarLetter(QLatin1Char('D'));
    const QStringList avatarCandidates = {
        QDir(appDir).filePath(QStringLiteral("images/user0.png")),
        QDir(appDir).filePath(QStringLiteral("../images/user0.png")),
        QDir(appDir).filePath(QStringLiteral("../../images/user0.png"))
    };
    for (const QString &path : avatarCandidates) {
        QPixmap avatar(path);
        if (!avatar.isNull()) {
            profile->setAvatarPixmap(avatar);
            break;
        }
    }

    th->addWidget(title);
    th->addWidget(sub, 1);
    th->addWidget(bellBtn);
    th->addWidget(profile, 0, Qt::AlignRight);

    auto *shell = new NAppShell(this);
    shell->setSidebarWidth(200);

    auto *side = new QWidget;
    side->setObjectName(QStringLiteral("NAppSidePanel"));

    auto *sideLayout = new QVBoxLayout(side);
    sideLayout->setContentsMargins(12, 14, 12, 14);
    sideLayout->setSpacing(12);
    auto makeNavButton = [side](const QString &text) {
        auto *btn = new QToolButton(side);
        // 通过统一前导空白控制图标与文本间距
        btn->setText(QStringLiteral("   ") + text);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setIconSize(QSize(16, 16));
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setProperty("navButton", true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(34);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        return btn;
    };
    auto *navOverview = makeNavButton(tr("Overview"));
    auto *navSales = makeNavButton(tr("Sales"));
    auto *navUsers = makeNavButton(tr("Users"));
    auto *navReport = makeNavButton(tr("Reports"));
    navOverview->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));
    navSales->setIcon(style()->standardIcon(QStyle::SP_DriveHDIcon));
    navUsers->setIcon(style()->standardIcon(QStyle::SP_FileDialogInfoView));
    navReport->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    sideLayout->addSpacing(8);
    sideLayout->addWidget(navOverview);
    sideLayout->addWidget(navSales);
    sideLayout->addWidget(navUsers);
    sideLayout->addWidget(navReport);
    sideLayout->addStretch(1);

    auto *pages = new QStackedWidget;
    pages->setObjectName(QStringLiteral("NAppPageStack"));

    auto makeMetricCard = [](const QString &titleTxt,
                             const QString &valueTxt,
                             const QString &deltaTxt,
                             bool positive,
                             const QString &footTxt) {
        auto *card = new NMetricSummaryCard;
        card->setMinimumHeight(132);
        card->setTitle(titleTxt);
        card->setValueText(valueTxt);
        card->setFootnote(footTxt);
        card->setCornerRadius(12);
        card->trendWidget()->setDirection(positive ? NTrendIndicator::Up : NTrendIndicator::Down);
        card->trendWidget()->setDeltaText(deltaTxt);
        card->setBackgroundColor(QColor(26, 30, 40));
        card->setAccentColor(QColor(56, 125, 255));
        return card;
    };

    auto makeRightInfoCard = [](const QString &objName,
                                const QString &valueText,
                                const QString &descText,
                                double gaugeValue,
                                const QColor &gaugeColor) {
        auto *card = new QFrame;
        card->setObjectName(objName);
        card->setMinimumHeight(122);
        auto *layout = new QHBoxLayout(card);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(14);

        auto *gauge = new NCircularGaugeCard;
        gauge->setFixedSize(100, 100);
        gauge->setBackgroundColor(QColor(20, 24, 32));
        gauge->setTrackColor(QColor(45, 52, 66));
        gauge->setGaugeColor(gaugeColor);
        gauge->setThickness(2);
        gauge->setValue(gaugeValue);
        gauge->setCaption(QString());
        gauge->setCornerRadius(50);

        auto *right = new QWidget;
        auto *v = new QVBoxLayout(right);
        v->setContentsMargins(0, 2, 0, 2);
        v->setSpacing(0);
        auto *value = new QLabel(valueText);
        value->setObjectName(QStringLiteral("NAppRightValue"));
        auto *desc = new QLabel(descText);
        desc->setObjectName(QStringLiteral("NAppRightDesc"));
        v->addStretch(1);
        v->addWidget(value);
        v->addSpacing(2);
        v->addWidget(desc);
        v->addStretch(1);

        layout->addWidget(gauge, 0, Qt::AlignVCenter);
        layout->addWidget(right, 1);
        return card;
    };

    auto *overviewPage = new QWidget;
    auto *overviewLayout = new QVBoxLayout(overviewPage);
    overviewLayout->setContentsMargins(18, 14, 18, 16);
    overviewLayout->setSpacing(14);

    auto *headerRow = new QWidget;
    auto *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    auto *overviewTitle = new QLabel(tr("Overview"));
    overviewTitle->setObjectName(QStringLiteral("NAppOverviewTitle"));
    auto *projectLabel = new QLabel(tr("Show: All Projects"));
    projectLabel->setObjectName(QStringLiteral("NAppProjectLabel"));
    headerLayout->addWidget(overviewTitle);
    headerLayout->addWidget(projectLabel);
    headerLayout->addStretch(1);

    overviewLayout->addWidget(headerRow);

    auto *cardsRow = new QWidget;
    auto *cardsLayout = new QHBoxLayout(cardsRow);
    cardsLayout->setContentsMargins(0, 0, 0, 0);
    cardsLayout->setSpacing(14);
    cardsLayout->addWidget(makeMetricCard(QStringLiteral("Sales"),
                                          QStringLiteral("$27632"),
                                          QStringLiteral("+2.5%"),
                                          true,
                                          QStringLiteral("Compared to ($21340 last year)")));
    cardsLayout->addWidget(makeMetricCard(QStringLiteral("Purchase"),
                                          QStringLiteral("$20199"),
                                          QStringLiteral("+0.5%"),
                                          true,
                                          QStringLiteral("Compared to ($19000 last year)")));
    cardsLayout->addWidget(makeMetricCard(QStringLiteral("Return"),
                                          QStringLiteral("$110"),
                                          QStringLiteral("-1.5%"),
                                          false,
                                          QStringLiteral("Compared to ($165 last year)")));
    cardsLayout->addWidget(makeMetricCard(QStringLiteral("Marketing"),
                                          QStringLiteral("$12632"),
                                          QStringLiteral("+2.5%"),
                                          true,
                                          QStringLiteral("Compared to ($10500 last year)")));
    overviewLayout->addWidget(cardsRow);

    auto *centerRow = new QWidget;
    auto *centerLayout = new QHBoxLayout(centerRow);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(14);

    auto *chartPanel = new QFrame;
    chartPanel->setObjectName(QStringLiteral("NAppBigPanel"));
    auto *chartLayout = new QVBoxLayout(chartPanel);
    chartLayout->setContentsMargins(16, 14, 16, 14);
    chartLayout->setSpacing(12);
    auto *chartHeader = new QWidget;
    auto *chartHeaderLayout = new QHBoxLayout(chartHeader);
    chartHeaderLayout->setContentsMargins(0, 0, 0, 0);
    chartHeaderLayout->setSpacing(8);
    auto *chartTitle = new QLabel(tr("Sales Figures"));
    chartTitle->setObjectName(QStringLiteral("NAppPanelTitle"));
    auto *legendA = new QLabel(tr("Marketing Sales"));
    auto *legendB = new QLabel(tr("Cases Sales"));
    legendA->setObjectName(QStringLiteral("NAppLegendBlue"));
    legendB->setObjectName(QStringLiteral("NAppLegendGreen"));
    chartHeaderLayout->addWidget(chartTitle);
    chartHeaderLayout->addStretch(1);
    chartHeaderLayout->addWidget(legendA);
    chartHeaderLayout->addWidget(legendB);
    chartLayout->addWidget(chartHeader);

    auto *linePanel = new NTimeSeriesLineChartPanel;
    linePanel->setMinimumHeight(336);
    linePanel->setCornerRadius(10);
    linePanel->setBackgroundColor(QColor(24, 28, 36));
    linePanel->setPlotBackgroundColor(QColor(20, 24, 32));
    linePanel->setLineColor(QColor(56, 125, 255));
    linePanel->setSecondaryLineColor(QColor(63, 211, 179));
    linePanel->setLineWidth(3);
    linePanel->setVerticalGridVisible(true);
    linePanel->setHorizontalGridVisible(false);
    static const QStringList kMonths12 = {
        QStringLiteral("Jan"), QStringLiteral("Feb"), QStringLiteral("Mar"), QStringLiteral("Apr"),
        QStringLiteral("May"), QStringLiteral("Jun"), QStringLiteral("Jul"), QStringLiteral("Aug"),
        QStringLiteral("Sep"), QStringLiteral("Oct"), QStringLiteral("Nov"), QStringLiteral("Dec")};
    linePanel->setXAxisCategories(kMonths12);
    QVector<QPointF> marketingPoints;
    QVector<QPointF> casePoints;
    for (int month = 0; month < 12; ++month) {
        marketingPoints.append(QPointF(month, 420 + 120 * qSin(month / 2.2) + 60 * qCos(month / 1.7)));
        casePoints.append(QPointF(month, 360 + 95 * qSin((month + 0.8) / 2.0) + 45 * qCos((month + 0.6) / 1.9)));
    }
    linePanel->setPoints(marketingPoints);
    linePanel->setSecondaryPoints(casePoints);
    chartLayout->addWidget(linePanel, 1);
    centerLayout->addWidget(chartPanel, 3);

    auto *rightCol = new QWidget;
    rightCol->setMinimumWidth(300);
    auto *rightLayout = new QVBoxLayout(rightCol);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);
    rightLayout->addWidget(makeRightInfoCard(QStringLiteral("NAppRightCard"),
                                             QStringLiteral("68%"),
                                             QStringLiteral("Hit Rate this year"),
                                             68.0,
                                             QColor(56, 125, 255)));
    rightLayout->addWidget(makeRightInfoCard(QStringLiteral("NAppRightCard"),
                                             QStringLiteral("76%"),
                                             QStringLiteral("Deals this year"),
                                             76.0,
                                             QColor(49, 202, 171)));

    auto *visitorsCard = new QFrame;
    visitorsCard->setObjectName(QStringLiteral("NAppRightCard"));
    visitorsCard->setMinimumHeight(196);
    auto *visLayout = new QVBoxLayout(visitorsCard);
    visLayout->setContentsMargins(16, 14, 16, 14);
    visLayout->setSpacing(10);
    auto *miniChart = new QtCharts::QChart;
    miniChart->setBackgroundVisible(false);
    miniChart->setMargins(QMargins(0, 0, 0, 0));
    miniChart->legend()->hide();
    auto *miniSeries = new QtCharts::QLineSeries;
    for (int i = 0; i < 18; ++i) {
        miniSeries->append(i, 4.5 + 0.7 * qSin(i / 1.4) + 0.35 * qCos(i / 0.8));
    }
    miniSeries->setPen(QPen(QColor(56, 125, 255), 2.2));
    miniChart->addSeries(miniSeries);
    auto *axX = new QtCharts::QValueAxis;
    auto *axY = new QtCharts::QValueAxis;
    axX->setRange(0, 17);
    axY->setRange(3.2, 6.0);
    axX->setVisible(false);
    axY->setVisible(false);
    miniChart->addAxis(axX, Qt::AlignBottom);
    miniChart->addAxis(axY, Qt::AlignLeft);
    miniSeries->attachAxis(axX);
    miniSeries->attachAxis(axY);
    auto *miniView = new QtCharts::QChartView(miniChart);
    miniView->setRenderHint(QPainter::Antialiasing);
    miniView->setFrameShape(QFrame::NoFrame);
    miniView->setMinimumHeight(66);
    visLayout->addWidget(miniView);
    auto *bottomRow = new QWidget;
    auto *bottomLayout = new QHBoxLayout(bottomRow);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(6);


    auto *visitorValue = new QLabel(QStringLiteral("10,254"));
    visitorValue->setObjectName(QStringLiteral("NAppVisitorValue"));
    auto *visitorDelta = new QLabel(QStringLiteral("1.5%↓"));
    visitorDelta->setObjectName(QStringLiteral("NAppVisitorDelta"));
    bottomLayout->addWidget(visitorValue);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(visitorDelta);
    visLayout->addWidget(bottomRow);
    auto *visitorDesc = new QLabel(QStringLiteral("Visitors this year"));
    visitorDesc->setObjectName(QStringLiteral("NAppRightDesc"));
    visLayout->addWidget(visitorDesc);
    rightLayout->addWidget(visitorsCard, 1);
    centerLayout->addWidget(rightCol, 1);

    overviewLayout->addWidget(centerRow, 1);

    pages->addWidget(overviewPage);

    auto makePlaceholderPage = [](const QString &titleText, const QString &hintText) {
        auto *page = new QWidget;
        auto *layout = new QVBoxLayout(page);
        layout->setContentsMargins(26, 20, 26, 24);
        layout->setSpacing(12);
        auto *title = new QLabel(titleText);
        title->setObjectName(QStringLiteral("NAppOverviewTitle"));
        auto *card = new QFrame;
        card->setObjectName(QStringLiteral("NAppBigPanel"));
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(18, 16, 18, 16);
        auto *hint = new QLabel(hintText);
        hint->setObjectName(QStringLiteral("NAppRightDesc"));
        hint->setWordWrap(true);
        cardLayout->addWidget(hint);
        cardLayout->addStretch(1);
        layout->addWidget(title);
        layout->addWidget(card, 1);
        return page;
    };
    pages->addWidget(makePlaceholderPage(tr("Sales"), tr("Sales 页面：可继续接入销售明细、客户转化、订单看板等组件。")));

    auto *usersPage = new QWidget;
    auto *usersLayout = new QVBoxLayout(usersPage);
    usersLayout->setContentsMargins(26, 20, 26, 24);
    usersLayout->setSpacing(12);
    auto *usersTitle = new QLabel(tr("股票"));
    usersTitle->setObjectName(QStringLiteral("NAppOverviewTitle"));
    auto *usersCard = new QFrame;
    usersCard->setObjectName(QStringLiteral("NAppBigPanel"));
    auto *usersCardLayout = new QVBoxLayout(usersCard);
    usersCardLayout->setContentsMargins(18, 16, 18, 16);
    usersCardLayout->setSpacing(10);

    auto *stockCard = new NStockQuoteCard;
    stockCard->setSubscriptions(QStringList()
                                << QStringLiteral("601288")
                                << QStringLiteral("600519")
                                << QStringLiteral("000001"));
    stockCard->setCurrentSymbol(QStringLiteral("601288"));
    stockCard->setStockName(QStringLiteral("农业银行"));
    stockCard->pinSymbol(QStringLiteral("601288"));
    stockCard->setRefreshIntervalMs(3000);
    stockCard->setAutoRefresh(true);
    stockCard->setBackgroundColor(QColor(24, 28, 36));
    stockCard->setCornerRadius(12);
    usersCardLayout->addWidget(stockCard, 1);
    usersLayout->addWidget(usersTitle);
    usersLayout->addWidget(usersCard, 1);
    pages->addWidget(usersPage);

    pages->addWidget(makePlaceholderPage(tr("Reports"), tr("Reports 页面：可继续接入报表筛选、导出、订阅等模块。")));

    navOverview->setChecked(true);
    pages->setCurrentIndex(0);
    connect(navOverview, &QPushButton::clicked, this, [pages]() { pages->setCurrentIndex(0); });
    connect(navSales, &QPushButton::clicked, this, [pages]() { pages->setCurrentIndex(1); });
    connect(navUsers, &QPushButton::clicked, this, [pages]() { pages->setCurrentIndex(2); });
    connect(navReport, &QPushButton::clicked, this, [pages]() { pages->setCurrentIndex(3); });

    shell->setSidebarWidget(side);
    shell->setContentWidget(pages);

    root->addWidget(titleBar);
    root->addWidget(shell, 1);

    setStyleSheet(QStringLiteral(
                      "NAppWindow { background: #14161c; font-family: 'Segoe UI'; }"
                      "#NAppTitleBar { background: #1e222c; border: none; border-bottom: 1px solid #2a3040; }"
                      "#NAppTitle { color: #eef1f7; font-size: 16px; font-weight: 700; }"
                      "#NAppSubTitle { color: #8b97ab; font-size: 13px; }"
                      "#NAppSidePanel { background: #1a1e29; border-right: 1px solid #2a3040; }"
                      "#NAppLogo { background: #2f66ff; color: #f4f7ff; border-radius: 20px; font-size: 18px; font-weight: 800; }"
                      "QToolButton[navButton='true'] { text-align: left; border: none; border-radius: 8px; color: #97a5bf; padding: 6px 10px; font-size: 13px; }"
                      "QToolButton[navButton='true']:hover { background: rgba(255,255,255,0.05); color: #d9e3ff; }"
                      "QToolButton[navButton='true']:checked { background: rgba(47,102,255,0.20); color: #d9e3ff; font-weight: 700; }"
                      "#NAppOverviewTitle { color: #e9eef9; font-size: 20px; font-weight: 700; }"
                      "#NAppProjectLabel { color: #93a3bf; font-size: 13px; }"
                      "#NAppBigPanel { background: #1a1f2a; border: 1px solid #2a3242; border-radius: 12px; }"
                      "#NAppPanelTitle { color: #aeb7c8; font-size: 17px; font-weight: 700; }"
                      "#NAppLegendBlue { color: #98a5ba; font-size: 13px; }"
                      "#NAppLegendGreen { color: #98a5ba; font-size: 13px; }"
                      "#NAppRightCard { background: #1a1f2a; border: 1px solid #2a3242; border-radius: 12px; }"
                      "#NAppRightValue { color: #f0f4ff; font-size: 25px; font-weight: 700; }"
                      "#NAppRightDesc { color: #8d9ab2; font-size: 15px; }"
                      "#NAppVisitorValue { color: #f0f4ff; font-size: 25px; font-weight: 700; }"
                      "#NAppVisitorDelta { color: #d85a66; font-size: 18px; font-weight: 700; }"
                      "QLabel { color: #d3dbec; }"));

    const QColor ac(106, 149, 255);
    const QColor sideBg(34, 38, 48);
    shell->setAccentColor(ac);
    shell->setSidebarBackgroundColor(sideBg);
    shell->setContentBackgroundColor(QColor(18, 22, 30));
}
