#ifndef NDASHBOARDCONTENTLAYOUT_H
#define NDASHBOARDCONTENTLAYOUT_H

#include "ndashboardwidgetbase.h"

class QVBoxLayout;

class NDASHBOARD_EXPORT NDashboardContentLayout : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NDashboardContentLayout(QWidget *parent = nullptr);

    void addWidget(QWidget *w, int stretch = 0);
    void addSpacing(int px);
    void addStretch(int stretch = 1);

    void setMargins(int left, int top, int right, int bottom);
    void setColumnSpacing(int s);

private:
    QVBoxLayout *m_main;
};

#endif // NDASHBOARDCONTENTLAYOUT_H
