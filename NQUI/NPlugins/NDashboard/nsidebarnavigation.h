#ifndef NSIDEBARNAVIGATION_H
#define NSIDEBARNAVIGATION_H

#include "ndashboardwidgetbase.h"

class QVBoxLayout;
class NSidebarNavItem;

class NDASHBOARD_EXPORT NSidebarNavigation : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NSidebarNavigation(QWidget *parent = nullptr);

    void addSectionTitle(const QString &title);
    void addNavItem(NSidebarNavItem *item);

private:
    QVBoxLayout *m_layout;
};

#endif // NSIDEBARNAVIGATION_H
