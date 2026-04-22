#ifndef NUSERPROFILECOMPACT_H
#define NUSERPROFILECOMPACT_H

#include "ndashboardwidgetbase.h"

#include <QPixmap>

class QLabel;

class NDASHBOARD_EXPORT NUserProfileCompact : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NUserProfileCompact(QWidget *parent = nullptr);

    void setDisplayName(const QString &name);
    QString displayName() const;

    void setSubtitle(const QString &subtitle);
    QString subtitle() const;

    void setAvatarLetter(QChar letter);
    void setAvatarPixmap(const QPixmap &pixmap);
    QPixmap avatarPixmap() const;
    void clearAvatarPixmap();

    void setAvatarSize(int size);
    int avatarSize() const;

protected:
    void applyChromeStyle() override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void refreshAvatar();

    QLabel *m_avatar;
    QLabel *m_name;
    QLabel *m_sub;
    QChar m_letter;
    QPixmap m_avatarPixmap;
    int m_avatarSize;
};

#endif // NUSERPROFILECOMPACT_H
