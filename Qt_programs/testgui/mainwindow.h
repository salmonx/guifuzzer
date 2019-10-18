#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QComboBox comboBox;



private slots:
    void on_pushButton_clicked();

    void on_listButton_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_compare_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
