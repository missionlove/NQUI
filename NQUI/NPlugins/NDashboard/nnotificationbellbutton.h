#ifndef NNOTIFICATIONBELLBUTTON_H
#define NNOTIFICATIONBELLBUTTON_H

#include "ndashboardpushbuttonbase.h"

class NDASHBOARD_EXPORT NNotificationBellButton : public NDashboardPushButtonBase
{
    Q_OBJECT
public:
    explicit NNotificationBellButton(QWidget *parent = nullptr);

    void setBadgeCount(int count);
    int badgeCount() const;

    void setBellGlyph(const QString &glyph);

protected:
    void applyChromeStyle() override;
    void paintEvent(QPaintEvent *event) override;

private:
    int m_badgeCount;
};

#endif // NNOTIFICATIONBELLBUTTON_H
