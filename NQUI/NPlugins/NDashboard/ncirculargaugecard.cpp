#include "ncirculargaugecard.h"

#include <QPainter>
#include <QPaintEvent>
#include <QtMath>

NCircularGaugeCard::NCircularGaugeCard(QWidget *parent)
    : NDashboardWidgetBase(parent),
      m_value(62),
      m_min(0),
      m_max(100),
      m_gauge(QColor(106, 149, 255)),
      m_track(QColor(60, 64, 74)),
      m_thickness(10),
      m_caption(QStringLiteral("Gauge"))
{
    setMinimumSize(160, 160);
}

double NCircularGaugeCard::value() const
{
    return m_value;
}

void NCircularGaugeCard::setValue(double v)
{
    v = qBound(m_min, v, m_max);
    if (qFuzzyCompare(m_value, v)) {
        return;
    }
    m_value = v;
    update();
    emit valueChanged(m_value);
}

double NCircularGaugeCard::minimum() const
{
    return m_min;
}

void NCircularGaugeCard::setMinimum(double m)
{
    m_min = m;
    setValue(m_value);
}

double NCircularGaugeCard::maximum() const
{
    return m_max;
}

void NCircularGaugeCard::setMaximum(double m)
{
    m_max = m;
    setValue(m_value);
}

void NCircularGaugeCard::setGaugeColor(const QColor &c)
{
    m_gauge = c;
    update();
}

QColor NCircularGaugeCard::gaugeColor() const
{
    return m_gauge;
}

void NCircularGaugeCard::setTrackColor(const QColor &c)
{
    m_track = c;
    update();
}

QColor NCircularGaugeCard::trackColor() const
{
    return m_track;
}

void NCircularGaugeCard::setThickness(int px)
{
    m_thickness = qMax(2, px);
    update();
}

int NCircularGaugeCard::thickness() const
{
    return m_thickness;
}

void NCircularGaugeCard::setCaption(const QString &text)
{
    m_caption = text;
    update();
}

void NCircularGaugeCard::applyChromeStyle()
{
    NDashboardWidgetBase::applyChromeStyle();
    update();
}

void NCircularGaugeCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRect r = rect().adjusted(12, 12, -12, -28);
    const int side = qMin(r.width(), r.height());
    QRectF arcRect(r.center().x() - side / 2.0, r.center().y() - side / 2.0, side, side);

    const double span = 270.0;
    const double start = 135.0;
    const double ratio = (m_max > m_min) ? (m_value - m_min) / (m_max - m_min) : 0;
    const double valueSpan = span * qBound(0.0, ratio, 1.0);

    QPen trackPen(m_track, m_thickness, Qt::SolidLine, Qt::RoundCap);
    p.setPen(trackPen);
    p.drawArc(arcRect, int(start * 16), int(span * 16));

    QPen gaugePen(m_gauge, m_thickness, Qt::SolidLine, Qt::RoundCap);
    p.setPen(gaugePen);
    p.drawArc(arcRect, int(start * 16), int(valueSpan * 16));

    p.setPen(foregroundColor());
    QFont f = p.font();
    f.setPixelSize(22);
    f.setBold(true);
    p.setFont(f);
    p.drawText(arcRect, Qt::AlignCenter, QString::number(int(m_value)) + QLatin1Char('%'));

    f.setPixelSize(11);
    f.setBold(false);
    p.setFont(f);
    QRect capRect(0, height() - 28, width(), 24);
    p.drawText(capRect, Qt::AlignHCenter | Qt::AlignTop, m_caption);
}
