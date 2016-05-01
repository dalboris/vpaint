#ifndef RUNBUTTON_H
#define RUNBUTTON_H

#include <QPushButton>

class TestItem;

class RunButton: public QPushButton
{
    Q_OBJECT

public:
    RunButton(TestItem * item, QWidget * parent = nullptr);

signals:
    void runClicked(TestItem * item);

private slots:
    void onClicked_();

private:
    TestItem * item_;
};

#endif // RUNBUTTON_H
