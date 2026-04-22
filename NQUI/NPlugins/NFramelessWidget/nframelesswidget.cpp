#include "nframelesswidget.h"
#include <QApplication>
#include <QChildEvent>
#include <QColor>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QRegion>
#include <QSize>
#include <QTimer>
#include <QCursor>
#include <QWidget>
#include <QWindow>
#include <QtGlobal>

namespace {
QRegion buildRoundedRegion(int w, int h, int radius)
{
    if (w <= 0 || h <= 0 || radius <= 0) {
        return QRegion();
    }

    const int scale = 4;
    const int sw = w * scale;
    const int sh = h * scale;
    const int sr = radius * scale;

    QImage hi(sw, sh, QImage::Format_ARGB32_Premultiplied);
    hi.fill(Qt::transparent);
    {
        QPainter p(&hi);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawRoundedRect(QRectF(0, 0, sw, sh), sr, sr);
    }

    const QImage lo = hi.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QRegion region;
    for (int y = 0; y < h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(lo.constScanLine(y));
        int x = 0;
        while (x < w) {
            while (x < w && qAlpha(line[x]) < 128) {
                ++x;
            }
            const int start = x;
            while (x < w && qAlpha(line[x]) >= 128) {
                ++x;
            }
            if (start < x) {
                region = region.united(QRegion(start, y, x - start, 1));
            }
        }
    }
    return region;
}
} // namespace

NFramelessWidget::NFramelessWidget(QWidget *parent)
    : QWidget(parent),
      m_resizable(true),
      m_movable(true),
      m_leftPressed(false),
      m_resizeBorderWidth(14),
      m_resizeSensitivity(6),
      m_moveAreaHeight(40),
      m_shadowStyle(
#ifdef Q_OS_WIN
          ShadowWindowsSafe
#else
          ShadowPlatformNative
#endif
      ),
      m_shadowPadding(8),
      m_cornerRadius(0),
      m_shadowEffect(nullptr),
      m_pressedRegion(RegionNone),
      m_restoreOnDrag(false)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    installEventFilter(this);
    applyShadowStyle();
}

void NFramelessWidget::setResizable(bool enabled)
{
    m_resizable = enabled;
}

bool NFramelessWidget::isResizable() const
{
    return m_resizable;
}

void NFramelessWidget::setMovable(bool enabled)
{
    m_movable = enabled;
}

bool NFramelessWidget::isMovable() const
{
    return m_movable;
}

void NFramelessWidget::setResizeBorderWidth(int width)
{
    m_resizeBorderWidth = qMax(2, width);
}

int NFramelessWidget::resizeBorderWidth() const
{
    return m_resizeBorderWidth;
}

void NFramelessWidget::setResizeSensitivity(int sensitivity)
{
    m_resizeSensitivity = qMax(0, sensitivity);
}

int NFramelessWidget::resizeSensitivity() const
{
    return m_resizeSensitivity;
}

void NFramelessWidget::setMoveAreaHeight(int height)
{
    m_moveAreaHeight = height;
}

int NFramelessWidget::moveAreaHeight() const
{
    return m_moveAreaHeight;
}

void NFramelessWidget::setShadowStyle(NFramelessWidget::ShadowStyle style)
{
    if (m_shadowStyle == style) {
        return;
    }
    m_shadowStyle = style;
    applyShadowStyle();
}

NFramelessWidget::ShadowStyle NFramelessWidget::shadowStyle() const
{
    return m_shadowStyle;
}

void NFramelessWidget::setShadowPadding(int padding)
{
    m_shadowPadding = qMax(0, padding);
    applyShadowStyle();
}

int NFramelessWidget::shadowPadding() const
{
    return m_shadowPadding;
}

void NFramelessWidget::setCornerRadius(int radius)
{
    radius = qMax(0, radius);
    if (m_cornerRadius == radius) {
        return;
    }
    m_cornerRadius = radius;
    applyWindowMask();
}

int NFramelessWidget::cornerRadius() const
{
    return m_cornerRadius;
}

bool NFramelessWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::ChildAdded) {
        QChildEvent *childEvent = static_cast<QChildEvent *>(event);
        QWidget *childWidget = qobject_cast<QWidget *>(childEvent->child());
        if (childWidget) {
            registerChildForCursorSync(childWidget);
        }
    } else if (!m_leftPressed &&
               (event->type() == QEvent::MouseMove ||
                event->type() == QEvent::HoverMove ||
                event->type() == QEvent::Enter ||
                event->type() == QEvent::Leave)) {
        syncCursorFromGlobalPos();
    }

    return QWidget::eventFilter(watched, event);
}

void NFramelessWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        applyShadowStyle();
        applyWindowMask();
    }
    QWidget::changeEvent(event);
}

void NFramelessWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyWindowMask();
}

void NFramelessWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressedRegion = hitTest(event->pos());
        updateCursor(m_pressedRegion);

        if (tryStartSystemAction(m_pressedRegion)) {
            m_leftPressed = false;
            m_pressedRegion = RegionNone;
            event->accept();
            return;
        }

        m_leftPressed = (m_pressedRegion != RegionNone);
        m_pressGlobalPos = event->globalPos();
        m_pressPosLocal = event->pos();
        m_pressGeometry = geometry();
        m_restoreOnDrag = (m_pressedRegion == RegionMove && isMaximized());
    }

    QWidget::mousePressEvent(event);
}

void NFramelessWidget::mouseMoveEvent(QMouseEvent *event)
{
    const HitRegion currentRegion = hitTest(event->pos());

    if (!m_leftPressed || !(event->buttons() & Qt::LeftButton)) {
        updateCursor(currentRegion);
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QPoint delta = event->globalPos() - m_pressGlobalPos;

    if (m_pressedRegion == RegionMove && m_movable) {
        if (m_restoreOnDrag && isMaximized() &&
            delta.manhattanLength() >= QApplication::startDragDistance()) {
            QRect restoreGeometry = normalGeometry();
            if (!restoreGeometry.isValid()) {
                const int fallbackW = qMax(minimumWidth(), 960);
                const int fallbackH = qMax(minimumHeight(), 640);
                restoreGeometry = QRect(0, 0, fallbackW, fallbackH);
            }

            const qreal cursorRatioX = qBound(
                0.0,
                static_cast<qreal>(m_pressPosLocal.x()) / qMax(1, width()),
                1.0
            );

            showNormal();
            const int anchorY = qMin(m_pressPosLocal.y(), qMax(28, m_moveAreaHeight / 2));
            const int nextX = event->globalX() - static_cast<int>(restoreGeometry.width() * cursorRatioX);
            const int nextY = event->globalY() - anchorY;
            move(nextX, nextY);

            m_pressGeometry = geometry();
            m_pressGlobalPos = event->globalPos();
            m_restoreOnDrag = false;
        }

        if (isMaximized()) {
            onMoveDragUpdated(event->globalPos());
            QWidget::mouseMoveEvent(event);
            return;
        }
        move(m_pressGeometry.topLeft() + delta);
        onMoveDragUpdated(event->globalPos());
    } else if (m_resizable) {
        const QRect nextGeometry = calcResizeGeometry(m_pressGeometry, delta, m_pressedRegion);
        if (nextGeometry.isValid()) {
            setGeometry(nextGeometry);
        }
    }

    QWidget::mouseMoveEvent(event);
}

void NFramelessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    const HitRegion releasedRegion = m_pressedRegion;
    m_leftPressed = false;
    m_restoreOnDrag = false;
    m_pressedRegion = RegionNone;
    const HitRegion currentRegion = hitTest(event->pos());

    // End of resize should recover cursor state immediately.
    if (releasedRegion != RegionNone && releasedRegion != RegionMove &&
        (currentRegion == RegionNone || currentRegion == RegionMove)) {
        setCursor(Qt::ArrowCursor);
    } else {
        updateCursor(currentRegion);
    }
    QWidget::mouseReleaseEvent(event);

    // Fallback refresh on next frame for high-DPI/multi-monitor edge cases.
    QTimer::singleShot(0, this, [this]() {
        updateCursor(hitTest(mapFromGlobal(QCursor::pos())));
    });

    if (releasedRegion == RegionMove) {
        onMoveDragFinished(event->globalPos());
    }
}

void NFramelessWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && hitTest(event->pos()) == RegionMove) {
        if (!isFullScreen()) {
            if (isMaximized()) {
                showNormal();
            } else {
                showMaximized();
            }
            event->accept();
            return;
        }
    }

    QWidget::mouseDoubleClickEvent(event);
}

void NFramelessWidget::leaveEvent(QEvent *event)
{
    if (!m_leftPressed) {
        setCursor(Qt::ArrowCursor);
    }
    QWidget::leaveEvent(event);
}

NFramelessWidget::HitRegion NFramelessWidget::hitTest(const QPoint &pos) const
{
    const QRect rect = this->rect();
    const int x = pos.x();
    const int y = pos.y();
    const int bw = qMax(2, m_resizeBorderWidth);
    const int sensitivity = qMax(0, m_resizeSensitivity);
    const int detectBw = bw + sensitivity;
    const int cornerBw = detectBw + sensitivity;
    const QRect innerRect = rect.adjusted(detectBw, detectBw, -detectBw, -detectBw);

    const bool nearLeft = x <= rect.left() + detectBw;
    const bool nearRight = x >= rect.right() - detectBw;
    const bool nearTop = y <= rect.top() + detectBw;
    const bool nearBottom = y >= rect.bottom() - detectBw;

    const bool nearCornerLeft = x <= rect.left() + cornerBw;
    const bool nearCornerRight = x >= rect.right() - cornerBw;
    const bool nearCornerTop = y <= rect.top() + cornerBw;
    const bool nearCornerBottom = y >= rect.bottom() - cornerBw;

    const bool canResizeNow = m_resizable && !isMaximized() && !isFullScreen();
    if (canResizeNow && !innerRect.contains(pos)) {
        if (nearCornerTop && nearCornerLeft) {
            return RegionTopLeft;
        }
        if (nearCornerTop && nearCornerRight) {
            return RegionTopRight;
        }
        if (nearCornerBottom && nearCornerLeft) {
            return RegionBottomLeft;
        }
        if (nearCornerBottom && nearCornerRight) {
            return RegionBottomRight;
        }
        if (nearLeft) {
            return RegionLeft;
        }
        if (nearRight) {
            return RegionRight;
        }
        if (nearTop) {
            return RegionTop;
        }
        if (nearBottom) {
            return RegionBottom;
        }
    }

    const bool canMoveFromWholeArea = m_moveAreaHeight <= 0;
    if (m_movable && (canMoveFromWholeArea || y <= m_moveAreaHeight)) {
        return RegionMove;
    }

    return RegionNone;
}

void NFramelessWidget::syncCursorFromGlobalPos()
{
    updateCursor(hitTest(mapFromGlobal(QCursor::pos())));
}

void NFramelessWidget::registerChildForCursorSync(QWidget *child)
{
    if (!child) {
        return;
    }
    child->setMouseTracking(true);
    child->setAttribute(Qt::WA_Hover, true);
    child->installEventFilter(this);

    const QList<QWidget *> subWidgets = child->findChildren<QWidget *>();
    for (QWidget *sub : subWidgets) {
        sub->setMouseTracking(true);
        sub->setAttribute(Qt::WA_Hover, true);
        sub->installEventFilter(this);
    }
}

void NFramelessWidget::applyShadowStyle()
{
    const bool fillScreen = isMaximized() || isFullScreen();
    const int safePadding = (!fillScreen && m_shadowStyle == ShadowWindowsSafe) ? m_shadowPadding : 0;
    setContentsMargins(safePadding, safePadding, safePadding, safePadding);

    if (graphicsEffect()) {
        setGraphicsEffect(nullptr);
        m_shadowEffect = nullptr;
    }

#ifdef Q_OS_WIN
    setAttribute(Qt::WA_TranslucentBackground, false);
    return;
#else
    setAttribute(Qt::WA_TranslucentBackground, m_shadowStyle == ShadowPlatformNative && !fillScreen);
    if (m_shadowStyle != ShadowPlatformNative || fillScreen) {
        applyWindowMask();
        return;
    }

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(22);
    m_shadowEffect->setOffset(0, 4);
    m_shadowEffect->setColor(QColor(0, 0, 0, 72));
    setGraphicsEffect(m_shadowEffect);
#endif
    applyWindowMask();
}

void NFramelessWidget::applyWindowMask()
{
    const bool fillScreen = isMaximized() || isFullScreen();
    if (fillScreen || m_cornerRadius <= 0 || width() <= 0 || height() <= 0) {
        clearMask();
        return;
    }
    const int radius = qMin(m_cornerRadius, qMin(width(), height()) / 2);
    setMask(buildRoundedRegion(width(), height(), radius));
}

void NFramelessWidget::onMoveDragUpdated(const QPoint &globalPos)
{
    Q_UNUSED(globalPos);
}

void NFramelessWidget::onMoveDragFinished(const QPoint &globalPos)
{
    Q_UNUSED(globalPos);
}

void NFramelessWidget::updateCursor(HitRegion region)
{
    switch (region) {
    case RegionMove:
        setCursor(Qt::ArrowCursor);
        break;
    case RegionTop:
    case RegionBottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case RegionLeft:
    case RegionRight:
        setCursor(Qt::SizeHorCursor);
        break;
    case RegionTopLeft:
    case RegionBottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case RegionTopRight:
    case RegionBottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

Qt::Edges NFramelessWidget::regionToEdges(HitRegion region) const
{
    switch (region) {
    case RegionLeft:
        return Qt::LeftEdge;
    case RegionRight:
        return Qt::RightEdge;
    case RegionTop:
        return Qt::TopEdge;
    case RegionBottom:
        return Qt::BottomEdge;
    case RegionTopLeft:
        return Qt::TopEdge | Qt::LeftEdge;
    case RegionTopRight:
        return Qt::TopEdge | Qt::RightEdge;
    case RegionBottomLeft:
        return Qt::BottomEdge | Qt::LeftEdge;
    case RegionBottomRight:
        return Qt::BottomEdge | Qt::RightEdge;
    default:
        return Qt::Edges();
    }
}

bool NFramelessWidget::tryStartSystemAction(HitRegion region)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    if (!window() || !window()->isWindow()) {
        return false;
    }
    QWindow *winHandle = window()->windowHandle();
    if (!winHandle) {
        return false;
    }

    if (region == RegionMove && m_movable) {
        return winHandle->startSystemMove();
    }

    if (!m_resizable || isMaximized() || isFullScreen()) {
        return false;
    }

    const Qt::Edges edges = regionToEdges(region);
    if (edges == Qt::Edges()) {
        return false;
    }
    return winHandle->startSystemResize(edges);
#else
    Q_UNUSED(region);
    return false;
#endif
}

QRect NFramelessWidget::calcResizeGeometry(const QRect &base, const QPoint &delta, HitRegion region) const
{
    int left = base.left();
    int top = base.top();
    int right = base.right();
    int bottom = base.bottom();

    if (region == RegionLeft || region == RegionTopLeft || region == RegionBottomLeft) {
        left += delta.x();
    }
    if (region == RegionRight || region == RegionTopRight || region == RegionBottomRight) {
        right += delta.x();
    }
    if (region == RegionTop || region == RegionTopLeft || region == RegionTopRight) {
        top += delta.y();
    }
    if (region == RegionBottom || region == RegionBottomLeft || region == RegionBottomRight) {
        bottom += delta.y();
    }

    const QSize minSz = minimumSizeHint().expandedTo(minimumSize());
    if (right - left + 1 < minSz.width()) {
        if (region == RegionLeft || region == RegionTopLeft || region == RegionBottomLeft) {
            left = right - minSz.width() + 1;
        } else {
            right = left + minSz.width() - 1;
        }
    }
    if (bottom - top + 1 < minSz.height()) {
        if (region == RegionTop || region == RegionTopLeft || region == RegionTopRight) {
            top = bottom - minSz.height() + 1;
        } else {
            bottom = top + minSz.height() - 1;
        }
    }

    return QRect(QPoint(left, top), QPoint(right, bottom));
}
