#ifndef NSIDEBARNAVITEM_H
#define NSIDEBARNAVITEM_H

#include "ndashboardpushbuttonbase.h"

class NDASHBOARD_EXPORT NSidebarNavItem : public NDashboardPushButtonBase
{
    Q_OBJECT
public:
    explicit NSidebarNavItem(const QString &text, QWidget *parent = nullptr);

    void setBadgeCount(int count);
    int badgeCount() const;

protected:
    void applyChromeStyle() override;
    void paintEvent(QPaintEvent *event) override;

private:
    int m_badgeCount;
};

#endif // NSIDEBARNAVITEM_H
