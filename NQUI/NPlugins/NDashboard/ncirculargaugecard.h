#ifndef NCIRCULARGAUGECARD_H
#define NCIRCULARGAUGECARD_H

#include "ndashboardwidgetbase.h"

class NDASHBOARD_EXPORT NCircularGaugeCard : public NDashboardWidgetBase
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)

public:
    explicit NCircularGaugeCard(QWidget *parent = nullptr);

    double value() const;
    void setValue(double v);

    double minimum() const;
    void setMinimum(double m);

    double maximum() const;
    void setMaximum(double m);

    void setGaugeColor(const QColor &c);
    QColor gaugeColor() const;

    void setTrackColor(const QColor &c);
    QColor trackColor() const;

    void setThickness(int px);
    int thickness() const;

    void setCaption(const QString &text);

signals:
    void valueChanged(double v);

protected:
    void paintEvent(QPaintEvent *event) override;
    void applyChromeStyle() override;

private:
    double m_value;
    double m_min;
    double m_max;
    QColor m_gauge;
    QColor m_track;
    int m_thickness;
    QString m_caption;
};

#endif // NCIRCULARGAUGECARD_H
