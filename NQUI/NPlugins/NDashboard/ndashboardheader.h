#ifndef NDASHBOARDHEADER_H
#define NDASHBOARDHEADER_H

#include "ndashboardwidgetbase.h"

class QHBoxLayout;

class NDASHBOARD_EXPORT NDashboardHeader : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NDashboardHeader(QWidget *parent = nullptr);

    void setLeftWidget(QWidget *w);
    void setCenterWidget(QWidget *w);
    void setRightWidget(QWidget *w);

private:
    void replaceHost(QWidget *host, QWidget *w);

    QWidget *m_leftHost;
    QWidget *m_centerHost;
    QWidget *m_rightHost;
    QHBoxLayout *m_layout;
};

#endif // NDASHBOARDHEADER_H
