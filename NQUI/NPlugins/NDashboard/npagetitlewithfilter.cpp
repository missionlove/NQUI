#include "npagetitlewithfilter.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

NPageTitleWithFilter::NPageTitleWithFilter(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_title(new QLabel(this)),
      m_combo(new QComboBox(this))
{
    m_title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 700;"));
    m_combo->setMinimumWidth(160);
    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(horizontalSpacing());
    h->addWidget(m_title, 0, Qt::AlignVCenter);
    h->addStretch(1);
    h->addWidget(m_combo, 0, Qt::AlignVCenter);
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this, h]() {
        h->setSpacing(horizontalSpacing());
    });
    connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NPageTitleWithFilter::filterIndexChanged);
    applyChromeStyle();
}

void NPageTitleWithFilter::setTitle(const QString &title)
{
    m_title->setText(title);
}

QString NPageTitleWithFilter::title() const
{
    return m_title->text();
}

void NPageTitleWithFilter::clearFilterItems()
{
    m_combo->clear();
}

void NPageTitleWithFilter::addFilterItem(const QString &text, const QVariant &userData)
{
    m_combo->addItem(text, userData);
}

int NPageTitleWithFilter::currentFilterIndex() const
{
    return m_combo->currentIndex();
}

void NPageTitleWithFilter::setCurrentFilterIndex(int index)
{
    m_combo->setCurrentIndex(index);
}

void NPageTitleWithFilter::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    const QString fg = foregroundColor().name(QColor::HexArgb);
    const QString ac = accentColor().name(QColor::HexArgb);
    m_title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 700; color: %1;").arg(fg));
    m_combo->setStyleSheet(QStringLiteral(
        "QComboBox { color: %1; background: rgba(255,255,255,0.06); border: 1px solid %2; border-radius: 6px; padding: 4px 8px; }"
        "QComboBox::drop-down { border: none; width: 22px; }")
                               .arg(fg, ac));
}
