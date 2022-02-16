#include "qlink.h"
#include "ui_qlink.h"
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
QLink::QLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QLink)
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
    //定时检测道具是否被激活
    connect(timer, SIGNAL(timeout()), this, SLOT(detectTools()));
    //绑定激活道具信号与实现功能的槽函数
    connect(this, SIGNAL(addTime()), this, SLOT(acceptAddTime()));
    connect(this, SIGNAL(shuffle()), this, SLOT(acceptShuffle()));
    connect(this, SIGNAL(hint()), this, SLOT(acceptHint()));

    //随机生成地图mapOfBox,边缘空出
    createMap();

    //定时生成各类道具
    toolTime = 0.0;
    connect(timer, SIGNAL(timeout()), this, SLOT(createTools()));

    //Hint模式设置
    hintTime = 0.0;
    hintArr.reserve(2);

    //生成Player
    p1 = new Player(40, 40, Qt::black);
    //玩家行动与停止(pause)继续(continue)操作
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));

    //设置窗口
    pix.load("Pictures/china.jpg");
    setWindowTitle("QLink");
    resize(800,800);
}

//
//读取存档时的构造函数
//
QLink::QLink(int mode, QWidget *parent)
    : QMainWindow(parent)
    ,ui(new Ui::QLink)
{
    ui->setupUi(this);
    pau = new Pause();

    //打开存档
    fstream file("save.txt");
    //提取第一行模式描述性标识
    string str;
    getline(file,str);
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
    //定时检测道具是否被激活
    connect(timer, SIGNAL(timeout()), this, SLOT(detectTools()));
    //绑定激活道具信号与实现功能的槽函数
    connect(this, SIGNAL(addTime()), this, SLOT(acceptAddTime()));
    connect(this, SIGNAL(shuffle()), this, SLOT(acceptShuffle()));
    connect(this, SIGNAL(hint()), this, SLOT(acceptHint()));

    //还原Player
    getline(file, str);
    int posX = stoi(str);
    getline(file, str);
    int posY = stoi(str);
    p1 = new Player(posX, posY, Qt::black);
    getline(file, str);
    p1->setScore(stoi(str));
    //玩家行动与停止(pause)继续(continue)操作
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));

    //还原mapOfBox
    QColor colorMap[5]={Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::gray};
    for (int i = 0; i <= M + 1; ++i)
        for (int j = 0; j <= N + 1; ++j) {
            getline(file, str);
            int cd = stoi(str);
            getline(file, str);
            int s = stoi(str);
            mapOfBox[i][j] = new Box(i, j, colorMap[cd], s);
        }
    file.close();

    //定时生成各类道具
    toolTime = 0.0;
    connect(timer, SIGNAL(timeout()), this, SLOT(createTools()));

    //Hint模式设置
    hintTime = 0.0;
    hintArr.reserve(2);

    //设置窗口
    pix.load("D:/SEP_Program/QLink_2/china.jpg");
    setWindowTitle("QLink");
    resize(800,800);
}

QLink::~QLink()
{
    delete ui;
}

//
//createMap创建初始的地图，保证各种颜色的箱子为偶数个
//
void QLink::createMap()
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
void QLink::keyPressEvent(QKeyEvent *event)
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
void QLink::keyReleaseEvent(QKeyEvent *event)
{
    //按键释放，从容器中移除，如果是长按触发的repeat就不判断
    if(!event->isAutoRepeat())
       pressedKeys.remove(event->key());
}

//
//paintEvent在窗口内绘制图形
//
void QLink::paintEvent(QPaintEvent *event)
{
     QPainter painter(this);

     //绘制背景
     painter.drawPixmap(0, 0, 800, 800, pix);
     //绘制地图
     for (int i = 0; i <= M + 1; ++i)
         for (int j = 0; j <= N + 1; ++j)
             mapOfBox[i][j]->paintBox(&painter);     
     //绘制人物
     p1->paintPlayer(&painter);     
     //激活两个箱子后重新绘画
     if (p1->arrSize() == 2)
         copeTwoBox(&painter);     
     //显示倒计时
     showTime(&painter);     
     //显示得分
     showScore(&painter);     
     //Hint模式下绘制高亮
     copeHint(&painter);
     //显示游戏是否可解
     showCanContinue();

}

//
//按键W摁下后执行(位置移动/激活方块)
//
void QLink::move_W()
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p1->getX();
            int y = p1->getY();
            //判断是否满足相应的位置关系
            if (abs(x - PosX - 10) <= 20 && y - PosY == 40 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p1->addInArr(i, j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p1->Press_W();
}

//
//按键A摁下后执行(位置移动/激活方块)
//
void QLink::move_A()
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p1->getX();
            int y = p1->getY();
            //判断是否满足相应的位置关系
            if (abs(y - PosY - 10) <= 20 && x - PosX == 40 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p1->addInArr(i, j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p1->Press_A();
}

//
//按键S摁下后执行(位置移动/激活方块)
//
void QLink::move_S()
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p1->getX();
            int y = p1->getY();
            //判断是否满足相应的位置关系
            if (abs(x - PosX - 10) <= 20 && y - PosY == -20 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p1->addInArr(i, j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p1->Press_S();
}

//
//按键D摁下后执行(位置移动/激活方块)
//
void QLink::move_D()
{
    bool isCrash = false;
    //判定是否接触到了方块
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int PosX = mapOfBox[i][j]->returnPosX();
            int PosY = mapOfBox[i][j]->returnPosY();
            int x = p1->getX();
            int y = p1->getY();
            //判断是否满足相应的位置关系
            if (abs(y - PosY - 10) <= 20 && x - PosX == -20 && mapOfBox[i][j]->returnState() >= 1) {
                //若接触到方块，则激活相应的方块
                isCrash = true;
                mapOfBox[i][j]->setState(2);
                p1->addInArr(i, j);
                break;
            }
        }
    //若没有接触到方块，则进行位置移动
    if (!isCrash) p1->Press_D();
}

//
//isLinkedOnce接受两个位置坐标(r1,c1)(r2,c2)作为形参，返回布尔值
//布尔值为命题两个箱子能否通过一次/零次弯折相连的真假
//
bool QLink::isLinkedOnce(int r1, int c1, int r2, int c2)
{
    int minC = min(c1, c2),maxC = max(c1, c2);
    //VerFLag，ParrFlag分别表示竖直、水平路劲能否相连
    bool VerFlag = true,ParrFlag = true;

    //r1=r2的情况
    if (r1 == r2) {
        ParrFlag = false;   //此时只能是水平路径
        for (int j = minC + 1; j < maxC; ++j)
            if (mapOfBox[r1][j]->returnState() == 1) VerFlag = false;
    }
    //c1=c2的情况
    else if (c1 == c2) {
        VerFlag = false;     //此时只能是竖直路径
        int minR = min(r1, r2), maxR = max(r1, r2);
        for (int i = minR + 1; i < maxR; ++i)
            if (mapOfBox[i][c1]->returnState() == 1) ParrFlag = false;
    }
    //r1<r2的情况
    else if (r1 < r2) {
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
    //r1>r2的情况
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
bool QLink::isLinkedOnce(int r1, int c1, int r2, int c2, int &path)
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
    //r1>r2的情况
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
bool QLink::isDeleted(int r1, int c1, int r2, int c2)
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
bool QLink::isDeleted(int r1, int c1, int r2, int c2, QPainter *painter)
{
    //颜色不同，直接返回false
    if (mapOfBox[r1][c1]->returnColor() != mapOfBox[r2][c2]->returnColor()) return false;

    int path = 0;
    painter->setPen(QPen(Qt::red, 10));
    painter->setBrush(Qt::NoBrush);
    //一次/零次弯折相连的情况
    if (isLinkedOnce(r1, c1, r2, c2, path)) {
        if (path == 1) {
            painter->drawLine(120 + 40 * c1,120 + 40 * r1,120 + 40 * c2, 120 + 40 * r1);
            painter->drawLine(120 + 40 * c2,120 + 40 * r1,120 + 40 * c2, 120 + 40 * r2);
        }
        if (path == 2) {
            painter->drawLine(120 + 40 * c1,120 + 40 * r1,120 + 40 * c1, 120 + 40 * r2);
            painter->drawLine(120 + 40 * c1,120 + 40 * r2,120 + 40 * c2, 120 + 40 * r2);
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
bool QLink::isEmpty()
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
void QLink::showTime(QPainter *painter)
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
void QLink::showScore(QPainter *painter)
{
    painter->setPen(Qt::black);
    painter->setBrush(Qt::NoBrush);
    QFont f = painter->font();
    f.setFamily("黑体");
    f.setPixelSize(20);
    painter->setFont(f);

    QString str;
    str.setNum(p1->getScore());

    painter->drawText(600, 40, "得分:");
    painter->drawText(660, 40, str);
}

//
//showCanContinue显示游戏能否继续
//
void QLink::showCanContinue()
{
    //cnt起到延时效果
    static int cnt=0;
    if (!canContinue()) {
       if ( cnt++ < 20 || cnt > 21) return;
       //若游戏不能继续，则关闭计时器、重置延时效果
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
bool QLink::canContinue()
{
    int r1,r2,c1,c2;
    for (int i = 0; i < M * N - 1; ++i) {
        r1 = i / M + 1;
        c1 = i % N + 1;
        if (mapOfBox[r1][c1]->returnState() <= 0) continue;
        for (int j = i + 1; j < M * N; ++j) {
            r2 = j / M + 1;
            c2 = j % N + 1;
            if (mapOfBox[r2][c2]->returnState() <= 0) continue;
            if (isDeleted(r1,c1,r2,c2)) return true;
        }
    }
    return false;
}

//
//copeTwoBox接收绘图工具(painter)作为参数，处理两个箱子被激活的情况
//
void QLink::copeTwoBox(QPainter *painter)
{
    int r1 = p1->arrShow(0)->row, c1 = p1->arrShow(0)->col;
    int r2 = p1->arrShow(1)->row, c2 = p1->arrShow(1)->col;
    p1->arrClear();
    //能被消除的情况
    if (isDeleted(r1, c1, r2, c2, painter)) {
        painter->setPen(Qt::black);
        QFont f = painter->font();
        f.setFamily("黑体");
        f.setPixelSize(20);
        painter->setFont(f);

        painter->drawText(QPoint(200, 40), QString("消除"));
        mapOfBox[r1][c1]->setState(0);
        mapOfBox[r2][c2]->setState(0);
        p1->addScore();
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

//
//copeHint接收绘图工具(painter)作为参数，实现Hint道具的效果
//
void QLink::copeHint(QPainter *painter)
{
    if (hintTime <= 0) return;
    if (!canContinue()) return;
    hintTime -= 0.025;
    int r1,c1,r2,c2;
    //遍历所有方块，找出能被消除的一对
    for (int i = 0; i < M * N - 1; ++i) {
        r1 = i / M + 1;
        c1 = i % N + 1;
        if (mapOfBox[r1][c1]->returnState() <= 0) continue;
        for (int j = i + 1; j < M * N; ++j) {
            r2 = j / M + 1;
            c2 = j % N + 1;
            if (mapOfBox[r2][c2]->returnState() <= 0) continue;
            if (isDeleted(r1, c1, r2, c2) && hintArr.size() == 0) {
                hintArr.push_back(Pos(r1, c1));
                hintArr.push_back(Pos(r2, c2));
            }
        }
    }
    //如果两个方块处于健在或者激活状态，则将这两个方块高亮
    if (mapOfBox[hintArr[0].row][hintArr[0].col]->returnState() >= 1
            && mapOfBox[hintArr[1].row][hintArr[1].col]->returnState() >= 1) {
        int r1 = hintArr[0].row, c1 = hintArr[0].col;
        int r2 = hintArr[1].row, c2 = hintArr[1].col;
        int posX1 = 100 + 40 * c1, posY1 = 100 + 40 * r1;
        int posX2 = 100 + 40 * c2, posY2 = 100 + 40 * r2;
        painter->setPen(QPen(Qt::black,4));
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(posX1 + 2, posY1 + 2, posX1 + 40 - 2, posY1 + 2);
        painter->drawLine(posX1 + 40 - 2, posY1 + 2, posX1 + 40 - 2, posY1 + 40 - 2);
        painter->drawLine(posX1 + 40 - 2 , posY1 + 40 - 2, posX1 + 2, posY1 + 40 - 2);
        painter->drawLine(posX1 + 2, posY1 + 40 - 2, posX1 + 2, posY1 + 2);
        painter->drawLine(posX2 + 2, posY2 + 2, posX2 + 40 - 2, posY2 + 2);
        painter->drawLine(posX2 + 40 - 2, posY2 + 2, posX2 + 40 - 2, posY2 + 40 - 2);
        painter->drawLine(posX2 + 40 - 2, posY2 + 40 - 2, posX2 + 2, posY2 + 40 - 2);
        painter->drawLine(posX2 + 2, posY2 + 40 - 2, posX2 + 2, posY2 + 2);
    }
    //否则寻找下一对
    else hintArr.clear();
}

//
//createTestMap生成测试集用4*4地图
//注意：使用前需将全局变量M和N更改为4
//
void QLink::createTestMap()
{
    //更改一些方块的颜色
    mapOfBox[1][1]->setColor(Qt::red);
    mapOfBox[2][1]->setColor(Qt::red);
    mapOfBox[1][4]->setColor(Qt::red);
    mapOfBox[4][1]->setColor(Qt::red);
    mapOfBox[4][4]->setColor(Qt::red);
    mapOfBox[1][2]->setColor(Qt::green);
    mapOfBox[3][1]->setColor(Qt::green);
    mapOfBox[2][3]->setColor(Qt::blue);
    mapOfBox[4][2]->setColor(Qt::blue);

    //更改一些方块的状态
    for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j)
            mapOfBox[i][j]->setState(1);
    mapOfBox[2][2]->setState(0);
    mapOfBox[3][2]->setState(0);

}

//
//pause处理按键P按下后的暂停操作
//暂停界面打开，时间暂停
//
void QLink::pause()
{
    pau->show();
    timer->stop();
}

//
//acceptOpenNewMenu打开新的menu界面
//
void QLink::acceptOpenNewMenu()
{
    Menu *m = new Menu;
    m->show();
}

//
//getCloseSignal处理关闭信号
//关闭本页面
//
void QLink::getCloseSignal()
{
    this->close();
}

//
//countTime更新倒计时
//
void QLink::countTime()
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
//detectTools探测是否触发道具
//
void QLink::detectTools()
{
    int x = p1->getX(), y = p1->getY();
    //遍历检测
    for (int i = 0; i <= M + 1; ++i)
        for (int j = 0; j <= N + 1; ++j) {
            int posX = mapOfBox[i][j]->returnPosX();
            int posY = mapOfBox[i][j]->returnPosY();
            bool flag = false;
            if (x - posX >= 0 && x - posX <= 20
                    && y - posY >= 0 && y - posY <= 20)
                flag = true;
            //激活+1s
            if (mapOfBox[i][j]->returnState() == -1 && flag == true) {
                mapOfBox[i][j]->setState(0);
                emit addTime();
                break;
            }
            //激活shuffle
            if (mapOfBox[i][j]->returnState() == -2 && flag == true) {
                mapOfBox[i][j]->setState(0);
                emit shuffle();
                break;
            }
            //激活hint
            if (mapOfBox[i][j]->returnState() == -3 && flag == true) {
                mapOfBox[i][j]->setState(0);
                emit hint();
                break;
            }
        }
}

//
//createTools定时随机生成道具
//
void QLink::createTools()
{
    //若小于设定的时间，则更新计时
    if (toolTime < 5.0) toolTime += 0.025;
    //若大于等于设定时间，生成道具
    else {
        //重置计时
        toolTime = 0.0;
        bool flag = false;
        srand(time(NULL));
        //利用随机数生成道具位置
        while (!flag) {
            int row, col;
            row = rand() % (M + 2);
            col = rand() % (N + 2);
            //当该位置无任何箱子时，生成道具
            if (mapOfBox[row][col]->returnState() == 0) {
                flag = true;
                int state = rand() % 3 - 3;
                mapOfBox[row][col]->setState(state);
            }
        }
    }
}

//
//acceptAddTime实现+1s的功能
//
void QLink::acceptAddTime()
{
    timeLimit += 30;
}

//
//acceptShuffle实现shuffle的功能
//
void QLink::acceptShuffle()
{
     Box *newMapOfBox[M + 2][N + 2];

     //重新排列，采用哈希方法
     for (int i = 1; i <= M; ++i)
        for (int j = 1; j <= N; ++j) {
            int newi = i * 17 % M + 1;
            int newj = j * 17 % N + 1;
            QColor color = mapOfBox[i][j]->returnColor();
            int state = mapOfBox[i][j]->returnState();
            newMapOfBox[newi][newj] = new Box(newi, newj, color, state);
        }
     for (int i = 0; i <= M + 1; ++i) {
         newMapOfBox[i][0] = new Box(i, 0, Qt::white, mapOfBox[i][0]->returnState());
         newMapOfBox[i][N + 1] = new Box(i, N + 1, Qt::white, mapOfBox[i][N + 1]->returnState());
     }
     for (int j = 1; j <= N; ++j) {
         newMapOfBox[0][j] = new Box(0, j, Qt::white, mapOfBox[0][j]->returnState());
         newMapOfBox[M + 1][j] = new Box(M + 1, j, Qt::white, mapOfBox[M + 1][j]->returnState());
     }

     //清空mapOfBox内存
     for (int i = 0; i <= M + 1; ++i)
         for (int j = 0; j <= N+1; ++j)
             delete mapOfBox[i][j];

     //重新赋值
     for (int i = 0; i <= M + 1; ++i)
         for (int j = 0; j <= N + 1; ++j) {
             QColor color = newMapOfBox[i][j]->returnColor();
             int state = newMapOfBox[i][j]->returnState();
             mapOfBox[i][j] = new Box(i, j, color, state);
         }

     //重置player的位置
     p1->setX(40);
     p1->setY(40);

     //清空newMapOfBox内存
     for (int i = 0; i <= M + 1; ++i)
         for (int j = 0; j <= N + 1; ++j)
             delete newMapOfBox[i][j];

}

//
//acceptHint重置hintTime,设置为10s
//
void QLink::acceptHint()
{
    hintTime = 10.0;
}

//
//saveGame实现保存游戏功能，将数据输出至"save.txt"
//
void QLink::saveGame()
{
    ofstream ss;
    ss.open("save.txt");
    ss.clear();
    //模式标识，1-单人模式，2-双人模式
    ss << 1 <<endl;
    //倒计时
    ss << timeLimit <<endl;
    //Player的位置和得分
    ss << p1->getX() <<endl;
    ss << p1->getY() <<endl;
    ss << p1->getScore() <<endl;
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
void QLink::move()
{
    if(pressedKeys.isEmpty()) return;
    for (int key:pressedKeys) {
        switch (key) {
            case Qt::Key_W:   //摁下W,p1向上
                move_W();
                break;
            case Qt::Key_A:   //摁下A,p1向左
                move_A();
                break;
            case Qt::Key_S:   //摁下S,p1向下
                move_S();
                break;
            case Qt::Key_D:   //摁下D,p1向右
                move_D();
                break;
        }
    }

}

