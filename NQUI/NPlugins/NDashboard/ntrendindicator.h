#ifndef NTRENDINDICATOR_H
#define NTRENDINDICATOR_H

#include "ndashboardwidgetbase.h"

class QLabel;

class NDASHBOARD_EXPORT NTrendIndicator : public NDashboardWidgetBase
{
    Q_OBJECT
public:
    enum Direction { Up, Down, Neutral };
    Q_ENUM(Direction)

    explicit NTrendIndicator(QWidget *parent = nullptr);

    void setDirection(Direction d);
    Direction direction() const;

    void setDeltaText(const QString &text);
    QString deltaText() const;

    void setPositiveColor(const QColor &c);
    void setNegativeColor(const QColor &c);

    QColor positiveColor() const;
    QColor negativeColor() const;

protected:
    void applyChromeStyle() override;

private:
    void refreshGlyph();

    QLabel *m_arrow;
    QLabel *m_value;
    Direction m_dir;
    QColor m_pos;
    QColor m_neg;
};

#endif // NTRENDINDICATOR_H
