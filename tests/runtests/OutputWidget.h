#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QTextEdit>
#include <QTextDocument>

class OutputWidget: public QTextEdit
{
public:
    OutputWidget(QWidget * parent = nullptr);
    void setOutput(const QString & text);

private:
    QTextDocument textDocument_;
};

#endif // OUTPUTWIDGET_H
