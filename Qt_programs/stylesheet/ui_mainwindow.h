/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *exitAction;
    QAction *aboutQtAction;
    QAction *editStyleAction;
    QAction *aboutAction;
    QWidget *centralwidget;
    QVBoxLayout *vboxLayout;
    QFrame *mainFrame;
    QGridLayout *gridLayout;
    QCheckBox *agreeCheckBox;
    QLabel *label;
    QLabel *nameLabel;
    QRadioButton *maleRadioButton;
    QLabel *passwordLabel;
    QComboBox *countryCombo;
    QLabel *ageLabel;
    QLabel *countryLabel;
    QLabel *genderLabel;
    QLineEdit *passwordEdit;
    QRadioButton *femaleRadioButton;
    QSpinBox *ageSpinBox;
    QComboBox *nameCombo;
    QSpacerItem *spacerItem;
    QSpacerItem *spacerItem1;
    QDialogButtonBox *buttonBox;
    QListWidget *professionList;
    QMenuBar *menubar;
    QMenu *menu_File;
    QMenu *menu_Help;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(400, 413);
        exitAction = new QAction(MainWindow);
        exitAction->setObjectName(QString::fromUtf8("exitAction"));
        aboutQtAction = new QAction(MainWindow);
        aboutQtAction->setObjectName(QString::fromUtf8("aboutQtAction"));
        editStyleAction = new QAction(MainWindow);
        editStyleAction->setObjectName(QString::fromUtf8("editStyleAction"));
        aboutAction = new QAction(MainWindow);
        aboutAction->setObjectName(QString::fromUtf8("aboutAction"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        vboxLayout = new QVBoxLayout(centralwidget);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        mainFrame = new QFrame(centralwidget);
        mainFrame->setObjectName(QString::fromUtf8("mainFrame"));
        mainFrame->setFrameShape(QFrame::StyledPanel);
        mainFrame->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(mainFrame);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        agreeCheckBox = new QCheckBox(mainFrame);
        agreeCheckBox->setObjectName(QString::fromUtf8("agreeCheckBox"));

        gridLayout->addWidget(agreeCheckBox, 6, 0, 1, 5);

        label = new QLabel(mainFrame);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);

        gridLayout->addWidget(label, 5, 0, 1, 1);

        nameLabel = new QLabel(mainFrame);
        nameLabel->setObjectName(QString::fromUtf8("nameLabel"));
        nameLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(nameLabel, 0, 0, 1, 1);

        maleRadioButton = new QRadioButton(mainFrame);
        maleRadioButton->setObjectName(QString::fromUtf8("maleRadioButton"));

        gridLayout->addWidget(maleRadioButton, 1, 1, 1, 1);

        passwordLabel = new QLabel(mainFrame);
        passwordLabel->setObjectName(QString::fromUtf8("passwordLabel"));
        passwordLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(passwordLabel, 3, 0, 1, 1);

        countryCombo = new QComboBox(mainFrame);
        countryCombo->addItem(QString());
        countryCombo->addItem(QString());
        countryCombo->addItem(QString());
        countryCombo->addItem(QString());
        countryCombo->addItem(QString());
        countryCombo->addItem(QString());
        countryCombo->addItem(QString());
        countryCombo->setObjectName(QString::fromUtf8("countryCombo"));

        gridLayout->addWidget(countryCombo, 4, 1, 1, 4);

        ageLabel = new QLabel(mainFrame);
        ageLabel->setObjectName(QString::fromUtf8("ageLabel"));
        ageLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(ageLabel, 2, 0, 1, 1);

        countryLabel = new QLabel(mainFrame);
        countryLabel->setObjectName(QString::fromUtf8("countryLabel"));
        countryLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(countryLabel, 4, 0, 1, 1);

        genderLabel = new QLabel(mainFrame);
        genderLabel->setObjectName(QString::fromUtf8("genderLabel"));
        genderLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(genderLabel, 1, 0, 1, 1);

        passwordEdit = new QLineEdit(mainFrame);
        passwordEdit->setObjectName(QString::fromUtf8("passwordEdit"));
        passwordEdit->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(passwordEdit, 3, 1, 1, 4);

        femaleRadioButton = new QRadioButton(mainFrame);
        femaleRadioButton->setObjectName(QString::fromUtf8("femaleRadioButton"));

        gridLayout->addWidget(femaleRadioButton, 1, 2, 1, 2);

        ageSpinBox = new QSpinBox(mainFrame);
        ageSpinBox->setObjectName(QString::fromUtf8("ageSpinBox"));
        ageSpinBox->setMinimum(12);
        ageSpinBox->setValue(22);

        gridLayout->addWidget(ageSpinBox, 2, 1, 1, 2);

        nameCombo = new QComboBox(mainFrame);
        nameCombo->setObjectName(QString::fromUtf8("nameCombo"));
        nameCombo->setEditable(true);

        gridLayout->addWidget(nameCombo, 0, 1, 1, 4);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem, 1, 4, 1, 1);

        spacerItem1 = new QSpacerItem(61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem1, 2, 3, 1, 2);

        buttonBox = new QDialogButtonBox(mainFrame);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 7, 3, 1, 2);

        professionList = new QListWidget(mainFrame);
        new QListWidgetItem(professionList);
        new QListWidgetItem(professionList);
        new QListWidgetItem(professionList);
        professionList->setObjectName(QString::fromUtf8("professionList"));

        gridLayout->addWidget(professionList, 5, 1, 1, 4);


        vboxLayout->addWidget(mainFrame);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 400, 29));
        menu_File = new QMenu(menubar);
        menu_File->setObjectName(QString::fromUtf8("menu_File"));
        menu_Help = new QMenu(menubar);
        menu_Help->setObjectName(QString::fromUtf8("menu_Help"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
#ifndef QT_NO_SHORTCUT
        label->setBuddy(professionList);
        nameLabel->setBuddy(nameCombo);
        passwordLabel->setBuddy(passwordEdit);
        ageLabel->setBuddy(ageSpinBox);
        countryLabel->setBuddy(countryCombo);
#endif // QT_NO_SHORTCUT

        menubar->addAction(menu_File->menuAction());
        menubar->addAction(menu_Help->menuAction());
        menu_File->addAction(editStyleAction);
        menu_File->addSeparator();
        menu_File->addAction(exitAction);
        menu_Help->addAction(aboutAction);
        menu_Help->addAction(aboutQtAction);

        retranslateUi(MainWindow);

        countryCombo->setCurrentIndex(6);
        professionList->setCurrentRow(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Style Sheet", nullptr));
        exitAction->setText(QApplication::translate("MainWindow", "&Exit", nullptr));
        aboutQtAction->setText(QApplication::translate("MainWindow", "About Qt", nullptr));
        editStyleAction->setText(QApplication::translate("MainWindow", "Edit &Style...", nullptr));
        aboutAction->setText(QApplication::translate("MainWindow", "About", nullptr));
#ifndef QT_NO_TOOLTIP
        agreeCheckBox->setToolTip(QApplication::translate("MainWindow", "Please read the LICENSE file before checking", nullptr));
#endif // QT_NO_TOOLTIP
        agreeCheckBox->setText(QApplication::translate("MainWindow", "I accept the terms and &conditions", nullptr));
        label->setText(QApplication::translate("MainWindow", "Profession:", nullptr));
        nameLabel->setText(QApplication::translate("MainWindow", "&Name:", nullptr));
#ifndef QT_NO_TOOLTIP
        maleRadioButton->setToolTip(QApplication::translate("MainWindow", "Check this if you are male", nullptr));
#endif // QT_NO_TOOLTIP
        maleRadioButton->setText(QApplication::translate("MainWindow", "&Male", nullptr));
        passwordLabel->setText(QApplication::translate("MainWindow", "&Password:", nullptr));
        countryCombo->setItemText(0, QApplication::translate("MainWindow", "Egypt", nullptr));
        countryCombo->setItemText(1, QApplication::translate("MainWindow", "France", nullptr));
        countryCombo->setItemText(2, QApplication::translate("MainWindow", "Germany", nullptr));
        countryCombo->setItemText(3, QApplication::translate("MainWindow", "India", nullptr));
        countryCombo->setItemText(4, QApplication::translate("MainWindow", "Italy", nullptr));
        countryCombo->setItemText(5, QApplication::translate("MainWindow", "Norway", nullptr));
        countryCombo->setItemText(6, QApplication::translate("MainWindow", "Pakistan", nullptr));

#ifndef QT_NO_TOOLTIP
        countryCombo->setToolTip(QApplication::translate("MainWindow", "Specify country of origin", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        countryCombo->setStatusTip(QApplication::translate("MainWindow", "Specify country of origin", nullptr));
#endif // QT_NO_STATUSTIP
        ageLabel->setText(QApplication::translate("MainWindow", "&Age:", nullptr));
        countryLabel->setText(QApplication::translate("MainWindow", "Country:", nullptr));
        genderLabel->setText(QApplication::translate("MainWindow", "Gender:", nullptr));
#ifndef QT_NO_TOOLTIP
        passwordEdit->setToolTip(QApplication::translate("MainWindow", "Specify your password", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        passwordEdit->setStatusTip(QApplication::translate("MainWindow", "Specify your password", nullptr));
#endif // QT_NO_STATUSTIP
        passwordEdit->setText(QApplication::translate("MainWindow", "Password", nullptr));
#ifndef QT_NO_TOOLTIP
        femaleRadioButton->setToolTip(QApplication::translate("MainWindow", "Check this if you are female", nullptr));
#endif // QT_NO_TOOLTIP
        femaleRadioButton->setText(QApplication::translate("MainWindow", "&Female", nullptr));
#ifndef QT_NO_TOOLTIP
        ageSpinBox->setToolTip(QApplication::translate("MainWindow", "Specify your age", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        ageSpinBox->setStatusTip(QApplication::translate("MainWindow", "Specify your age", nullptr));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_TOOLTIP
        nameCombo->setToolTip(QApplication::translate("MainWindow", "Specify your name", nullptr));
#endif // QT_NO_TOOLTIP

        const bool __sortingEnabled = professionList->isSortingEnabled();
        professionList->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = professionList->item(0);
        ___qlistwidgetitem->setText(QApplication::translate("MainWindow", "Developer", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = professionList->item(1);
        ___qlistwidgetitem1->setText(QApplication::translate("MainWindow", "Student", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = professionList->item(2);
        ___qlistwidgetitem2->setText(QApplication::translate("MainWindow", "Fisherman", nullptr));
        professionList->setSortingEnabled(__sortingEnabled);

#ifndef QT_NO_TOOLTIP
        professionList->setToolTip(QApplication::translate("MainWindow", "Select your profession", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        professionList->setStatusTip(QApplication::translate("MainWindow", "Specify your name here", nullptr));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        professionList->setWhatsThis(QApplication::translate("MainWindow", "Specify your name here", nullptr));
#endif // QT_NO_WHATSTHIS
        menu_File->setTitle(QApplication::translate("MainWindow", "&File", nullptr));
        menu_Help->setTitle(QApplication::translate("MainWindow", "&Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
