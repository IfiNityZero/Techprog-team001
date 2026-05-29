#include <QtTest>
#include "../EchoServer/functionsforserver.h"
// add necessary includes here

class FuncForServer_Test : public QObject
{
    Q_OBJECT

public:
    FuncForServer_Test();
    ~FuncForServer_Test();

private slots:
    void test_case1();

};

FuncForServer_Test::FuncForServer_Test()
{

}

FuncForServer_Test::~FuncForServer_Test()
{

}

void FuncForServer_Test::test_case1()
{
    //QVERIFY(parsing("ewqeewqewq") =="error1");
    QVERIFY2(parsing("ewqeewqewq") =="error", "parsing(\"ewqeewqewq\") !=error1");
}

QTEST_APPLESS_MAIN(FuncForServer_Test)

#include "tst_funcforserver_test.moc"
