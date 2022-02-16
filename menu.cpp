#include "menu.h"
#include "qlink.h"
#include "qlinkt.h"
#include "ui_menu.h"
#include "fstream"

Menu::Menu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);
    //点击开始游戏
    connect(ui->StartNewGame, SIGNAL(clicked()), this, SLOT(startNewGame()));
    //点击保存游戏
    connect(ui->LoadGame, SIGNAL(clicked()), this, SLOT(loadGame()));
    //点击退出游戏
    connect(ui->QuitGame, SIGNAL(clicked()), this, SLOT(quitGame()));
}

Menu::~Menu()
{
    delete ui;
}

//
//处理“开始游戏”按键对应的事件
//打开模式选择页面，关闭本页面
//
void Menu::startNewGame()
{
    Mode *mode = new Mode();
    close();
    mode->show();
}

//
//处理“继续游戏”对应的事件
//读取存档
//
void Menu::loadGame()
{
    ifstream file("save.txt");
    string str;
    getline(file,str);
    if (stoi(str) == 1) {
        QLink *q = new QLink(1);
        q->show();
    }
    else if (stoi(str) == 2) {
        QLinkT *q = new QLinkT(2);
        q->show();
    }
    close();

}

//
//处理“退出游戏”对应的事件
//关闭本页面
//
void Menu::quitGame()
{
    close();
}


