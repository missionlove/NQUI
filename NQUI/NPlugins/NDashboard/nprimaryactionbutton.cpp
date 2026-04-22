#include "nprimaryactionbutton.h"
#include <QMetaObject>

NPrimaryActionButton::NPrimaryActionButton(const QString &text, QWidget *parent)
    : NDashboardPushButtonBase(parent)
{
    setText(text);
    setForegroundColor(QColor(250, 250, 250));
}

void NPrimaryActionButton::applyChromeStyle()
{
    const QString cls = QString::fromLatin1(metaObject()->className());
    const QString fill = accentColor().name(QColor::HexArgb);
    const QString fg = foregroundColor().name(QColor::HexArgb);
    const QString hover = accentColor().lighter(115).name(QColor::HexArgb);
    const QString pressed = accentColor().darker(115).name(QColor::HexArgb);
    setStyleSheet(QStringLiteral(
                      "%1 { background-color: %2; color: %3; border-radius: %4px; border: none; padding: 8px 18px; font-weight: 600; }"
                      "%1:hover { background-color: %5; }"
                      "%1:pressed { background-color: %6; }"
                      "%1:disabled { background-color: rgba(255,255,255,0.12); color: rgba(255,255,255,0.35); }")
                      .arg(cls, fill, fg)
                      .arg(cornerRadius())
                      .arg(hover, pressed));
}
