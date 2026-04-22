#include "nuserprofilecompact.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

NUserProfileCompact::NUserProfileCompact(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_avatar(new QLabel(this)),
      m_name(new QLabel(this)),
      m_sub(new QLabel(this)),
      m_letter(QLatin1Char('U')),
      m_avatarSize(26)
{
    m_avatar->setFixedSize(m_avatarSize, m_avatarSize);
    m_avatar->setAlignment(Qt::AlignCenter);
    m_name->setStyleSheet(QStringLiteral("font-weight: 700;"));
    m_sub->setStyleSheet(QStringLiteral("font-size: 11px; opacity: 0.7;"));
    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(horizontalSpacing());
    h->addWidget(m_avatar, 0, Qt::AlignVCenter);
    QVBoxLayout *v = new QVBoxLayout;
    v->setSpacing(2);
    v->setContentsMargins(0, 0, 0, 0);
    v->addWidget(m_name);
    v->addWidget(m_sub);
    h->addLayout(v);
    connect(this, &NDashboardWidgetBase::spacingChanged, this, [this, h]() {
        h->setSpacing(horizontalSpacing());
    });
    refreshAvatar();
    applyChromeStyle();
}

void NUserProfileCompact::setDisplayName(const QString &name)
{
    m_name->setText(name);
    if (name.isEmpty()) {
        m_letter = QLatin1Char('U');
    } else {
        m_letter = name.at(0).toUpper();
    }
    refreshAvatar();
}

QString NUserProfileCompact::displayName() const
{
    return m_name->text();
}

void NUserProfileCompact::setSubtitle(const QString &subtitle)
{
    m_sub->setText(subtitle);
}

QString NUserProfileCompact::subtitle() const
{
    return m_sub->text();
}

void NUserProfileCompact::setAvatarLetter(QChar letter)
{
    m_letter = letter.toUpper();
    refreshAvatar();
}

void NUserProfileCompact::setAvatarPixmap(const QPixmap &pixmap)
{
    m_avatarPixmap = pixmap;
    refreshAvatar();
}

QPixmap NUserProfileCompact::avatarPixmap() const
{
    return m_avatarPixmap;
}

void NUserProfileCompact::clearAvatarPixmap()
{
    if (m_avatarPixmap.isNull()) {
        return;
    }
    m_avatarPixmap = QPixmap();
    refreshAvatar();
}

void NUserProfileCompact::setAvatarSize(int size)
{
    size = qMax(16, size);
    if (m_avatarSize == size) {
        return;
    }
    m_avatarSize = size;
    refreshAvatar();
}

int NUserProfileCompact::avatarSize() const
{
    return m_avatarSize;
}

void NUserProfileCompact::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    const QString fg = foregroundColor().name(QColor::HexArgb);
    const QString ac = accentColor().name(QColor::HexArgb);
    m_name->setStyleSheet(QStringLiteral("font-weight: 700; color: %1;").arg(fg));
    m_sub->setStyleSheet(QStringLiteral("font-size: 11px; color: %1;").arg(fg));
    refreshAvatar();
}

void NUserProfileCompact::resizeEvent(QResizeEvent *event)
{
    NDashboardWidgetBase::resizeEvent(event);
    refreshAvatar();
}

void NUserProfileCompact::refreshAvatar()
{
    m_avatar->setFixedSize(m_avatarSize, m_avatarSize);
    const int radius = m_avatarSize / 2;
    if (!m_avatarPixmap.isNull()) {
        const QPixmap scaled = m_avatarPixmap.scaled(m_avatarSize,
                                                     m_avatarSize,
                                                     Qt::KeepAspectRatioByExpanding,
                                                     Qt::SmoothTransformation);
        QPixmap rounded(m_avatarSize, m_avatarSize);
        rounded.fill(Qt::transparent);
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPainterPath path;
        path.addEllipse(0, 0, m_avatarSize, m_avatarSize);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, scaled);
        m_avatar->setPixmap(rounded);
        m_avatar->setText(QString());
        m_avatar->setStyleSheet(QStringLiteral("QLabel { background: transparent; border-radius: %1px; }").arg(radius));
        return;
    }

    const QString letter = m_letter.isNull() ? QStringLiteral("?") : QString(m_letter);
    m_avatar->setPixmap(QPixmap());
    m_avatar->setText(letter);
    const QString ac = accentColor().name(QColor::HexArgb);
    const QString fg = foregroundColor().name(QColor::HexArgb);
    const int fontPx = qMax(10, m_avatarSize / 2);
    m_avatar->setStyleSheet(QStringLiteral(
                                "QLabel { background-color: %1; color: %2; border-radius: %3px; font-weight: 800; font-size: %4px; }")
                                .arg(ac, fg)
                                .arg(radius)
                                .arg(fontPx));
}
