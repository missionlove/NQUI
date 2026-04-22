#ifndef NDASHBOARDPUSHBUTTONBASE_H
#define NDASHBOARDPUSHBUTTONBASE_H

#include "NDashboard_global.h"
#include <QColor>
#include <QPushButton>

class NDASHBOARD_EXPORT NDashboardPushButtonBase : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(int cornerRadius READ cornerRadius WRITE setCornerRadius NOTIFY cornerRadiusChanged)

public:
    explicit NDashboardPushButtonBase(QWidget *parent = nullptr);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &c);

    QColor foregroundColor() const;
    void setForegroundColor(const QColor &c);

    QColor accentColor() const;
    void setAccentColor(const QColor &c);

    int cornerRadius() const;
    void setCornerRadius(int r);

signals:
    void backgroundColorChanged(const QColor &c);
    void foregroundColorChanged(const QColor &c);
    void accentColorChanged(const QColor &c);
    void cornerRadiusChanged(int r);

protected:
    virtual void applyChromeStyle();
    QString chromeStyleSheet() const;

private:
    QColor m_backgroundColor;
    QColor m_foregroundColor;
    QColor m_accentColor;
    int m_cornerRadius;
};

#endif // NDASHBOARDPUSHBUTTONBASE_H
