#ifndef DASHBOARDDEMOWIDGET_H
#define DASHBOARDDEMOWIDGET_H

#include "nframelesswidget.h"

class QEvent;
class QFrame;
class QGraphicsDropShadowEffect;
class QPushButton;
class QVBoxLayout;

class DashboardDemoWidget : public NFramelessWidget
{
    Q_OBJECT
public:
    explicit DashboardDemoWidget(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private:
    void refreshMaximizeButton();
    void refreshWindowVisualState();

    QFrame *m_shadowHost;
    QFrame *m_chromeFrame;
    QVBoxLayout *m_rootLayout;
    QVBoxLayout *m_shadowLayout;
    QVBoxLayout *m_chromeLayout;
    QGraphicsDropShadowEffect *m_shadowEffect;
    QPushButton *m_maxButton;
};

#endif // DASHBOARDDEMOWIDGET_H
