#ifndef NPRIMARYACTIONBUTTON_H
#define NPRIMARYACTIONBUTTON_H

#include "ndashboardpushbuttonbase.h"

class NDASHBOARD_EXPORT NPrimaryActionButton : public NDashboardPushButtonBase
{
    Q_OBJECT
public:
    explicit NPrimaryActionButton(const QString &text, QWidget *parent = nullptr);

protected:
    void applyChromeStyle() override;
};

#endif // NPRIMARYACTIONBUTTON_H
