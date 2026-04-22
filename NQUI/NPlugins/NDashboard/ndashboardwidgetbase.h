#ifndef NDASHBOARDWIDGETBASE_H
#define NDASHBOARDWIDGETBASE_H

#include "NDashboard_global.h"
#include <QColor>
#include <QWidget>

class NDASHBOARD_EXPORT NDashboardWidgetBase : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(int cornerRadius READ cornerRadius WRITE setCornerRadius NOTIFY cornerRadiusChanged)
    Q_PROPERTY(int horizontalSpacing READ horizontalSpacing WRITE setHorizontalSpacing NOTIFY spacingChanged)
    Q_PROPERTY(int verticalSpacing READ verticalSpacing WRITE setVerticalSpacing NOTIFY spacingChanged)

public:
    explicit NDashboardWidgetBase(QWidget *parent = nullptr);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &c);

    QColor foregroundColor() const;
    void setForegroundColor(const QColor &c);

    QColor accentColor() const;
    void setAccentColor(const QColor &c);

    int cornerRadius() const;
    void setCornerRadius(int r);

    int horizontalSpacing() const;
    void setHorizontalSpacing(int s);

    int verticalSpacing() const;
    void setVerticalSpacing(int s);

signals:
    void backgroundColorChanged(const QColor &c);
    void foregroundColorChanged(const QColor &c);
    void accentColorChanged(const QColor &c);
    void cornerRadiusChanged(int r);
    void spacingChanged();

protected:
    virtual void applyChromeStyle();
    QString surfaceStyleSheet() const;

private:
    QColor m_backgroundColor;
    QColor m_foregroundColor;
    QColor m_accentColor;
    int m_cornerRadius;
    int m_hSpacing;
    int m_vSpacing;
};

#endif // NDASHBOARDWIDGETBASE_H
