#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>

#include "qdebug.h"

 enum Spec { Invalid, Rgb, Hsv, Cmyk, Hsl };
class A{

public:
    union {
        struct {
            ushort alpha;
            ushort red;
            ushort green;
            ushort blue;
            ushort pad;
        } argb;
        struct {
            ushort alpha;
            ushort hue;
            ushort saturation;
            ushort value;
            ushort pad;
        } ahsv;
        struct {
            ushort alpha;
            ushort cyan;
            ushort magenta;
            ushort yellow;
            ushort black;
        } acmyk;

        struct {
            ushort alpha;
            ushort hue;
            ushort saturation;
            ushort lightness;
            ushort pad;
        } ahsl;
        ushort array[5];
    } ct;



    Spec spec;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    QFont font("Times", 10, QFont::Bold);
    printf("sizeof font:%d\n", sizeof(font));

    QColor color;
    printf("sizeof color:%d\n", sizeof(color));



    printf("sizeof ushort %d\n", sizeof(ushort));


    A a;
    a.spec = Rgb;
    a.ct.argb.alpha = 255;
    a.ct.argb.red = 1;
    a.ct.argb.green = 1;
    a.ct.argb.blue = 1;
    a.ct.argb.pad = 0;


    QColor *color1 = new QColor();
    color1->setRgb(1, 2, 32);



    printf("sizeof A %d %x\n", sizeof(a), a);

    color.setRgb(1, 2, 3, 255);

    printf("sizeof color %d\n", sizeof(color));


    bool b1 = true;
    bool b2 = false;
    bool b3 = true;

    printf(" bool %x %x %x\n", &b1, &b2, &b3);

       QComboBox mycombobox;
    printf("QComboBox size: %d\n", sizeof(mycombobox));

    QDate qdate;
    QTime qtime;
    QDateTime qdatetime;
    qdate.setYMD(2019, 9, 24);
    qtime.setHMS(15, 15, 15);

    qdatetime.setDate(qdate);
    qdatetime.setTime(qtime);


    printf("QDate: %d %x, QTime %\n", sizeof(QDate), qdate, sizeof(QTime));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{




}

void MainWindow::on_listButton_clicked()
{
    QString str = ui->comboBox->currentText();
    int idx = ui->comboBox->currentIndex();

    QMessageBox::warning(this, "selected idx", QString::number(idx));
    QMessageBox::warning(this, "selected text", str);
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{

}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{

}
// Unicode case-sensitive compare two same-sized strings
static int ucstrncmp(const QChar *a, const QChar *b, int l)
{
    while (l-- && *a == *b)
        a++,b++;
    if (l==-1)
        return 0;
    return a->unicode() - b->unicode();
}

// Unicode case-sensitive comparison
static int ucstrcmp(const QChar *a, int alen, const QChar *b, int blen)
{
    if (a == b && alen == blen)
        return 0;
    int l = qMin(alen, blen);
    int cmp = ucstrncmp(a, b, l);
    return cmp ? cmp : (alen-blen);
}

int compare(const QString &self, const QString &other)
{
    return ucstrcmp(self.constData(), self.length(), other.constData(), other.length());
}



void MainWindow::on_pushButton_compare_clicked()
{
    QString fileName("c:\\out\\afl.input");

    QString myret = QString();

    QString password("2222222222222222222222");

    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("string cmp"),
                             tr("LoadFile Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }


    QTextStream in(&file);
    QString str = in.readAll();


    if (str.length() != password.length()) return;

    //int ret = compare(str, password);
    int ret = str.compare(password);

    if (ret == 0){
        printf("bingo\n");
        myret = QString::number(ret);
        QMessageBox::warning(this, "compare", myret);

        abort();

    } else {
        printf("compare ret: %d\n", ret);
        myret = QString::number(ret);
        QMessageBox::warning(this, "compare", myret);

    }


    /*
    ret = str == password;
    myret = QString::number(ret);
    QMessageBox::warning(this, "==", myret);

    ret = str.startsWith(password);
    myret = QString::number(ret);
    QMessageBox::warning(this, "startswith", myret);
    */
}
