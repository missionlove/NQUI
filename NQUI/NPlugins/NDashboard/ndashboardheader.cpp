#include "ndashboardheader.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

NDashboardHeader::NDashboardHeader(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_leftHost(new QWidget(this)),
      m_centerHost(new QWidget(this)),
      m_rightHost(new QWidget(this)),
      m_layout(new QHBoxLayout(this))
{
    m_layout->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
    m_layout->setSpacing(horizontalSpacing());
    m_layout->addWidget(m_leftHost, 0, Qt::AlignLeft);
    m_layout->addWidget(m_centerHost, 1, Qt::AlignCenter);
    m_layout->addWidget(m_rightHost, 0, Qt::AlignRight);
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this]() {
        m_layout->setContentsMargins(horizontalSpacing(), verticalSpacing(), horizontalSpacing(), verticalSpacing());
        m_layout->setSpacing(horizontalSpacing());
    });
}

void NDashboardHeader::replaceHost(QWidget *host, QWidget *w)
{
    if (!host) {
        return;
    }
    if (QLayout *old = host->layout()) {
        QLayoutItem *it;
        while ((it = old->takeAt(0)) != nullptr) {
            if (QWidget *cw = it->widget()) {
                cw->deleteLater();
            }
            delete it;
        }
        old->deleteLater();
    }
    QVBoxLayout *v = new QVBoxLayout(host);
    v->setContentsMargins(0, 0, 0, 0);
    if (w) {
        v->addWidget(w);
    }
}

void NDashboardHeader::setLeftWidget(QWidget *w)
{
    replaceHost(m_leftHost, w);
}

void NDashboardHeader::setCenterWidget(QWidget *w)
{
    replaceHost(m_centerHost, w);
}

void NDashboardHeader::setRightWidget(QWidget *w)
{
    replaceHost(m_rightHost, w);
}
