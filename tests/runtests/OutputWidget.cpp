#include "OutputWidget.h"

#include <QFontDatabase>

namespace
{

// Thx to Kuba Ober:
// http://stackoverflow.com/questions/18896933/qt-qfont-selection-of-a-monospace-font-doesnt-work
bool isFixedPitch(const QFont & font)
{
    const QFontInfo fi(font);
    return fi.fixedPitch();
}

QFont getMonospaceFont()
{
    QFont font("monospace");
    font.setPixelSize(12);
    if (isFixedPitch(font)) return font;
    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font)) return font;
    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font)) return font;
    font.setFamily("courier");
    if (isFixedPitch(font)) return font;
    return font;
}

}

OutputWidget::OutputWidget(QWidget * parent) :
    QTextEdit(parent)
{
    textDocument_.setDefaultFont(getMonospaceFont());
    setDocument(&textDocument_);
    setReadOnly(true);
}

void OutputWidget::setOutput(const QString & text)
{
    textDocument_.setPlainText(text);
}
