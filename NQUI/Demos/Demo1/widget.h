#ifndef WIDGET_H
#define WIDGET_H

#include "nframelesswidget.h"

class QEvent;
class QFrame;
class QGraphicsDropShadowEffect;
class QPushButton;
class QWidget;
class QVBoxLayout;

class Widget : public NFramelessWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void changeEvent(QEvent *event) override;
    void onMoveDragUpdated(const QPoint &globalPos) override;
    void onMoveDragFinished(const QPoint &globalPos) override;

private:
    enum SnapPreviewMode {
        SnapNone,
        SnapMaximize,
        SnapLeftHalf,
        SnapRightHalf
    };

    void refreshMaximizeButton();
    void refreshWindowVisualState();
    void ensureSnapPreviewWidget();
    SnapPreviewMode updateSnapPreview(const QPoint &globalPos);
    void hideSnapPreview();
    void applySnapFromMode(SnapPreviewMode mode);

private:
    QFrame *m_shadowHost;
    QFrame *m_chromeFrame;
    QVBoxLayout *m_rootLayout;
    QVBoxLayout *m_shadowLayout;
    QVBoxLayout *m_chromeLayout;
    QGraphicsDropShadowEffect *m_shadowEffect;
    QPushButton *m_maxButton;
    QWidget *m_snapPreview;
    SnapPreviewMode m_snapMode;
};
#endif // WIDGET_H
