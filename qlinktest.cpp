#include "qlinktest.h"
#include "qlink.h"
#include <QtTest/QtTest>

QlinkTest::QlinkTest()
{    

}

//验证不同颜色方块不能消除
void QlinkTest::case1_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(!q.isDeleted(2, 1, 3, 1));
}

//验证0折消除
void QlinkTest::case2_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(q.isDeleted(1, 1, 2, 1));
}

//验证1折消除（先水平后竖直）
void QlinkTest::case3_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(q.isDeleted(3, 1, 1, 2));
}

//验证1折消除（先竖直后水平）
void QlinkTest::case4_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(q.isDeleted(4, 2, 2, 3));
}

//验证2折消除（向上探测）
void QlinkTest::case5_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(q.isDeleted(1, 1, 1, 4));
}

//验证2折消除（向下探测）
void QlinkTest::case6_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(q.isDeleted(4, 1, 4, 4));
}

//验证2折消除（向左探测）
void QlinkTest::case7_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(q.isDeleted(1, 1, 4, 1));
}

//验证2折消除(向右探测）
void QlinkTest::case8_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(q.isDeleted(1, 4, 4, 4));
}

//验证3折以上不可消除
void QlinkTest::case9_testcase()
{
    QLink q;
    q.createTestMap();
    QVERIFY(!q.isDeleted(1, 4, 4, 1));
}

//QTEST_MAIN(QlinkTest)

