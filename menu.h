#pragma once
#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <qlink.h>
#include <mode.h>
#include <QPushButton>

namespace Ui {
class Menu;
}

class Menu : public QWidget
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu();

private:
    Ui::Menu *ui;

private slots:
    void startNewGame();
    void loadGame();
    void quitGame();
};

#endif // MENU_H
