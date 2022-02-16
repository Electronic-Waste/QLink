#include "player.h"
#include "box.h"

#include <iostream>
#include <QObject>
#include <QPainter>
#include <stdlib.h>
#include <QKeyEvent>
#include <time.h>
#include <synchapi.h>
using namespace std;

Player::Player(int px,int py,const QColor &color)
    :x(px),y(py),score(0),colorOfPlayer(color)
{
    arr.reserve(2);  //设置arr的容量为2
}

//
//paintPlayer接受绘画工具(painter)为参数，绘制Player
//
void Player::paintPlayer(QPainter *painter)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(colorOfPlayer);
    painter->drawEllipse(x, y, sizeOfPlayer, sizeOfPlayer);
}

//
//addInArr接受行号(r)和列号(c)为参数，将相应的box信息存入arr中
//
void Player::addInArr(int r, int c)
{
    //arr为空，存入(r,c)
    if (arr.size() == 0)
        arr.push_back(Pos(r, c));

    //若(r,c)与arr数组内元素相同，则不存入
    else if (arr[0].row == r && arr[0].col == c)
        return;
    //若(r,c)与arr数组内元素比相同，则存入
    else if (arr.size() == 1)
        arr.push_back(Pos(r, c));
}

//
//返回arr中第i个元素的地址
//
Pos* Player::arrShow(int i)
{
    return &arr[i];
}

//
//清除arr中所有元素
//
void Player::arrClear()
{
    arr.clear();
}

//
//返回arr数组中此时的元素个数
//
int Player::arrSize()
{
    return arr.size();
}
