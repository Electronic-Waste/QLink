#pragma once
#ifndef QLINKTEST_H
#define QLINKTEST_H

#include <QObject>
#include <qlink.h>
#include <QtTest/QtTest>

class QlinkTest:public QObject
{
    Q_OBJECT

public:
    QlinkTest();

private slots:
    void case1_testcase();
    void case2_testcase();
    void case3_testcase();
    void case4_testcase();
    void case5_testcase();
    void case6_testcase();
    void case7_testcase();
    void case8_testcase();
    void case9_testcase();

};

#endif // QLINKTEST_H
