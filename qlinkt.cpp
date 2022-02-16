#include "qlinkt.h"
#include "ui_qlinkt.h"
#include "pause.h"
#include "player.h"
#include "menu.h"
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QKeyEvent>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <QMessageBox>
#include <fstream>
#include <iostream>
using namespace std;

//
//创建新游戏时的构造函数
//
QLinkT::QLinkT(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QLinkT)
{
    ui->setupUi(this);
    pau = new Pause();

    //打开pause界面
    connect(this, SIGNAL(stop()), this, SLOT(pause()));
    //关闭pause界面并保存游戏
    connect(pau, SIGNAL(sendCloseSignal()), this, SLOT(getCloseSignal()));
    connect(pau, SIGNAL(sendCloseSignal()), this, SLOT(saveGame()));
    //关闭pause界面但不保存游戏
    connect(pau, SIGNAL(notSave()), this, SLOT(getCloseSignal()));
    //打开menu界面
    connect(this, SIGNAL(openNewMenu()), this, SLOT(acceptOpenNewMenu()));
    //定时更新时间
    timer = new QTimer(this);
    timeLimit = 180;
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QLink::update));
    connect(timer, SIGNAL(timeout()), this, SLOT(countTime()));
    timer->start(25);

    //随机生成地图mapOfBox,边缘空出
    createMap();

    //生成Player
    p1 = new Player(40, 40, Qt::black);
    p2 = new Player(40, 80, Qt::darkMagenta);
    //玩家行动与停止(pause)继续(continue)操作
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));

    //设置窗口
    pix.load("D:/SEP_Program/QLink_2/china.jpg");
    setWindowTitle("QLinkT");
    resize(800, 800);
}

//
//读取存档时的构造函数
//
QLinkT::QLinkT(int mode, QWidget *parent)
    : QWidget(parent)
    ,ui(new Ui::QLinkT)
{
    ui->setupUi(this);
    pau = new Pause();

    //打开存档
    fstream file("save.txt");
    //提取第一行模式描述性标识
    string str;
    getline(file, str);
    //打开pause界面
    connect(this, SIGNAL(stop()), this, SLOT(pause()));
    //关闭pause界面并保存游戏
    connect(pau, SIGNAL(sendCloseSignal()), this, SLOT(getCloseSignal()));
    connect(pau, SIGNAL(sendCloseSignal()), this, SLOT(saveGame()));
    //关闭pause界面但不保存游戏
    connect(pau, SIGNAL(notSave()), this, SLOT(getCloseSignal()));
    //打开menu界面
    connect(this, SIGNAL(openNewMenu()), this, SLOT(acceptOpenNewMenu()));
    //定时更新时间
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QLink::update));
    connect(timer, SIGNAL(timeout()), this, SLOT(countTime()));
    timer->start(25);

    //还原倒计时
    getline(file, str);
    timeLimit = stoi(str);

    //还原Player1
    getline(file, str);
    int posX1 = stoi(str);
    getline(file, str);
    int posY1 = stoi(str);
    p1 = new Player(posX1, posY1, Qt::black);
    getline(file, str);
    p1->setScore(stoi(str));

    //还原Player2
    getline(file, str);
    int posX2 = stoi(str);
    getline(file, str);
    int posY2 = stoi(str);
    p2 = new Player(posX2, posY2, Qt::darkMagenta);
    getline(file, str);
    p2->setScore(stoi(str));

    //玩家行动与停止(pause)继续(continue)操作
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));

    //还原mapOfBox
    QColor colorMap[5]={Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::gray};
    for (int i = 0; i <= M + 1; ++i)
        for (int j = 0; j <= N + 1; ++j) {
            getline(file, str);
            int cd = stoi(str);
            getline(file,str);
            int s = stoi(str);
            mapOfBox[i][j] = new Box(i, j, colorMap[cd], s);
        }
    file.close();

    //设置窗口
    pix.load("D:/SEP_Program/QLink_2/china.jpg");
    setWindowTitle("QLinkT");
    resize(800, 800);
}

QLinkT::~QLinkT()
{
    delete ui;
}

//
//createMap创建初始的地图，保证各种颜色的箱子为偶数个
//
void QLinkT::createMap()
{
    srand(time(NULL));
    QColor colorMap[5]={Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::gray};
    int cntMap[5] = {0};

    //中心M*N地图随机生成，保证各种颜色方块都是偶数个
    while (true) {
        //随机生成中心地图
        for (int i = 1; i <= M; ++i)
            for (int j = 1; j <= N; ++j) {
                int rnd = rand() % 5;
                mapOfBox[i][j] = new Box(i, j, colorMap[rnd]);
                ++cntMap[rnd];
            }
        //判断各个箱子的个数是否为偶数
        bool isEven = true;
        for (int i = 0; i < 5; ++i)
            if (cntMap[i] % 2 == 1) isEven =false;
        //若都是偶数，则地图生成成功
        if (isEven) break;
        //若不都是偶数，地图生成失败，释放内存，重新生成
        else {
            //释放内存
            for (int i = 1; i <= M; ++i)
                for (int j = 1; j <= N; ++j)
                    delete mapOfBox[i][j];
            //计数器置零
            for (int i = 0; i < 5; ++i) cntMap[i] = 0;
        }
    }

    //生成地图边缘
    for (int i = 0; i <= M + 1; ++i) {
        mapOfBox[i][0] = new Box(i, 0, Qt::white, 0);
        mapOfBox[i][N+1] = new Box(i, N + 1, Qt::white, 0);
    }
    for (int j = 1; j <= N; ++j) {
        mapOfBox[0][j] = new Box(0, j, Qt::white, 0);
        mapOfBox[M+1][j] = new Box(M+1, j, Qt::white, 0);
    }
}

//
//keyPressEvent以键盘事件(event)为参数，将键盘事件存入pressedKeys中
//
void QLinkT::keyPressEvent(QKeyEvent *event)
{
    //摁下P，发出暂停信号
    if (event->key() == Qt::Key_P)
        emit stop();
    //摁下R，暂停后重新开始计时
    else if (event->key() == Qt::Key_R)
        timer->start();
    //其它按键（运动相关）按下，key值放入容器，如果是长按触发的repeat就不判断
    else if(!event->isAutoRepeat())
        pressedKeys.insert(event->key());
}

//
//keyReleaseEvent以键盘事件(event)为参数，松开键盘后删除相应键值
//
void QLinkT::keyReleaseEvent(QKeyEvent *event)
{
    //按键释放，从容器中移除，如果是长按触发的repeat就不判断
    if(!event->isAutoRepeat())
       pressedKeys.remove(event->key());
}

//paintEvent在窗口内绘制图形
void QLinkT::paintEvent(QPaintEvent *event)
{
     QPainter painter(this);

     //绘制背景
     painter.drawPixmap(0, 0, 800, 800, pix);
     //绘制地图
     for (int i = 1; i <= M; ++i)
         for (int j = 1; j <= N; ++j)
             mapOfBox[i][j]->paintBox(&painter);
     //绘制人物
     p1->paintPlayer(&painter);
     p2->paintPlayer(&painter);
     //激活两个箱子后重新绘画
     if (p1->arrSize() == 2)
         copeTwoBox(p1, &painter);
     if (p2->arrSize() == 2)
         copeTwoBox(p2, &painter);
     //显示倒计时
     showTime(&painter);
     //显示得分
     showScore(&painter);
     //显示游戏是否可解
     showCanContinue();

}

//
//move_W接收玩家类(p)为参数，按键W摁下后相应玩家执行(位置移动/激活方块)
//
void QLinkT::move_W(Player *p)
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p->getX();
            int y = p->getY();
            //判断是否满足相应的位置关系
            if (abs(x - PosX - 10) <= 20 && y - PosY == 40 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p->addInArr(i, j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p->Press_W();
}

//
//move_A接收玩家类(p)为参数，按键A摁下后相应玩家执行(位置移动/激活方块)
//
void QLinkT::move_A(Player *p)
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p->getX();
            int y = p->getY();
            //判断是否满足相应的位置关系
            if (abs(y - PosY - 10) <= 20 && x - PosX == 40 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p->addInArr(i, j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p->Press_A();
}

//
//move_S接收玩家类(p)为参数，按键S摁下后相应玩家执行(位置移动/激活方块)
//
void QLinkT::move_S(Player *p)
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p->getX();
            int y = p->getY();
            //判断是否满足相应的位置关系
            if (abs(x-PosX-10) <= 20 && y - PosY == -20 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p->addInArr(i,j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p->Press_S();
}

//
//move_D接收玩家类(p)为参数，按键D摁下后相应玩家执行(位置移动/激活方块)
//
void QLinkT::move_D(Player *p)
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p->getX();
            int y = p->getY();
            //判断是否满足相应的位置关系
            if (abs(y-PosY-10) <= 20 && x - PosX == -20 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p->addInArr(i,j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p->Press_D();
}

//
//isLinkedOnce接受两个位置坐标(r1,c1)(r2,c2)作为形参，返回布尔值
//布尔值为命题两个箱子能否通过一次/零次弯折相连的真假
//
bool QLinkT::isLinkedOnce(int r1, int c1, int r2, int c2)
{
    int minC = min(c1, c2), maxC = max(c1, c2);
    //VerFLag，ParrFlag分别表示竖直、水平路劲能否相连
    bool VerFlag = true,ParrFlag = true;

    //r1=r2的情况
    if (r1 == r2) {
        ParrFlag = false;   //此时只能为竖直路径
        for (int j = minC + 1; j < maxC; ++j)
            if (mapOfBox[r1][j]->returnState() == 1) VerFlag = false;
    }
    //c1=c2的情况
    else if (c1 == c2) {
        VerFlag = false;    //此时只能为水平路径
        int minR = min(r1, r2), maxR = max(r1, r2);
        for (int i = minR + 1; i < maxR; ++i)
            if (mapOfBox[i][c1]->returnState() == 1) ParrFlag = false;
    }
    //r1<=r2的情况
    else if (r1 <= r2) {
        //判断竖直路径是否连通
        for (int i = r1; i <= r2; ++i)
            if (mapOfBox[i][c1]->returnState() == 1 && i != r1) VerFlag=false;
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r2][j]->returnState() == 1 && j != c2) VerFlag=false;

        //判断水平路径是否连通
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r1][j]->returnState() == 1 && j != c1) ParrFlag=false;
        for (int i = r1; i <= r2; ++i)
            if (mapOfBox[i][c2]->returnState() == 1 && i != r2) ParrFlag=false;
        }
    else {
        //判断水平路径是否连通
        for (int i = r2; i <= r1; ++i)
            if (mapOfBox[i][c2]->returnState() == 1 && i != r2) ParrFlag=false;
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r1][j]->returnState() == 1 && j != c1) ParrFlag=false;

        //判断竖直路径是否连通
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r2][j]->returnState() == 1 && j != c2) VerFlag=false;
        for (int i = r2; i <= r1; ++i)
            if (mapOfBox[i][c1]->returnState() == 1 && i != r1) VerFlag=false;
    }
    if (!ParrFlag && !VerFlag) return false;
    else return true;
}

//
//isLinkedOnce接受两个位置坐标(r1,c1)(r2,c2)以及路径(path)作为形参，返回布尔值
//布尔值为命题两个箱子能否通过一次/零次弯折相连的真假
//同时以引用传递的方式返回两个方块以何种路径相连
//0-无法相连 1-从(r1,c1)出发，先水平后垂直 2-从(r1,c1)出发，先垂直后水平
//
bool QLinkT::isLinkedOnce(int r1, int c1, int r2, int c2, int &path)
{
    int minC=min(c1, c2),maxC=max(c1, c2);
    //VerFLag，ParrFlag分别表示竖直、水平路劲能否相连
    bool VerFlag=true,ParrFlag=true;

    //r1<=r2的情况
    if (r1 <= r2) {
        //判断竖直路径是否连通
        for (int i = r1; i <= r2; ++i)
            if (mapOfBox[i][c1]->returnState() == 1) VerFlag=false;
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r2][j]->returnState() == 1) VerFlag=false;

        //判断水平路径是否连通
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r1][j]->returnState() == 1) ParrFlag=false;
        for (int i = r1; i <= r2; ++i)
            if (mapOfBox[i][c2]->returnState() == 1) ParrFlag=false;
        }
    else {
        //判断水平路径是否连通
        for (int i = r2; i <= r1; ++i)
            if (mapOfBox[i][c2]->returnState() == 1) ParrFlag=false;
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r1][j]->returnState() == 1) ParrFlag=false;

        //判断竖直路径是否连通
        for (int j = minC; j <= maxC; ++j)
            if (mapOfBox[r2][j]->returnState() == 1) VerFlag=false;
        for (int i = r2; i <= r1; ++i)
            if (mapOfBox[i][c1]->returnState() == 1) VerFlag=false;
    }

    //判断以何种方式相连
    //无法相连
    if (!ParrFlag && !VerFlag) {
        path = 0;
        return false;
    }

    //水平相连
    else if (ParrFlag) {
        path = 1;
        return true;
    }
    //竖直相连
    else if (VerFlag) {
        path = 2;
        return true;
    }
}

//
//isDeleted接受两个位置坐标(r1,c1)(r2,c2)作为形参，返回布尔值
//判断两个箱子能否消除
//
bool QLinkT::isDeleted(int r1, int c1, int r2, int c2)
{
    //颜色不同，直接返回false
    if (mapOfBox[r1][c1]->returnColor() != mapOfBox[r2][c2]->returnColor()) return false;

    //一次/零次弯折相连的情况
    if (isLinkedOnce(r1, c1, r2, c2)) return true;

    //两次弯折相连的情况
    for (int i = r1; i <= M + 1; ++i) {
        if (mapOfBox[i][c1]->returnState() == 1 && i != r1)  break;
        if (isLinkedOnce(i, c1, r2, c2)) return true;
    }

    for (int i = r1; i >= 0; --i) {
        if (mapOfBox[i][c1]->returnState() == 1 && i != r1)  break;
        if (isLinkedOnce(i, c1, r2, c2)) return true;
    }

    for (int j = c1; j <= N + 1; ++j) {
        if (mapOfBox[r1][j]->returnState() == 1 && j != c1) break;
        if (isLinkedOnce(r1, j, r2, c2)) return true;
    }

    for (int j = c1; j >= 0; --j) {
        if (mapOfBox[r1][j]->returnState() == 1 && j != c1) break;
        if (isLinkedOnce(r1, j, r2, c2)) return true;
    }
    return false;
}

//
//isDeleted接受两个位置坐标(r1,c1)(r2,c2)以及绘画工具(painter)作为形参，返回布尔值
//布尔值为命题两个箱子能否消除的真假,绘制消去的路径
//
bool QLinkT::isDeleted(int r1, int c1, int r2, int c2, QPainter *painter)
{
    //颜色不同，直接返回false
    if (mapOfBox[r1][c1]->returnColor() != mapOfBox[r2][c2]->returnColor()) return false;

    int path = 0;
    painter->setPen(QPen(Qt::red,10));
    painter->setBrush(Qt::NoBrush);
    //一次/零次弯折相连的情况
    if (isLinkedOnce(r1,c1,r2,c2,path)) {
        if (path == 1) {
            painter->drawLine(120 + 40 * c1, 120 + 40 * r1, 120 + 40 * c2, 120 + 40 * r1);
            painter->drawLine(120 + 40 * c2, 120 + 40 * r1, 120 + 40 * c2, 120 + 40 * r2);
        }
        if (path == 2) {
            painter->drawLine(120 + 40 * c1, 120 + 40 * r1, 120 + 40 * c1, 120 + 40 * r2);
            painter->drawLine(120 + 40 * c1, 120 + 40 * r2, 120 + 40 * c2, 120 + 40 * r2);
        }
        return true;
    }

    //两次弯折相连的情况
    for (int i = r1; i <= M + 1; ++i) {
        if (mapOfBox[i][c1]->returnState() == 1)  break;
        if (isLinkedOnce(i, c1, r2, c2, path)) {
            painter->drawLine(120 + 40 * c1, 120 + 40 * r1, 120 + 40 * c1, 120 + 40 * i);
            painter->drawLine(120 + 40 * c1, 120 + 40 * i, 120 + 40 * c2, 120 + 40 * i);
            painter->drawLine(120 + 40 * c2, 120 + 40 * i, 120 + 40 * c2, 120 + 40 * r2);
            return true;
        }
    }

    for (int i = r1; i >= 0; --i) {
        if (mapOfBox[i][c1]->returnState() == 1)  break;
        if (isLinkedOnce(i, c1, r2, c2, path)) {
            painter->drawLine(120 + 40 * c1, 120 + 40 * r1, 120 + 40 * c1, 120 + 40 * i);
            painter->drawLine(120 + 40 * c1, 120 + 40 * i, 120 + 40 * c2, 120 + 40 * i);
            painter->drawLine(120 + 40 * c2, 120 + 40 * i, 120 + 40 * c2, 120 + 40 * r2);
            return true;
        }
    }

    for (int j = c1; j <= N + 1; ++j) {
        if (mapOfBox[r1][j]->returnState() == 1) break;
        if (isLinkedOnce(r1, j, r2, c2, path)) {
            painter->drawLine(120 + 40 * c1, 120 + 40 * r1, 120 + 40 * j, 120 + 40 * r1);
            painter->drawLine(120 + 40 * j, 120 + 40 * r1, 120 + 40 * j, 120 + 40 * r2);
            painter->drawLine(120 + 40 * j, 120 + 40 * r2, 120 + 40 * c2, 120 + 40 * r2);
            return true;
        }
    }

    for (int j = c1; j >= 0; --j) {
        if (mapOfBox[r1][j]->returnState() == 1) break;
        if (isLinkedOnce(r1, j, r2, c2, path)) {
            painter->drawLine(120 + 40 * c1, 120 + 40 * r1, 120 + 40 * j, 120 + 40 * r1);
            painter->drawLine(120 + 40 * j, 120 + 40 * r1, 120 + 40 * j, 120 + 40 * r2);
            painter->drawLine(120 + 40 * j, 120 + 40 * r2, 120 + 40 * c2, 120 + 40 * r2);
            return true;
        }
    }
    return false;
}

//
//isEmpty判断地图是否为空,返回判断结果的布尔值
//
bool QLinkT::isEmpty()
{
    bool flag = true;
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j)
            if (mapOfBox[i][j]->returnState() >= 1) {
                flag = false;
                break;
            }
    return flag;

}

//
//showTime接受绘画工具(painter)作为形参，绘制倒计时
//
void QLinkT::showTime(QPainter *painter)
{
    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    QFont f = painter->font();
    f.setFamily("黑体");
    f.setPixelSize(20);
    painter->setFont(f);

    QString str;
    str.setNum(timeLimit);

    painter->drawText(320, 40, "倒计时:");
    painter->drawText(400, 40, str);
}

//
//showScore接受绘画工具(painter)作为形参，绘制得分
//
void QLinkT::showScore(QPainter *painter)
{
    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    QFont f=painter->font();
    f.setFamily("黑体");
    f.setPixelSize(20);
    painter->setFont(f);

    QString str;

    str.setNum(p1->getScore());
    painter->drawText(580, 40, "p1得分:");
    painter->drawText(660, 40, str);

    str.setNum(p2->getScore());
    painter->drawText(580, 80, "p2得分:");
    painter->drawText(660, 80, str);
}

//
//showCanContinue显示游戏能否继续
//
void QLinkT::showCanContinue()
{
    //cnt起到延时效果
    static int cnt=0;
    if (!canContinue()) {
       if ( cnt++ < 20 || cnt > 21) return; //延时0.5s
       //若游戏不能继续，则关闭倒计时、重置延时效果
       timer->stop();
       cnt = 0;
       //若游戏不能继续且有剩余方块，则跳出弹窗
       if (!isEmpty())
          QMessageBox::warning(this, "游戏结束", QObject::trUtf8("不可解!"));
       //若游戏不能继续但没有剩余方块，显示通关
       else
           QMessageBox::warning(this, "游戏结束", QObject::trUtf8("恭喜通关!"));
       close();
       //回到菜单页面
       emit openNewMenu();
    }
}

//
//canContinue检测游戏是否能继续,返回布尔值
//布尔值代表游戏能否继续进行
//
bool QLinkT::canContinue()
{
    int r1,r2,c1,c2;
    for (int i=0; i < M * N - 1; ++i) {
        r1 = i / M + 1;
        c1 = i % N + 1;
        if (mapOfBox[r1][c1]->returnState() <= 0) continue;
        for (int j = i + 1; j < M * N; ++j) {
            r2 = j / M + 1;
            c2 = j % N + 1;
            if (mapOfBox[r2][c2]->returnState() <= 0) continue;
            if (isDeleted(r1, c1, r2, c2)) return true;
        }
    }
    return false;
}

//
//copeTwoBox接收玩家类(p)绘图工具(painter)作为参数，处理相应玩家两个箱子被激活的情况
//
void QLinkT::copeTwoBox(Player *p, QPainter *painter)
{
    int r1 = p->arrShow(0)->row, c1 = p->arrShow(0)->col;
    int r2 = p->arrShow(1)->row, c2 = p->arrShow(1)->col;
    p->arrClear();
    //如果第一个箱子已经另一玩家被消除，消除arr中第一个箱子元素，保留第二个
    if (!mapOfBox[r1][c1]->returnState()) {
        p->addInArr(r2,c2);
        return;
    }
    //能被消除的情况
    if (isDeleted(r1, c1, r2, c2, painter)) {
        painter->setPen(Qt::black);
        QFont f = painter->font();
        f.setFamily("黑体");
        f.setPixelSize(20);
        painter->setFont(f);

        painter->drawText(QPoint(200,40), QString("消除"));
        mapOfBox[r1][c1]->setState(0);
        mapOfBox[r2][c2]->setState(0);
        p->addScore();
    }
    //不能被消除的情况
    else {
        painter->setPen(Qt::black);
        QFont f = painter->font();
        f.setFamily("黑体");
        f.setPixelSize(20);
        painter->setFont(f);

        painter->drawText(QPoint(200, 40), QString("不可消除"));
        mapOfBox[r1][c1]->setState(1);
        mapOfBox[r2][c2]->setState(1);
    }

}

///
//pause处理按键P按下后的暂停操作
//暂停界面打开，时间暂停
//
void QLinkT::pause()
{
    pau->show();
    timer->stop();
}

//
//acceptOpenNewMenu打开新的menu界面
//
void QLinkT::acceptOpenNewMenu()
{
    Menu *m = new Menu;
    m->show();
}

//
//getCloseSignal处理关闭信号
//关闭本页面
//
void QLinkT::getCloseSignal()
{
    this->close();
}

//
//countTime更新倒计时
//
void QLinkT::countTime()
{
    static int cnt = 0;
    if (++cnt % 40 == 0)
        --timeLimit;
    if (timeLimit <= 0) {
        timer->stop();
        //时间结束后弹窗“时间到！”
        QMessageBox::warning(this, "游戏结束", QObject::trUtf8("时间到!"));
        close();
        emit openNewMenu();
    }
}

//
//saveGame实现保存游戏功能，将数据输出至"save.txt"
//
void QLinkT::saveGame()
{
    ofstream ss;
    ss.open("save.txt");
    ss.clear();
    //模式表标识，1-单人模式，2-双人模式
    ss << 2 <<endl;
    //倒计时
    ss << timeLimit <<endl;
    //p1的位置和得分
    ss << p1->getX() <<endl;
    ss << p1->getY() <<endl;
    ss << p1->getScore() <<endl;
    //p2的位置和得分
    ss << p2->getX() <<endl;
    ss << p2->getY() <<endl;
    ss << p2->getScore() <<endl;
    //各个box的颜色和状态
    for (int i = 0; i <= M + 1; ++i)
        for (int j = 0; j <= N + 1; ++j) {
            int colorDigit;
            if (mapOfBox[i][j]->returnColor() == Qt::red) colorDigit = 0;
            else if (mapOfBox[i][j]->returnColor() == Qt::green) colorDigit = 1;
            else if (mapOfBox[i][j]->returnColor() == Qt::blue) colorDigit = 2;
            else if (mapOfBox[i][j]->returnColor() == Qt::yellow) colorDigit = 3;
            else colorDigit = 4;
            ss << colorDigit << endl;
            ss << mapOfBox[i][j]->returnState() <<endl;
          }
    ss.close();
}

//
//move函数实现按键具体的操作
//
void QLinkT::move()
{
    if(pressedKeys.isEmpty()) return;
    for (int key:pressedKeys) {
        switch (key) {
            case Qt::Key_W:   //摁下W,p1向上
                move_W(p1);
                break;
            case Qt::Key_A:   //摁下A,p1向左
                move_A(p1);
                break;
            case Qt::Key_S:   //摁下S,p1向下
                move_S(p1);
                break;
            case Qt::Key_D:   //摁下D,p1向右
                move_D(p1);
                break;
            case Qt::Key_Up:  //摁下↑，p2向上
                move_W(p2);
                break;
            case Qt::Key_Left:   //摁下←，p2向左
                move_A(p2);
                break;
            case Qt::Key_Down:   //摁下↓,p2向下
                move_S(p2);
                break;
            case Qt::Key_Right:   //摁下→,p2向右
                move_D(p2);
                break;
        }
    }

}











