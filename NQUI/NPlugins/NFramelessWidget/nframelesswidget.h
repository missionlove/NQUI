#ifndef NFRAMELESSWIDGET_H
#define NFRAMELESSWIDGET_H

#include "NFramelessWidget_global.h"
#include <QPoint>
#include <QRect>
#include <QWidget>

class QEvent;
class QGraphicsDropShadowEffect;
class QMouseEvent;
class QObject;
class QResizeEvent;
class QWidget;

class NFRAMELESSWIDGET_EXPORT NFramelessWidget : public QWidget
{
public:
    enum ShadowStyle {
        ShadowOff,
        // 非最大化：仅用边距留出“阴影带”，不启用分层窗口；外观请用透明/半透明样式自行绘制。
        ShadowWindowsSafe,
        // 非最大化：使用 QGraphicsDropShadowEffect（非 Windows 或按需），阴影为半透明。
        ShadowPlatformNative
    };

    explicit NFramelessWidget(QWidget *parent = nullptr);

    void setResizable(bool enabled);
    bool isResizable() const;

    void setMovable(bool enabled);
    bool isMovable() const;

    void setResizeBorderWidth(int width);
    int resizeBorderWidth() const;

    // Extra hot area (px) for resize hit-test.
    void setResizeSensitivity(int sensitivity);
    int resizeSensitivity() const;

    // <= 0 means whole widget surface can be used to move.
    void setMoveAreaHeight(int height);
    int moveAreaHeight() const;

    void setShadowStyle(ShadowStyle style);
    ShadowStyle shadowStyle() const;

    void setShadowPadding(int padding);
    int shadowPadding() const;

    void setCornerRadius(int radius);
    int cornerRadius() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    virtual void onMoveDragUpdated(const QPoint &globalPos);
    virtual void onMoveDragFinished(const QPoint &globalPos);

private:
    enum HitRegion {
        RegionNone,
        RegionMove,
        RegionLeft,
        RegionRight,
        RegionTop,
        RegionBottom,
        RegionTopLeft,
        RegionTopRight,
        RegionBottomLeft,
        RegionBottomRight
    };

    HitRegion hitTest(const QPoint &pos) const;
    void syncCursorFromGlobalPos();
    void registerChildForCursorSync(QWidget *child);
    void updateCursor(HitRegion region);
    Qt::Edges regionToEdges(HitRegion region) const;
    bool tryStartSystemAction(HitRegion region);
    void applyShadowStyle();
    void applyWindowMask();
    QRect calcResizeGeometry(const QRect &base, const QPoint &delta, HitRegion region) const;

private:
    bool m_resizable;
    bool m_movable;
    bool m_leftPressed;
    int m_resizeBorderWidth;
    int m_resizeSensitivity;
    int m_moveAreaHeight;
    ShadowStyle m_shadowStyle;
    int m_shadowPadding;
    int m_cornerRadius;
    QGraphicsDropShadowEffect *m_shadowEffect;

    HitRegion m_pressedRegion;
    QPoint m_pressGlobalPos;
    QPoint m_pressPosLocal;
    QRect m_pressGeometry;
    bool m_restoreOnDrag;

};

#endif // NFRAMELESSWIDGET_H
