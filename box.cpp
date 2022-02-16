#include "box.h"
#include "player.h"

#include <iostream>
#include <QPainter>
#include <stdlib.h>
#include <QKeyEvent>
#include <time.h>
#include <synchapi.h>
#include <QPixmap>
using namespace std;

//
//Box为构造函数，接收行(r)列(c)颜色(color)和状态(s)为参数，初始化各个变量
//
Box::Box(int r, int c ,const QColor &color, int s) :
    row(r), col(c), colorOfBox(color), state(s)
{
    if (colorOfBox == Qt::red) pixmap.load("Pictures/mao.jfif");
    if (colorOfBox == Qt::green) pixmap.load("Pictures/deng.jfif");
    if (colorOfBox == Qt::blue) pixmap.load("Pictures/jiang.jfif");
    if (colorOfBox == Qt::yellow) pixmap.load("Pictures/hu.jfif");
    if (colorOfBox == Qt::gray) pixmap.load("Pictures/xi.jfif");
}

//
//paintBox接受绘画工具(painter)为参数，绘制Box
//
void Box::paintBox(QPainter *painter)
{
    //绘制健在的和激活的方块
    if (state >= 1) {
        //painter->setPen(Qt::NoPen);
        //painter->setBrush(colorOfBox);
        //painter->drawRect(100 + 40 * col, 100 + 40 * row, sizeOfBox, sizeOfBox);
        painter->drawPixmap(100 + 40 * col, 100 + 40 * row, sizeOfBox, sizeOfBox, pixmap);
    }
    //绘制激活的方块和道具的边框
    if (state == 2 || state < 0) {
       painter->setPen(QPen(Qt::black, 4));
       painter->setBrush(Qt::NoBrush);
       int PosX = 100 + 40 * col, PosY = 100 + 40 * row;
       painter->drawLine(PosX + 2, PosY + 2, PosX + sizeOfBox - 2, PosY + 2);
       painter->drawLine(PosX + sizeOfBox - 2, PosY + 2, PosX + sizeOfBox - 2, PosY + sizeOfBox - 2);
       painter->drawLine(PosX + sizeOfBox - 2, PosY + sizeOfBox - 2, PosX + 2, PosY + sizeOfBox - 2);
       painter->drawLine(PosX + 2, PosY + sizeOfBox - 2, PosX + 2, PosY + 2);
    }
    //绘制道具图标
    painter->setPen(QPen(Qt::black, 4));
    painter->setBrush(Qt::NoBrush);
    if (state == -1)
        painter->drawText(105 + 40 * col, 125 + 40 * row, "+1s");
    if (state == -2)
        painter->drawText(105 + 40 * col, 125 + 40 * row, "Shuf");
    if (state == -3)
        painter->drawText(105 + 40 * col, 125 + 40 * row, "Hint");

}
