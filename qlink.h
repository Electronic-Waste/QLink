#pragma once
#ifndef QLINK_H
#define QLINK_H

#include <QMainWindow>
#include "pause.h"
#include "player.h"
#include "box.h"
#include "menu.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QLink; }
QT_END_NAMESPACE

class QLink : public QMainWindow
{
    Q_OBJECT

public:
    QLink(QWidget *parent = nullptr);
    QLink(int mode, QWidget *parent = nullptr);
    ~QLink();
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void move_W();
    void move_A();
    void move_S();
    void move_D();
    void copeTwoBox(QPainter *painter);
    void showTime(QPainter *painter);
    void showScore(QPainter *painter);
    void copeHint(QPainter *painter);
//    void showCanContinue();
    bool canContinue();
    bool isDeleted(int r1, int c1, int r2, int c2, QPainter *painter);
    bool isDeleted(int r1, int c1, int r2, int c2);
    bool isLinkedOnce(int r1, int c1, int r2, int c2, int &path);
    bool isLinkedOnce(int r1, int c1, int r2, int c2);
    bool isEmpty();

    void createMap();
    void createTestMap();

private:
    Ui::QLink *ui;
    Pause *pau;
    Box *mapOfBox[M+2][N+2];
    QTimer *timer;
    Player *p1;
    QVector<Pos> hintArr;
    int timeLimit;
    double hintTime;
    double toolTime;
    QSet<int> pressedKeys;
    QPixmap pix;

private slots:
    void pause();
    void getCloseSignal();
    void countTime();
    void showCanContinue();
    void detectTools();
    void createTools();
    void acceptAddTime();
    void acceptHint();
    void acceptShuffle();
    void acceptOpenNewMenu();
    void saveGame();
    void move();

signals:
    void stop();
    void addTime();
    void hint();
    void shuffle();
    void openNewMenu();

};
#endif // QLINK_H
