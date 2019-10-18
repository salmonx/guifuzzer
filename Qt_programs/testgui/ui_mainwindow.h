/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Fri Oct 11 16:18:40 2019
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QDoubleSpinBox *doubleSpinBox;
    QPushButton *pushButton;
    QComboBox *comboBox;
    QPushButton *listButton;
    QGroupBox *groupBox;
    QPushButton *pushButton_compare;
    QPushButton *pushButton_equal;
    QPushButton *pushButton_startswith;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(400, 300);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        doubleSpinBox = new QDoubleSpinBox(centralWidget);
        doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox"));
        doubleSpinBox->setGeometry(QRect(40, 20, 141, 22));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(190, 20, 75, 23));
        comboBox = new QComboBox(centralWidget);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        comboBox->setGeometry(QRect(40, 60, 141, 22));
        listButton = new QPushButton(centralWidget);
        listButton->setObjectName(QString::fromUtf8("listButton"));
        listButton->setGeometry(QRect(190, 60, 75, 23));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(0, 110, 181, 131));
        pushButton_compare = new QPushButton(groupBox);
        pushButton_compare->setObjectName(QString::fromUtf8("pushButton_compare"));
        pushButton_compare->setGeometry(QRect(20, 20, 75, 23));
        pushButton_equal = new QPushButton(groupBox);
        pushButton_equal->setObjectName(QString::fromUtf8("pushButton_equal"));
        pushButton_equal->setGeometry(QRect(20, 50, 75, 23));
        pushButton_startswith = new QPushButton(groupBox);
        pushButton_startswith->setObjectName(QString::fromUtf8("pushButton_startswith"));
        pushButton_startswith->setGeometry(QRect(20, 80, 75, 23));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 23));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("MainWindow", "PushButton", 0, QApplication::UnicodeUTF8));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "one", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "two", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "three", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "four", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "five", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "six", 0, QApplication::UnicodeUTF8)
        );
        listButton->setText(QApplication::translate("MainWindow", "PushButton", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("MainWindow", "\345\255\227\347\254\246\344\270\262\346\257\224\350\276\203", 0, QApplication::UnicodeUTF8));
        pushButton_compare->setText(QApplication::translate("MainWindow", "compare", 0, QApplication::UnicodeUTF8));
        pushButton_equal->setText(QApplication::translate("MainWindow", "==", 0, QApplication::UnicodeUTF8));
        pushButton_startswith->setText(QApplication::translate("MainWindow", "startswith", 0, QApplication::UnicodeUTF8));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
