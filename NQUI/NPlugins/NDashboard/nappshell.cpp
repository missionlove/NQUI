#include "nappshell.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtGlobal>

NAppShell::NAppShell(QWidget *parent)
    : QWidget(parent),
      m_sidebarHost(nullptr),
      m_contentHost(nullptr),
      m_layout(nullptr),
      m_sidebarWidth(240),
      m_sidebarBg(QColor(30, 34, 42)),
      m_contentBg(QColor(24, 27, 33)),
      m_accent(QColor(106, 149, 255)),
      m_cornerRadius(0)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_sidebarHost = new QWidget(this);
    m_sidebarHost->setFixedWidth(m_sidebarWidth);

    m_contentHost = new QWidget(this);
    m_layout->addWidget(m_sidebarHost);
    m_layout->addWidget(m_contentHost, 1);
    refreshStyle();
}

void NAppShell::setSidebarWidget(QWidget *w)
{
    if (!m_sidebarHost) {
        return;
    }
    if (QLayout *old = m_sidebarHost->layout()) {
        QLayoutItem *it;
        while ((it = old->takeAt(0)) != nullptr) {
            if (QWidget *cw = it->widget()) {
                cw->deleteLater();
            }
            delete it;
        }
        old->deleteLater();
    }
    QVBoxLayout *v = new QVBoxLayout(m_sidebarHost);
    v->setContentsMargins(0, 0, 0, 0);
    if (w) {
        v->addWidget(w);
    }
    refreshStyle();
}

void NAppShell::setContentWidget(QWidget *w)
{
    if (!m_contentHost) {
        return;
    }
    if (QLayout *old = m_contentHost->layout()) {
        QLayoutItem *it;
        while ((it = old->takeAt(0)) != nullptr) {
            if (QWidget *cw = it->widget()) {
                cw->deleteLater();
            }
            delete it;
        }
        old->deleteLater();
    }
    QVBoxLayout *v = new QVBoxLayout(m_contentHost);
    v->setContentsMargins(0, 0, 0, 0);
    if (w) {
        v->addWidget(w);
    }
    refreshStyle();
}

void NAppShell::setSidebarWidth(int width)
{
    width = qMax(48, width);
    m_sidebarWidth = width;
    if (m_sidebarHost) {
        m_sidebarHost->setFixedWidth(width);
    }
}

int NAppShell::sidebarWidth() const
{
    return m_sidebarWidth;
}

void NAppShell::setSidebarBackgroundColor(const QColor &c)
{
    m_sidebarBg = c;
    refreshStyle();
}

QColor NAppShell::sidebarBackgroundColor() const
{
    return m_sidebarBg;
}

void NAppShell::setContentBackgroundColor(const QColor &c)
{
    m_contentBg = c;
    refreshStyle();
}

QColor NAppShell::contentBackgroundColor() const
{
    return m_contentBg;
}

void NAppShell::setAccentColor(const QColor &c)
{
    m_accent = c;
    refreshStyle();
}

QColor NAppShell::accentColor() const
{
    return m_accent;
}

void NAppShell::setCornerRadius(int r)
{
    m_cornerRadius = qMax(0, r);
    refreshStyle();
}

int NAppShell::cornerRadius() const
{
    return m_cornerRadius;
}

void NAppShell::refreshStyle()
{
    const QString sb = m_sidebarBg.name(QColor::HexArgb);
    const QString cb = m_contentBg.name(QColor::HexArgb);
    const QString ac = m_accent.name(QColor::HexArgb);
    m_sidebarHost->setObjectName(QStringLiteral("NAppShellSidebar"));
    m_sidebarHost->setStyleSheet(QStringLiteral(
        "QWidget#NAppShellSidebar { background-color: %1; border-right: 1px solid %2; border-top-right-radius: %3px; border-bottom-right-radius: %3px; }")
                                       .arg(sb, ac)
                                       .arg(m_cornerRadius));
    m_contentHost->setStyleSheet(QStringLiteral("background-color: %1;").arg(cb));
}
