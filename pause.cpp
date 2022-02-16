#include "pause.h"
#include "ui_pause.h"
#include "menu.h"

Pause::Pause(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pause)
{
    ui->setupUi(this);

    //点击加载游戏
    connect(ui->LoadGame, SIGNAL(clicked()), this, SLOT(loadGame()));
    //点击保存游戏
    connect(ui->SaveGame, SIGNAL(clicked()), this, SLOT(saveGame()));
    //点击退出游戏
    connect(ui->QuitGame, SIGNAL(clicked()), this, SLOT(quitGame()));
}

Pause::~Pause()
{
    delete ui;
}

//
//执行加载游戏操作
//关闭暂停页面
//
void Pause::loadGame()
{
    this->close();
}

//
//执行保存游戏操作
//关闭暂停页面并发出保存游戏指令
//
void Pause::saveGame()
{
    this->close();
    emit sendCloseSignal();
    Menu *m = new Menu();
    m->show();
}

//
//执行退出游戏操作
//关闭暂停页面并发出不保存游戏的指令
//
void Pause::quitGame()
{
    this->close();
    emit notSave();
    Menu *m = new Menu();
    m->show();
}

