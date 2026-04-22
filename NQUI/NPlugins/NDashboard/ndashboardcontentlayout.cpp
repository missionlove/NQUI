#include "ndashboardcontentlayout.h"
#include <QVBoxLayout>

NDashboardContentLayout::NDashboardContentLayout(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_main(new QVBoxLayout(this))
{
    m_main->setContentsMargins(16, 16, 16, 16);
    m_main->setSpacing(verticalSpacing());
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this]() {
        m_main->setSpacing(verticalSpacing());
    });
}

void NDashboardContentLayout::addWidget(QWidget *w, int stretch)
{
    m_main->addWidget(w, stretch);
}

void NDashboardContentLayout::addSpacing(int px)
{
    m_main->addSpacing(px);
}

void NDashboardContentLayout::addStretch(int stretch)
{
    m_main->addStretch(stretch);
}

void NDashboardContentLayout::setMargins(int left, int top, int right, int bottom)
{
    m_main->setContentsMargins(left, top, right, bottom);
}

void NDashboardContentLayout::setColumnSpacing(int s)
{
    m_main->setSpacing(s);
}
