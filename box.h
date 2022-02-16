#pragma once
#ifndef BOX_H
#define BOX_H

#include <iostream>
#include <QPainter>
#include <stdlib.h>
#include <QKeyEvent>
#include <time.h>
#include <synchapi.h>
#include <QPixmap>
using namespace std;

class Box
{
    int row,col;
    int state; //0-消除 1-健在 2-激活 -1-+1s -2-shuffle -3-hint
    static const int sizeOfBox = 40;
    QColor colorOfBox;
    QPixmap pixmap;

public:
    Box(int r, int c ,const QColor &color, int s = 1);
    ~Box(){}
    void setState(int s){state = s;}
    void setColor(QColor color){colorOfBox = color;}
    void paintBox(QPainter *painter);
    void highlight();
    int returnPosX() const {return 100 + 40 * col;}
    int returnPosY() const {return 100 + 40 * row;}
    int returnState() const {return state;}
    QColor returnColor() const {return colorOfBox;}

};

#endif // BOX_H
