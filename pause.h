#pragma once
#ifndef PAUSE_H
#define PAUSE_H

#include <QWidget>

namespace Ui {
class Pause;
}

class Pause : public QWidget
{
    Q_OBJECT

public:
    explicit Pause(QWidget *parent = nullptr);
    ~Pause();

private:
    Ui::Pause *ui;

private slots:
    void loadGame();
    void saveGame();
    void quitGame();

signals:
    void sendCloseSignal();
    void notSave();
};

#endif // PAUSE_H
