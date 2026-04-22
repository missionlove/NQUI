#include "nsidebarnavigation.h"
#include "nsidebarnavitem.h"
#include <QLabel>
#include <QVBoxLayout>

NSidebarNavigation::NSidebarNavigation(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_layout(new QVBoxLayout(this))
{
    m_layout->setContentsMargins(12, 16, 12, 16);
    m_layout->setSpacing(4);
    m_layout->addStretch(1);
}

void NSidebarNavigation::addSectionTitle(const QString &title)
{
    QLabel *lab = new QLabel(title, this);
    lab->setStyleSheet(QStringLiteral("color: rgba(200,200,200,0.55); font-size: 11px; font-weight: 600;"));
    m_layout->insertWidget(m_layout->count() - 1, lab);
    m_layout->insertSpacing(m_layout->count() - 1, 8);
}

void NSidebarNavigation::addNavItem(NSidebarNavItem *item)
{
    if (item) {
        m_layout->insertWidget(m_layout->count() - 1, item);
    }
}
