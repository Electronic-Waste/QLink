#pragma once
#ifndef MODE_H
#define MODE_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
class Mode;
}

class Mode : public QWidget
{
    Q_OBJECT

public:
    explicit Mode(QWidget *parent = nullptr);
    ~Mode();

private:
    Ui::Mode *ui;

private slots:
    void Single();
    void Double();
};

#endif // MODE_H
