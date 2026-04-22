#include "widget.h"
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QStyle>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

Widget::Widget(QWidget *parent)
    : NFramelessWidget(parent),
      m_shadowHost(nullptr),
      m_chromeFrame(nullptr),
      m_rootLayout(nullptr),
      m_shadowLayout(nullptr),
      m_chromeLayout(nullptr),
      m_shadowEffect(nullptr),
      m_maxButton(nullptr),
      m_snapPreview(nullptr),
      m_snapMode(SnapNone)
{
    setObjectName("DemoFramelessWindow");
#ifdef Q_OS_WIN
    setAttribute(Qt::WA_TranslucentBackground, false);
#else
    setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    setMinimumSize(720, 420);
    setMoveAreaHeight(48);
    setResizeBorderWidth(8);
    setResizeSensitivity(10);
    setShadowStyle(NFramelessWidget::ShadowWindowsSafe);
    setCornerRadius(12);
    // 仅由插件 setContentsMargins 留出外环，避免与 m_shadowLayout 双重叠加造成“双框”
    setShadowPadding(8);

    setAttribute(Qt::WA_StyledBackground, true);

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(0, 0, 0, 0);
    m_rootLayout->setSpacing(0);

    m_shadowHost = new QFrame(this);
    m_shadowHost->setObjectName("ShadowHost");
    m_shadowHost->setProperty("active", true);
    m_shadowLayout = new QVBoxLayout(m_shadowHost);
    m_shadowLayout->setContentsMargins(0, 0, 0, 0);
    m_shadowLayout->setSpacing(0);

    m_chromeFrame = new QFrame(m_shadowHost);
    m_chromeFrame->setObjectName("ChromeFrame");
    m_chromeFrame->setProperty("active", true);
    m_chromeLayout = new QVBoxLayout(m_chromeFrame);
    m_chromeLayout->setContentsMargins(1, 1, 1, 1);
    m_chromeLayout->setSpacing(0);

#ifndef Q_OS_WIN
    m_shadowEffect = new QGraphicsDropShadowEffect(m_chromeFrame);
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setOffset(0, 2);
    m_shadowEffect->setColor(QColor(0, 0, 0, 140));
    m_chromeFrame->setGraphicsEffect(m_shadowEffect);
#endif

    QFrame *titleBar = new QFrame(m_chromeFrame);
    titleBar->setObjectName("TitleBar");
    titleBar->setFixedHeight(48);

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(14, 0, 10, 0);
    titleLayout->setSpacing(8);

    QLabel *title = new QLabel("NFramelessWidget Demo", titleBar);
    QLabel *subTitle = new QLabel("Native move/resize + UX polish", titleBar);
    QPushButton *minBtn = new QPushButton(titleBar);
    m_maxButton = new QPushButton(titleBar);
    QPushButton *closeBtn = new QPushButton(titleBar);
    minBtn->setFixedSize(32, 30);
    m_maxButton->setFixedSize(32, 30);
    closeBtn->setFixedSize(32, 30);
    minBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    closeBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    minBtn->setIconSize(QSize(12, 12));
    m_maxButton->setIconSize(QSize(12, 12));
    closeBtn->setIconSize(QSize(12, 12));
    minBtn->setFocusPolicy(Qt::NoFocus);
    m_maxButton->setFocusPolicy(Qt::NoFocus);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    subTitle->setObjectName("SubTitle");
    minBtn->setToolTip("Minimize");
    closeBtn->setToolTip("Close");
    refreshMaximizeButton();

    titleLayout->addWidget(title);
    titleLayout->addWidget(subTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(minBtn);
    titleLayout->addWidget(m_maxButton);
    titleLayout->addWidget(closeBtn);

    QFrame *content = new QFrame(m_chromeFrame);
    content->setObjectName("Content");

    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 24, 24, 24);
    contentLayout->setSpacing(12);

    QLabel *desc1 = new QLabel("This window uses NFramelessWidget with native system move/resize.", content);
    QLabel *desc2 = new QLabel("UX tips: drag title area to move, double-click title area to maximize/restore, drag edges/corners to resize.", content);
    QLabel *desc3 = new QLabel("Sensitivity is now configurable via setResizeBorderWidth + setResizeSensitivity.", content);
    desc1->setWordWrap(true);
    desc2->setWordWrap(true);
    desc3->setWordWrap(true);
    contentLayout->addWidget(desc1);
    contentLayout->addWidget(desc2);
    contentLayout->addWidget(desc3);
    contentLayout->addStretch();

    m_chromeLayout->addWidget(titleBar);
    m_chromeLayout->addWidget(content);
    m_shadowLayout->addWidget(m_chromeFrame);
    m_rootLayout->addWidget(m_shadowHost);

    setStyleSheet(
        "#DemoFramelessWindow { background: #1f1f1f; }"
        "#DemoFramelessWindow[maximized=\"true\"] { background: #1f1f1f; }"
        "#ShadowHost { background: transparent; border: none; }"
        "#ShadowHost[maximized=\"true\"] { background: #1f1f1f; }"
        "#ChromeFrame { background: #1f1f1f; border: none; border-radius: 0px; }"
        "#ChromeFrame[active=\"true\"] { border: none; }"
        "#TitleBar { background: #292d33; border: none; border-radius: 0px; }"
        "#SubTitle { color: #a6b2c0; font-size: 12px; }"
        "#Content { background: #1f1f1f; border: none; border-radius: 0px; }"
        "QLabel { color: #f0f0f0; font-size: 13px; }"
        "QPushButton { background: #383d45; color: #f0f0f0; border: 1px solid #59616d; border-radius: 0px; }"
        "QPushButton:hover { background: #4a5563; border-color: #7f91a8; }"
        "QPushButton:pressed { background: #303640; }"
    );
    refreshWindowVisualState();

    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(m_maxButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
}

Widget::~Widget()
{
    if (m_snapPreview) {
        m_snapPreview->close();
        delete m_snapPreview;
        m_snapPreview = nullptr;
    }
}

void Widget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange || event->type() == QEvent::ActivationChange) {
        refreshMaximizeButton();
        refreshWindowVisualState();
        if (!isActiveWindow()) {
            hideSnapPreview();
        }
    }
    NFramelessWidget::changeEvent(event);
}

void Widget::onMoveDragUpdated(const QPoint &globalPos)
{
    m_snapMode = updateSnapPreview(globalPos);
}

void Widget::onMoveDragFinished(const QPoint &globalPos)
{
    const SnapPreviewMode mode = updateSnapPreview(globalPos);
    applySnapFromMode(mode);
    hideSnapPreview();
}

void Widget::refreshMaximizeButton()
{
    if (!m_maxButton) {
        return;
    }
    m_maxButton->setIcon(style()->standardIcon(isMaximized()
                                                ? QStyle::SP_TitleBarNormalButton
                                                : QStyle::SP_TitleBarMaxButton));
    m_maxButton->setToolTip(isMaximized() ? "Restore" : "Maximize");
}

void Widget::refreshWindowVisualState()
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

    if (m_rootLayout) {
        m_rootLayout->setContentsMargins(0, 0, 0, 0);
    }
    if (m_shadowEffect) {
        m_shadowEffect->setEnabled(!fillScreen);
    }
    if (m_shadowLayout) {
        m_shadowLayout->setContentsMargins(0, 0, 0, 0);
    }
    if (m_chromeLayout) {
        m_chromeLayout->setContentsMargins(0, 0, 0, 0);
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

void Widget::ensureSnapPreviewWidget()
{
    if (m_snapPreview) {
        return;
    }

    m_snapPreview = new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_snapPreview->setAttribute(Qt::WA_ShowWithoutActivating, true);
    m_snapPreview->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_snapPreview->setWindowOpacity(0.22);
    m_snapPreview->setStyleSheet("background: rgba(106, 149, 255, 0.35); border: none; border-radius: 0px;");
}

Widget::SnapPreviewMode Widget::updateSnapPreview(const QPoint &globalPos)
{
    if (!isVisible()) {
        hideSnapPreview();
        return SnapNone;
    }

    QScreen *targetScreen = QGuiApplication::screenAt(globalPos);
    if (!targetScreen) {
        targetScreen = screen();
    }
    if (!targetScreen) {
        hideSnapPreview();
        return SnapNone;
    }

    const QRect avail = targetScreen->availableGeometry();
    const int snapThreshold = 16;
    QRect previewRect;
    SnapPreviewMode mode = SnapNone;

    if (globalPos.y() <= avail.top() + snapThreshold) {
        mode = SnapMaximize;
        previewRect = avail.adjusted(6, 6, -6, -6);
    } else if (globalPos.x() <= avail.left() + snapThreshold) {
        mode = SnapLeftHalf;
        previewRect = QRect(avail.left(), avail.top(), avail.width() / 2, avail.height()).adjusted(6, 6, -3, -6);
    } else if (globalPos.x() >= avail.right() - snapThreshold) {
        mode = SnapRightHalf;
        previewRect = QRect(avail.left() + avail.width() / 2, avail.top(), avail.width() - avail.width() / 2, avail.height()).adjusted(3, 6, -6, -6);
    }

    if (mode == SnapNone) {
        hideSnapPreview();
        return mode;
    }

    ensureSnapPreviewWidget();
    m_snapPreview->setGeometry(previewRect);
    if (!m_snapPreview->isVisible()) {
        m_snapPreview->show();
    }
    m_snapPreview->raise();
    return mode;
}

void Widget::hideSnapPreview()
{
    m_snapMode = SnapNone;
    if (m_snapPreview) {
        m_snapPreview->hide();
    }
}

void Widget::applySnapFromMode(SnapPreviewMode mode)
{
    if (mode == SnapNone) {
        return;
    }

    const QPoint cursorPos = QCursor::pos();
    QScreen *targetScreen = QGuiApplication::screenAt(cursorPos);
    if (!targetScreen) {
        targetScreen = screen();
    }
    if (!targetScreen) {
        return;
    }
    const QRect avail = targetScreen->availableGeometry();

    if (mode == SnapMaximize) {
        showMaximized();
        return;
    }

    showNormal();
    if (mode == SnapLeftHalf) {
        setGeometry(QRect(avail.left(), avail.top(), avail.width() / 2, avail.height()));
    } else if (mode == SnapRightHalf) {
        setGeometry(QRect(avail.left() + avail.width() / 2, avail.top(), avail.width() - avail.width() / 2, avail.height()));
    }
}

