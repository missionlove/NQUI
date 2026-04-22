#ifndef NAPPSHELL_H
#define NAPPSHELL_H

#include "NDashboard_global.h"
#include <QColor>
#include <QWidget>

class QHBoxLayout;

class NDASHBOARD_EXPORT NAppShell : public QWidget
{
    Q_OBJECT
public:
    explicit NAppShell(QWidget *parent = nullptr);

    void setSidebarWidget(QWidget *w);
    void setContentWidget(QWidget *w);

    void setSidebarWidth(int width);
    int sidebarWidth() const;

    void setSidebarBackgroundColor(const QColor &c);
    QColor sidebarBackgroundColor() const;

    void setContentBackgroundColor(const QColor &c);
    QColor contentBackgroundColor() const;

    void setAccentColor(const QColor &c);
    QColor accentColor() const;

    void setCornerRadius(int r);
    int cornerRadius() const;

private:
    void refreshStyle();

    QWidget *m_sidebarHost;
    QWidget *m_contentHost;
    QHBoxLayout *m_layout;
    int m_sidebarWidth;
    QColor m_sidebarBg;
    QColor m_contentBg;
    QColor m_accent;
    int m_cornerRadius;
};

#endif // NAPPSHELL_H
