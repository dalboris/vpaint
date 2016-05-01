#include "RunButton.h"
#include "TestItem.h"

namespace
{
QString runButtonStyleSheet =
        "QPushButton {border: none} "
        "QPushButton:hover:!pressed {background-color: rgba(150,150,150, 0.3)} "
        "QPushButton:hover:pressed  {background-color: rgba(150,150,150, 0.6)} ";
}

RunButton::RunButton(TestItem * item, QWidget * parent) :
    QPushButton(parent),
    item_(item)
{
    // Set style
    setMinimumSize(16, 16);
    setMaximumSize(16, 16);
    setIcon(QIcon(":/runicon.png"));
    setFlat(true);
    setStyleSheet(runButtonStyleSheet);

    // Prevent keyboard focus (so the focus is always on the TreeView
    setFocusPolicy(Qt::NoFocus);

    // Run test when clicked
    connect(this, &RunButton::clicked, this, &RunButton::onClicked_);
}

void RunButton::onClicked_()
{
    item_->run();
    emit runClicked(item_);
}
