#ifndef NPAGETITLEWITHFILTER_H
#define NPAGETITLEWITHFILTER_H

#include "ndashboardwidgetbase.h"

#include <QVariant>

class QComboBox;
class QLabel;

class NDASHBOARD_EXPORT NPageTitleWithFilter : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    explicit NPageTitleWithFilter(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    QString title() const;

    void clearFilterItems();
    void addFilterItem(const QString &text, const QVariant &userData = QVariant());

    int currentFilterIndex() const;
    void setCurrentFilterIndex(int index);

signals:
    void filterIndexChanged(int index);

protected:
    void applyChromeStyle() override;

private:
    QLabel *m_title;
    QComboBox *m_combo;
};

#endif // NPAGETITLEWITHFILTER_H
