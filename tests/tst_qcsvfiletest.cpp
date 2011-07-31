#include <QtCore/QString>
#include <QtTest/QtTest>
#include <../src/qcsvfile.h>
#include <math.h>

class QCSVFileTest : public QObject
{
    Q_OBJECT
private:
    static const char filePathWrite[];
    static const char filePathRead[];

public:
    QCSVFileTest();

private Q_SLOTS:
    void testWriteFile();
    void testCompareResult();
};

const char QCSVFileTest::filePathWrite[] = "tst_qcsvfiletest.csv";
const char QCSVFileTest::filePathRead[] = "../tests/tst_qcsvfiletest.csv";

QCSVFileTest::QCSVFileTest()
{
}

void QCSVFileTest::testWriteFile()
{
    QCSVFile csvFile(4);

    csvFile.setFileName(filePathWrite);
    QVERIFY2(csvFile.open(), "csvFile.open() failed");

    QVERIFY2(csvFile.write(), "csvFile.write() empty row failed");

    csvFile[0] = "A";
    csvFile[1] = "B";
    //csvFile[2] = "C";
    csvFile[3] = "D";
    QVERIFY2(csvFile.write(), "csvFile.write() failed");

    csvFile.setAt(0, "space test");
    csvFile.setAt(1, "comma,test");
    QVERIFY2(csvFile.write(), "csvFile.write() failed");

    csvFile.setAt(0, "b\"e");
    csvFile.setAt(1, "b\"\"e");
    csvFile.setAt(2, " ");
    csvFile.setAt(3, "b\"\"\"e");
    QVERIFY2(csvFile.write(), "csvFile.write() failed");

    csvFile.setAt(0, M_PI * 2);
    csvFile.setAt(1 , M_E * 10.0e-25);
    csvFile.setAt(2, 1.1);
    csvFile.setAt(3, 1./10);
    QVERIFY2(csvFile.write(), "csvFile.write() failed");

    csvFile.setAt(0, -5);
    csvFile.setAt(1, 0);
    csvFile.setAt(2, 3);
    csvFile.setAt(2, (2LL<<32L) - 1);
    QVERIFY2(csvFile.write(), "csvFile.write() failed");

    QDateTime date;
    date.setDate(QDate(2011, 7, 31));
    date.setTime(QTime(17, 03, 15));
    csvFile.setAt(2, date);
    QVERIFY2(csvFile.write(), "csvFile.write() failed");

    csvFile.close();
}

void QCSVFileTest::testCompareResult()
{
    QFile expected(filePathRead), writen(filePathWrite);

    QVERIFY2(expected.open(QFile::ReadOnly), "expected.open() failed");
    QVERIFY2(writen.open(QFile::ReadOnly), "expected.open() failed");

    QCOMPARE(QString(writen.readAll()), QString(expected.readAll()));

    expected.close();
    writen.close();
}

QTEST_APPLESS_MAIN(QCSVFileTest);

#include "tst_qcsvfiletest.moc"
