#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include <QPainter>
#include <stdlib.h>
#include <QKeyEvent>
#include <time.h>
#include <synchapi.h>
using namespace std;

const int M = 10;
const int N = 10;

struct Pos
{
    int row;
    int col;
    Pos(int r,int c):row(r),col(c){}
    ~Pos(){}
};

class Player
{
    int x,y;
    int score;
    static const int sizeOfPlayer=20;
    const QColor colorOfPlayer;
    QVector<Pos> arr;

public:
    Player(int px,int py,const QColor &color);
    ~Player(){}
    void paintPlayer(QPainter *painter);
    void Press_W(){y = (800 + y - 5) % 800;}
    void Press_A(){x = (800 + x - 5) % 800;}
    void Press_S(){y = (800 + y + 5) % 800;}
    void Press_D(){x = (800 + x + 5) % 800;}
    void addInArr(int r, int c);
    void arrClear();
    int arrSize();
    Pos* arrShow(int i);
    int getX() const {return x;}
    int getY() const {return y;}
    void setX(int px){x = px;}
    void setY(int py){y = py;}
    void setScore(int s){score = s;}
    int getScore() const {return score;}
    void addScore(){score += 2;}
};

#endif // PLAYER_H
