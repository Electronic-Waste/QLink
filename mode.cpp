#include "mode.h"
#include "ui_mode.h"
#include "qlink.h"
#include "qlinkt.h"

Mode::Mode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mode)
{
    ui->setupUi(this);
    //单人模式
    connect(ui->Single, SIGNAL(clicked()), this, SLOT(Single()));
    //双人模式
    connect(ui->Double, SIGNAL(clicked()), this, SLOT(Double()));
}

Mode::~Mode()
{
    delete ui;
}

//
//打开双人模式窗口
//
void Mode::Double()
{
    this->close();
    QLinkT *q = new QLinkT();
    q->show();
}

//
//打开单人模式窗口
//
void Mode::Single()
{
    this->close();
    QLink *q = new QLink();
    q->show();
}
