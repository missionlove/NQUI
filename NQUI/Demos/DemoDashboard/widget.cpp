#include "widget.h"

#include "nappshell.h"
#include "ndashboardcontentlayout.h"
#include "ndashboardheader.h"
#include "nnotificationbellbutton.h"
#include "npagetitlewithfilter.h"
#include "nprimaryactionbutton.h"
#include "nsidebarnavigation.h"
#include "nsidebarnavitem.h"
#include "nuserprofilecompact.h"
#include "ntrendindicator.h"
#include "nmetricsummarycard.h"
#include "ntimeserieslinechartpanel.h"
#include "ncirculargaugecard.h"
#include "nsparklinemetriccard.h"
#include "ngroupedbarchartpanel.h"

#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMargins>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSlider>
#include <QStyle>
#include <QVBoxLayout>
#include <QtMath>

namespace {

QWidget *wrapDemo(const QString &title, QWidget *demo, QWidget *controls)
{
    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("DemoCard"));
    auto *v = new QVBoxLayout(card);
    v->setContentsMargins(24, 20, 24, 22);
    v->setSpacing(16);
    auto *t = new QLabel(title);
    t->setObjectName(QStringLiteral("DemoCardTitle"));
    t->setWordWrap(true);
    v->addWidget(t);
    v->addSpacing(6);
    v->addWidget(demo);
    if (controls) {
        v->addSpacing(10);
        auto *sep = new QFrame;
        sep->setObjectName(QStringLiteral("DemoCardSep"));
        sep->setFixedHeight(1);
        v->addWidget(sep);
        v->addSpacing(10);
        v->addWidget(controls);
    }
    return card;
}

QWidget *hStretchRow(QWidget *parent, const QVector<QWidget *> &widgets, const QVector<int> &stretch,
                     int hSpacing = 36, const QMargins &margins = QMargins(0, 10, 0, 10))
{
    auto *w = new QWidget(parent);
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(margins);
    h->setSpacing(hSpacing);
    for (int i = 0; i < widgets.size(); ++i) {
        const int s = (i < stretch.size()) ? stretch.at(i) : 1;
        QWidget *cw = widgets.at(i);
        cw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        h->addWidget(cw, s);
    }
    return w;
}

} // namespace

DashboardDemoWidget::DashboardDemoWidget(QWidget *parent)
    : NFramelessWidget(parent),
      m_shadowHost(nullptr),
      m_chromeFrame(nullptr),
      m_rootLayout(nullptr),
      m_shadowLayout(nullptr),
      m_chromeLayout(nullptr),
      m_shadowEffect(nullptr),
      m_maxButton(nullptr)
{
    setObjectName(QStringLiteral("DemoDashboardWindow"));
#ifdef Q_OS_WIN
    setAttribute(Qt::WA_TranslucentBackground, false);
#else
    setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    setMinimumSize(960, 600);
    setMoveAreaHeight(48);
    setResizeBorderWidth(8);
    setResizeSensitivity(10);
    setShadowStyle(NFramelessWidget::ShadowWindowsSafe);
    setCornerRadius(12);
    setShadowPadding(8);
    setAttribute(Qt::WA_StyledBackground, true);

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(0, 0, 0, 0);
    m_rootLayout->setSpacing(0);

    m_shadowHost = new QFrame(this);
    m_shadowHost->setObjectName(QStringLiteral("ShadowHost"));
    m_shadowHost->setProperty("active", true);
    m_shadowLayout = new QVBoxLayout(m_shadowHost);
    m_shadowLayout->setContentsMargins(0, 0, 0, 0);
    m_shadowLayout->setSpacing(0);



    m_chromeFrame = new QFrame(m_shadowHost);
    m_chromeFrame->setObjectName(QStringLiteral("ChromeFrame"));
    m_chromeFrame->setProperty("active", true);
    m_chromeLayout = new QVBoxLayout(m_chromeFrame);
    m_chromeLayout->setContentsMargins(0, 0, 0, 0);
    m_chromeLayout->setSpacing(0);

    m_chromeFrame->setMinimumSize(800,600);
    m_rootLayout->addWidget(m_chromeFrame);


#ifndef Q_OS_WIN
    m_shadowEffect = new QGraphicsDropShadowEffect(m_chromeFrame);
    m_shadowEffect->setBlurRadius(22);
    m_shadowEffect->setOffset(0, 3);
    m_shadowEffect->setColor(QColor(0, 0, 0, 150));
    m_chromeFrame->setGraphicsEffect(m_shadowEffect);
#endif

    auto *titleBar = new QFrame(m_chromeFrame);
    titleBar->setObjectName(QStringLiteral("TitleBar"));
    titleBar->setFixedHeight(48);

    auto *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(18, 0, 12, 0);
    titleLayout->setSpacing(12);

    auto *title = new QLabel(tr("NDashboard \u7ec4\u4ef6\u6f14\u793a"), titleBar);
    auto *subTitle = new QLabel(
        tr("\u65e0\u8fb9\u6846\u7a97\u53e3 \u00b7 \u62d6\u52a8\u6807\u9898\u680f\u79fb\u52a8 \u00b7 \u5185\u5bb9\u533a\u6ed1\u5757\u9a8c\u8bc1\u4e3b\u9898 API"),
        titleBar);
    auto *minBtn = new QPushButton(titleBar);
    m_maxButton = new QPushButton(titleBar);
    auto *closeBtn = new QPushButton(titleBar);
    minBtn->setFixedSize(36, 30);
    m_maxButton->setFixedSize(36, 30);
    closeBtn->setFixedSize(36, 30);
    minBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    closeBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    minBtn->setIconSize(QSize(12, 12));
    m_maxButton->setIconSize(QSize(12, 12));
    closeBtn->setIconSize(QSize(12, 12));
    minBtn->setFocusPolicy(Qt::NoFocus);
    m_maxButton->setFocusPolicy(Qt::NoFocus);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    subTitle->setObjectName(QStringLiteral("SubTitle"));
    minBtn->setObjectName(QStringLiteral("TitleBtn"));
    m_maxButton->setObjectName(QStringLiteral("TitleBtn"));
    closeBtn->setObjectName(QStringLiteral("TitleBtnClose"));
    minBtn->setToolTip(tr("\u6700\u5c0f\u5316"));
    closeBtn->setToolTip(tr("\u5173\u95ed"));
    refreshMaximizeButton();

    titleLayout->addWidget(title);
    titleLayout->addWidget(subTitle, 1);
    titleLayout->addWidget(minBtn);
    titleLayout->addWidget(m_maxButton);
    titleLayout->addWidget(closeBtn);

    auto *content = new QFrame(m_chromeFrame);
    content->setObjectName(QStringLiteral("DashContent"));
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    auto *inner = new QFrame(content);
    inner->setObjectName(QStringLiteral("DashInner"));
    auto *innerLay = new QVBoxLayout(inner);
    innerLay->setContentsMargins(16, 16, 16, 16);
    innerLay->setSpacing(0);

    auto *shell = new NAppShell(inner);
    shell->setSidebarWidth(280);
    innerLay->addWidget(shell, 1);
    contentLayout->addWidget(inner, 1);

    m_chromeLayout->addWidget(titleBar);
    m_chromeLayout->addWidget(content, 1);
    m_rootLayout->addWidget(m_shadowHost);

    auto *nav = new NSidebarNavigation;
    nav->addSectionTitle(tr("\u6a21\u5757"));
    auto *navHome = new NSidebarNavItem(tr("\u7ec4\u4ef6\u603b\u89c8"));
    navHome->setChecked(true);
    auto *navMsg = new NSidebarNavItem(tr("\u6d88\u606f\u4e2d\u5fc3"));
    navMsg->setBadgeCount(12);
    nav->addNavItem(navHome);
    nav->addNavItem(navMsg);
    nav->addSectionTitle(tr("\u5176\u5b83"));
    nav->addNavItem(new NSidebarNavItem(tr("\u8bbe\u7f6e")));

    auto *page = new NDashboardContentLayout;
    page->setMargins(36, 28, 36, 40);
    page->setVerticalSpacing(30);
    page->setHorizontalSpacing(20);
    page->setColumnSpacing(30);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(page);
    scroll->setObjectName(QStringLiteral("DashScroll"));

    shell->setSidebarWidget(nav);
    shell->setContentWidget(scroll);

    auto *shellHueRow = new QFrame;
    shellHueRow->setObjectName(QStringLiteral("DemoToolbar"));
    auto *sh = new QHBoxLayout(shellHueRow);
    sh->setContentsMargins(22, 16, 22, 16);
    sh->setSpacing(18);
    auto *hueLab = new QLabel(tr("AppShell \u5f3a\u8c03\u8272\uff08HSV H\uff09"));
    hueLab->setObjectName(QStringLiteral("ToolbarLabel"));
    auto *shellSlider = new QSlider(Qt::Horizontal);
    shellSlider->setRange(0, 359);
    shellSlider->setValue(220);
    shellSlider->setMinimumHeight(22);
    sh->addWidget(hueLab);
    sh->addWidget(shellSlider, 1);
    page->addWidget(shellHueRow);
    page->addSpacing(8);

    auto syncShellTheme = [shell, nav, navHome, navMsg](int hue) {
        const QColor ac = QColor::fromHsv(hue, 210, 235);
        const QColor side = QColor::fromHsv(hue, 42, 54);
        shell->setAccentColor(ac);
        shell->setContentBackgroundColor(QColor(20, 23, 29));
        shell->setSidebarBackgroundColor(side.darker(108));
        nav->setBackgroundColor(side);
        nav->setForegroundColor(QColor(238, 240, 245));
        nav->setAccentColor(ac);
        navHome->setBackgroundColor(side);
        navHome->setForegroundColor(nav->foregroundColor());
        navHome->setAccentColor(ac);
        navMsg->setBackgroundColor(side);
        navMsg->setForegroundColor(nav->foregroundColor());
        navMsg->setAccentColor(ac);
    };
    QObject::connect(shellSlider, &QSlider::valueChanged, this, syncShellTheme);
    syncShellTheme(shellSlider->value());

    // --- \u9876\u680f\u4e0e\u7b5b\u9009 ---
    auto *header = new NDashboardHeader;
    header->setMinimumHeight(58);
    header->setLeftWidget(new QLabel(tr("NDashboardHeader / \u5de6\u4fa7")));
    auto *cLab = new QLabel(tr("\u5c45\u4e2d\u5bfc\u822a\u4e0e\u72b6\u6001"));
    cLab->setAlignment(Qt::AlignCenter);
    header->setCenterWidget(cLab);
    auto *hdrRight = new QHBoxLayout;
    hdrRight->setContentsMargins(0, 0, 0, 0);
    hdrRight->setSpacing(8);
    hdrRight->addWidget(new QLabel(tr("\u533a\u5757\u64cd\u4f5c")));
    auto *hdrRightW = new QWidget;
    hdrRightW->setLayout(hdrRight);
    header->setRightWidget(hdrRightW);
    auto *hdrRad = new QSlider(Qt::Horizontal);
    hdrRad->setRange(0, 24);
    hdrRad->setValue(10);
    QObject::connect(hdrRad, &QSlider::valueChanged, header, &NDashboardWidgetBase::setCornerRadius);
    auto *hdrForm = new QFormLayout;
    hdrForm->setHorizontalSpacing(20);
    hdrForm->setVerticalSpacing(12);
    hdrForm->addRow(tr("\u5361\u7247\u5706\u89d2"), hdrRad);
    auto *hdrCtrl = new QWidget;
    hdrCtrl->setLayout(hdrForm);

    auto *pt = new NPageTitleWithFilter;
    pt->setTitle(tr("\u9875\u9762\u6807\u9898 + \u7b5b\u9009"));
    pt->clearFilterItems();
    pt->addFilterItem(tr("\u6700\u8fd1 7 \u5929"));
    pt->addFilterItem(tr("\u6700\u8fd1 30 \u5929"));
    auto *ptEcho = new QLabel(tr("\u5f53\u524d\u7b5b\u9009\u7d22\u5f15: 0"));
    QObject::connect(pt, &NPageTitleWithFilter::filterIndexChanged, ptEcho, [ptEcho](int i) {
        ptEcho->setText(tr("\u5f53\u524d\u7b5b\u9009\u7d22\u5f15: %1").arg(i));
    });
    auto *ptWrap = new QWidget;
    auto *ptLay = new QVBoxLayout(ptWrap);
    ptLay->setContentsMargins(0, 10, 0, 0);
    ptLay->setSpacing(6);
    ptLay->addWidget(ptEcho);

    page->addWidget(wrapDemo(QStringLiteral("NDashboardHeader"), header, hdrCtrl));
    page->addSpacing(6);
    page->addWidget(wrapDemo(QStringLiteral("NPageTitleWithFilter"), pt, ptWrap));
    page->addSpacing(10);

    // --- \u64cd\u4f5c\u4e0e\u8eab\u4efd ---
    auto *primary = new NPrimaryActionButton(tr("\u4e3b\u64cd\u4f5c"));
    auto *bell = new NNotificationBellButton;
    bell->setBadgeCount(1);
    bell->setBackgroundColor(QColor(32, 36, 44));
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(16);
    btnRow->addWidget(primary, 0, Qt::AlignLeft);
    btnRow->addWidget(bell, 0, Qt::AlignLeft);
    btnRow->addStretch(1);
    auto *btnHost = new QWidget;
    btnHost->setLayout(btnRow);
    auto *btnHue = new QSlider(Qt::Horizontal);
    btnHue->setRange(0, 359);
    btnHue->setValue(220);
    QObject::connect(btnHue, &QSlider::valueChanged, this, [primary, bell](int h) {
        const QColor c = QColor::fromHsv(h, 200, 230);
        primary->setAccentColor(c);
        bell->setAccentColor(c);
    });
    auto *bf = new QFormLayout;
    bf->setHorizontalSpacing(18);
    bf->setVerticalSpacing(12);
    bf->addRow(tr("\u6309\u94ae\u5f3a\u8c03\u8272 (H)"), btnHue);
    auto *bc = new QWidget;
    bc->setLayout(bf);

    auto *profile = new NUserProfileCompact;
    profile->setDisplayName(tr("\u6f14\u793a\u7528\u6237"));
    profile->setSubtitle(tr("NDashboard \u00b7 Demo"));
    profile->setAvatarLetter(QLatin1Char('D'));
    profile->setAvatarSize(36);
    QPixmap profileAvatar(120, 120);
    profileAvatar.fill(QColor(66, 86, 130));
    {
        QPainter p(&profileAvatar);
        p.setRenderHint(QPainter::Antialiasing, true);
        QFont f = p.font();
        f.setBold(true);
        f.setPixelSize(56);
        p.setFont(f);
        p.setPen(QColor(232, 238, 250));
        p.drawText(profileAvatar.rect(), Qt::AlignCenter, QStringLiteral("D"));
    }
    profile->setAvatarPixmap(profileAvatar);
    auto *pRad = new QSlider(Qt::Horizontal);
    pRad->setRange(0, 20);
    pRad->setValue(10);
    QObject::connect(pRad, &QSlider::valueChanged, profile, &NDashboardWidgetBase::setCornerRadius);
    auto *pSize = new QSlider(Qt::Horizontal);
    pSize->setRange(20, 64);
    pSize->setValue(36);
    QObject::connect(pSize, &QSlider::valueChanged, profile, &NUserProfileCompact::setAvatarSize);
    auto *pf = new QFormLayout;
    pf->setHorizontalSpacing(18);
    pf->setVerticalSpacing(12);
    pf->addRow(tr("\u5934\u50cf\u5361\u7247\u5706\u89d2"), pRad);
    pf->addRow(tr("\u5934\u50cf\u5c3a\u5bf8"), pSize);
    auto *pw = new QWidget;
    pw->setLayout(pf);

    page->addWidget(hStretchRow(page, QVector<QWidget *>()
                                              << wrapDemo(QStringLiteral("NPrimaryActionButton + NNotificationBellButton"), btnHost, bc)
                                              << wrapDemo(QStringLiteral("NUserProfileCompact"), profile, pw),
                                QVector<int>() << 1 << 1));
    page->addSpacing(10);

    // --- \u8d8b\u52bf\u4e0e\u6307\u6807\u5361 ---
    auto *trend = new NTrendIndicator;
    trend->setMinimumHeight(44);
    trend->setDirection(NTrendIndicator::Up);
    trend->setDeltaText(QStringLiteral("+12.4%"));
    auto *tHue = new QSlider(Qt::Horizontal);
    tHue->setRange(0, 359);
    tHue->setValue(140);
    QObject::connect(tHue, &QSlider::valueChanged, trend, [trend](int hue) {
        trend->setPositiveColor(QColor::fromHsv(hue, 200, 200));
    });
    auto *th = new QHBoxLayout;
    th->setSpacing(14);
    th->addWidget(new QLabel(tr("\u6b63\u5411\u8272 H")), 0);
    th->addWidget(tHue, 1);
    auto *tw = new QWidget;
    tw->setLayout(th);

    auto *metric = new NMetricSummaryCard;
    metric->setMinimumHeight(168);
    metric->setTitle(tr("\u6708\u5ea6\u8425\u6536"));
    metric->setValueText(QStringLiteral("CNY 128,400"));
    metric->setFootnote(tr("\u540c\u6bd4\u4e0a\u6708"));
    metric->trendWidget()->setDirection(NTrendIndicator::Up);
    metric->trendWidget()->setDeltaText(QStringLiteral("+8.2%"));
    auto *mBg = new QSlider(Qt::Horizontal);
    mBg->setRange(0, 100);
    mBg->setValue(40);
    QObject::connect(mBg, &QSlider::valueChanged, metric, [metric](int v) {
        metric->setBackgroundColor(QColor(v + 20, v + 24, v + 30));
    });
    auto *mf = new QFormLayout;
    mf->setHorizontalSpacing(18);
    mf->setVerticalSpacing(12);
    mf->addRow(tr("\u80cc\u666f\u4eae\u5ea6"), mBg);
    auto *mw = new QWidget;
    mw->setLayout(mf);

    page->addWidget(hStretchRow(page, QVector<QWidget *>()
                                              << wrapDemo(QStringLiteral("NTrendIndicator"), trend, tw)
                                              << wrapDemo(QStringLiteral("NMetricSummaryCard"), metric, mw),
                                QVector<int>() << 1 << 2));
    page->addSpacing(10);

    // --- \u6298\u7ebf\u56fe ---
    auto *linePanel = new NTimeSeriesLineChartPanel;
    linePanel->setMinimumHeight(248);
    QVector<QPointF> pts;
    for (int i = 0; i < 20; ++i) {
        pts.append(QPointF(i, 15 + 6 * qSin(i / 3.0)));
    }
    linePanel->setPoints(pts);
    auto *lw = new QSlider(Qt::Horizontal);
    lw->setRange(1, 6);
    lw->setValue(2);
    QObject::connect(lw, &QSlider::valueChanged, linePanel, &NTimeSeriesLineChartPanel::setLineWidth);
    auto *lf = new QFormLayout;
    lf->setHorizontalSpacing(18);
    lf->setVerticalSpacing(12);
    lf->addRow(tr("\u7ebf\u5bbd"), lw);
    auto *lwW = new QWidget;
    lwW->setLayout(lf);
    page->addWidget(wrapDemo(QStringLiteral("NTimeSeriesLineChartPanel"), linePanel, lwW));
    page->addSpacing(10);

    // --- \u4eea\u8868\u4e0e\u5fae\u8d8b\u7ebf ---
    auto *gauge = new NCircularGaugeCard;
    gauge->setMinimumWidth(240);
    gauge->setCaption(tr("\u5b8c\u6210\u7387"));
    auto *gv = new QSlider(Qt::Horizontal);
    gv->setRange(0, 100);
    gv->setValue(62);
    QObject::connect(gv, &QSlider::valueChanged, gauge, &NCircularGaugeCard::setValue);
    auto *gt = new QSlider(Qt::Horizontal);
    gt->setRange(4, 24);
    gt->setValue(10);
    QObject::connect(gt, &QSlider::valueChanged, gauge, &NCircularGaugeCard::setThickness);
    auto *gf = new QFormLayout;
    gf->setHorizontalSpacing(18);
    gf->setVerticalSpacing(12);
    gf->addRow(tr("\u6570\u503c"), gv);
    gf->addRow(tr("\u5f27\u7c97"), gt);
    auto *gw = new QWidget;
    gw->setLayout(gf);

    auto *spark = new NSparklineMetricCard;
    spark->setMinimumHeight(168);
    spark->setTitle(tr("\u6d3b\u8dc3\u7528\u6237"));
    spark->setValueText(QStringLiteral("18.2k"));
    QVector<double> sy;
    for (int i = 0; i < 16; ++i) {
        sy.append(2.5 + qCos(i / 2.5) * 0.8);
    }
    spark->setSparkline(sy);
    auto *spH = new QSlider(Qt::Horizontal);
    spH->setRange(0, 359);
    spH->setValue(200);
    QObject::connect(spH, &QSlider::valueChanged, spark, [spark](int h) {
        spark->setLineColor(QColor::fromHsv(h, 200, 230));
    });
    auto *sf = new QFormLayout;
    sf->setHorizontalSpacing(18);
    sf->setVerticalSpacing(12);
    sf->addRow(tr("\u6298\u7ebf\u8272\u76f8"), spH);
    auto *sw = new QWidget;
    sw->setLayout(sf);

    page->addWidget(hStretchRow(page, QVector<QWidget *>()
                                              << wrapDemo(QStringLiteral("NCircularGaugeCard"), gauge, gw)
                                              << wrapDemo(QStringLiteral("NSparklineMetricCard"), spark, sw),
                                QVector<int>() << 0 << 1));
    page->addSpacing(10);

    // --- \u67f1\u72b6\u56fe ---
    auto *bars = new NGroupedBarChartPanel;
    bars->setMinimumHeight(280);
    auto *barHue = new QSlider(Qt::Horizontal);
    barHue->setRange(0, 359);
    barHue->setValue(40);
    QObject::connect(barHue, &QSlider::valueChanged, bars, [bars](int h) {
        QVector<QColor> pal;
        pal << QColor::fromHsv(h, 200, 230) << QColor::fromHsv((h + 80) % 360, 180, 220)
            << QColor::fromHsv((h + 160) % 360, 160, 210);
        bars->setBarColors(pal);
    });
    auto *barf = new QFormLayout;
    barf->setHorizontalSpacing(18);
    barf->setVerticalSpacing(12);
    barf->addRow(tr("\u8c03\u8272\u76d8\u8272\u76f8"), barHue);
    auto *barw = new QWidget;
    barw->setLayout(barf);
    page->addWidget(wrapDemo(QStringLiteral("NGroupedBarChartPanel"), bars, barw));

    page->addStretch(1);

    QObject::connect(shellSlider, &QSlider::valueChanged, header, [header](int hue) {
        header->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, pt, [pt](int hue) {
        pt->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, profile, [profile](int hue) {
        profile->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, trend, [trend](int hue) {
        trend->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, metric, [metric](int hue) {
        metric->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, linePanel, [linePanel](int hue) {
        linePanel->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, gauge, [gauge](int hue) {
        gauge->setAccentColor(QColor::fromHsv(hue, 210, 235));
        gauge->setGaugeColor(QColor::fromHsv(hue, 200, 230));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, spark, [spark](int hue) {
        spark->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });
    QObject::connect(shellSlider, &QSlider::valueChanged, bars, [bars](int hue) {
        bars->setAccentColor(QColor::fromHsv(hue, 210, 235));
    });

    setStyleSheet(QStringLiteral(
        "#DemoDashboardWindow { background: #12141a; }"
        "#DemoDashboardWindow[maximized=\"true\"] { background: #12141a; }"
        "#ShadowHost { background: transparent; border: none; }"
        "#ShadowHost[maximized=\"true\"] { background: #12141a; }"
        "#ChromeFrame { background: #161922; border: none; border-radius: 0px; }"
        "#TitleBar { background: #1e222c; border: none; border-bottom: 1px solid #2a3040; }"
        "#SubTitle { color: #8b97ab; font-size: 12px; }"
        "#DashContent { background: #161922; border: none; }"
        "#DashInner { background: transparent; border: none; }"
        "#DashScroll { background: transparent; border: none; }"
        "QLabel { color: #e8ecf4; font-size: 13px; }"
        "#ToolbarLabel { color: #aeb9cc; font-size: 13px; font-weight: 600; }"
        "#DemoToolbar { background: #1a1e28; border: 1px solid #2a3140; border-radius: 10px; }"
        "#DemoCard { background: #1c202a; border: 1px solid #2a3142; border-radius: 12px; margin-top: 2px; margin-bottom: 2px; }"
        "#DemoCardTitle { color: #c5cedd; font-size: 12px; font-weight: 700; }"
        "#DemoCardSep { background: #2a3142; max-height: 1px; }"
        "#TitleBtn { background: #2c323d; color: #e8ecf4; border: 1px solid #3d4658; border-radius: 6px; padding: 4px; }"
        "#TitleBtn:hover { background: #3a4250; border-color: #556070; }"
        "#TitleBtnClose { background: #2c323d; color: #e8ecf4; border: 1px solid #3d4658; border-radius: 6px; padding: 4px; }"
        "#TitleBtnClose:hover { background: #c42b2b; border-color: #e04848; }"
        "QSlider::groove:horizontal { height: 6px; background: #2a3142; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 16px; margin: -5px 0; background: #5b7fd9; border-radius: 8px; border: 1px solid #7a9ef0; }"
        "QSlider::sub-page:horizontal { background: #3d5280; border-radius: 3px; }"));

    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(m_maxButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    refreshWindowVisualState();
}

void DashboardDemoWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange || event->type() == QEvent::ActivationChange) {
        refreshMaximizeButton();
        refreshWindowVisualState();
    }
    NFramelessWidget::changeEvent(event);
}

void DashboardDemoWidget::refreshMaximizeButton()
{
    if (!m_maxButton) {
        return;
    }
    m_maxButton->setIcon(style()->standardIcon(isMaximized() ? QStyle::SP_TitleBarNormalButton : QStyle::SP_TitleBarMaxButton));
    m_maxButton->setToolTip(isMaximized() ? tr("\u8fd8\u539f") : tr("\u6700\u5927\u5316"));
}

void DashboardDemoWidget::refreshWindowVisualState()
{
    if (!m_chromeFrame) {
        return;
    }
    const bool fillScreen = isMaximized() || isFullScreen();
    setProperty("maximized", fillScreen);
    if (m_shadowHost) {
        m_shadowHost->setProperty("active", isActiveWindow());
        m_shadowHost->setProperty("maximized", fillScreen);
    }
    m_chromeFrame->setProperty("active", isActiveWindow());
    m_chromeFrame->setProperty("maximized", fillScreen);

    if (m_shadowEffect) {
        m_shadowEffect->setEnabled(!fillScreen);
    }

    style()->unpolish(this);
    style()->polish(this);
    update();
    if (m_shadowHost) {
        style()->unpolish(m_shadowHost);
        style()->polish(m_shadowHost);
        m_shadowHost->update();
    }
    style()->unpolish(m_chromeFrame);
    style()->polish(m_chromeFrame);
    m_chromeFrame->update();
}
