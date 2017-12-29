#include "lineedit.h"

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent), multiple(false)
{

}

LineEdit::~LineEdit()
{

}

void LineEdit::setMultipleValues()
{
    bool b = blockSignals(true);

    multiple = true;
    QLineEdit::setText("<< multiple values >>");

    blockSignals(b);
}

#include <QStyle>

void LineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);

    style()->unpolish(this);
    style()->polish(this);

    if (multiple)
    {
        bool b = blockSignals(true);

        clear();

        blockSignals(b);
    }

    update();
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);

    style()->unpolish(this);
    style()->polish(this);

    update();
}

void LineEdit::setText(const QString &text, bool spontaneous)
{
    bool b;

    if (!spontaneous)
        b = blockSignals(true);

    multiple = false;
    QLineEdit::setText(text);

    if (!spontaneous)
        blockSignals(b);
}
