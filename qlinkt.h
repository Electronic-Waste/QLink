#pragma once
#ifndef QLINKT_H
#define QLINKT_H

#include <QWidget>
#include "pause.h"
#include "player.h"
#include "box.h"
#include "menu.h"

namespace Ui {
class QLinkT;
}

class QLinkT : public QWidget
{
    Q_OBJECT

public:
    explicit QLinkT(QWidget *parent = nullptr);
    QLinkT(int mode, QWidget *parent = nullptr);
    ~QLinkT();
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void move_W(Player *p);
    void move_A(Player *p);
    void move_S(Player *p);
    void move_D(Player *p);
    void copeTwoBox(Player *p, QPainter *painter);
    void showTime(QPainter *painter);
    void showScore(QPainter *painter);
    void showCanContinue();
    bool canContinue();
    bool isDeleted(int r1, int c1, int r2, int c2, QPainter *painter);
    bool isDeleted(int r1, int c1, int r2, int c2);
    bool isLinkedOnce(int r1, int c1, int r2, int c2, int &path);
    bool isLinkedOnce(int r1, int c1, int r2, int c2);
    bool isEmpty();

    void createMap();

private:
    Ui::QLinkT *ui;
    Pause *pau;
    Box *mapOfBox[M+2][N+2];
    QTimer *timer;
    Player *p1;
    Player *p2;
    int timeLimit;
    QSet<int> pressedKeys;
    QPixmap pix;

private slots:
    void pause();
    void getCloseSignal();
    void countTime();
    void acceptOpenNewMenu();
    void saveGame();
    void move();

signals:
    void stop();
    void openNewMenu();

};

#endif // QLINKT_H
