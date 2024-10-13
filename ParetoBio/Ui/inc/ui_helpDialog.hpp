/***************************************************************/
#ifndef UI_HELPDIALOG_H_
#define UI_HELPDIALOG_H_

// define the standard import files -- add new headers as required
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE // start the QT Namespace

class Ui_HelpDialog { // the class that contains the ui definitions

};

namespace Ui {

class HelpDialog: public Ui_HelpDialog{ // the class HelpDialog inherits its objects from the class Ui_HelpDialog
public:
    // define all the contents of the Ui
    // define the objects as public members
    QVBoxLayout *mainLayout ; // the main layout. Add all new buttons and layouts to this layout
    //radioButtonExample
    QRadioButton *radioButton1;
    QRadioButton *radioButton2;
    QRadioButton *radioButton3;
    QRadioButton *radioButton4;
    QHBoxLayout *horizontalLayout;

    // Text label and lineEdit example
    QLabel *textLabel;
    QLineEdit *lineEdit;
    QHBoxLayout *textBoxLayout;

    // horizontal push button examples
    QPushButton *hPushButton1;
    QPushButton *hPushButton2;
    QPushButton *hPushButton3;
    QHBoxLayout *hPushButtonLayout;
    // vertical push button examples
    QPushButton *vPushButton1;
    QPushButton *vPushButton2;
    QPushButton *vPushButton3;
    QVBoxLayout *vPushButtonLayout;
    // check box examples
    QCheckBox *checkBox1;
    QCheckBox *checkBox2;
    QLabel *checkBoxLabel;
    QHBoxLayout *checkBoxLayout;
    // combo box layout
    QComboBox *comboBox;
    QLabel *comboBoxLabel;

    // construct the objects inside the function setupUi

    void setupUi(QDialog *Dialog)  // takes a QDialog argument
    {
        mainLayout = new QVBoxLayout(Dialog); // construct the main vertical layout
        // radio button example
        horizontalLayout = new QHBoxLayout(Dialog);
        radioButton1 = new QRadioButton(Dialog);
        radioButton2 = new QRadioButton(Dialog);
        radioButton3 = new QRadioButton(Dialog);
        radioButton4 = new QRadioButton(Dialog);
        // add these radio button to the horizontal layout
        horizontalLayout->addWidget(radioButton1);
        horizontalLayout->addWidget(radioButton2);
        horizontalLayout->addWidget(radioButton3);
        horizontalLayout->addWidget(radioButton4);

        // hPush button example
        hPushButton1 = new QPushButton(Dialog);
        hPushButton2 = new QPushButton(Dialog);
        hPushButton3 = new QPushButton(Dialog);
        hPushButtonLayout = new QHBoxLayout(Dialog);
        hPushButtonLayout->addWidget(hPushButton1);
        hPushButtonLayout->addWidget(hPushButton2);
        hPushButtonLayout->addWidget(hPushButton3);

        // vPush button example
        vPushButton1 = new QPushButton(Dialog);
        vPushButton2 = new QPushButton(Dialog);
        vPushButton3 = new QPushButton(Dialog);
        vPushButtonLayout = new QVBoxLayout(Dialog);
        vPushButtonLayout->addWidget(vPushButton1);
        vPushButtonLayout->addWidget(vPushButton2);
        vPushButtonLayout->addWidget(vPushButton3);

        // checkbox example
        checkBox1 = new QCheckBox(Dialog);
        checkBox2 = new QCheckBox(Dialog);
        checkBoxLabel = new QLabel(Dialog);
        checkBoxLayout = new QHBoxLayout(Dialog);
        checkBoxLayout->addWidget(checkBoxLabel);
        checkBoxLayout->addWidget(checkBox1);
        checkBoxLayout->addWidget(checkBox2);


        // combo box label
        comboBox = new QComboBox(Dialog);
        comboBoxLabel = new  QLabel(Dialog);

        // after adding all buttons , call these functions
        mainLayout->addLayout(horizontalLayout);
        mainLayout->addLayout(vPushButtonLayout);
        mainLayout->addLayout(hPushButtonLayout);
        mainLayout->addLayout(checkBoxLayout);
        mainLayout->addWidget(comboBoxLabel);
        mainLayout->addWidget(comboBox);
        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        // set the text to be displayed in the dialog boxes
        //window title
        Dialog->setWindowTitle(QApplication::translate("HelpDialog", "Help ", Q_NULLPTR));
        //radio button
        radioButton1->setText(QApplication::translate("HelpDialog", "Btn1", Q_NULLPTR));
        radioButton2->setText(QApplication::translate("HelpDialog", "Btn2", Q_NULLPTR));
        radioButton3->setText(QApplication::translate("HelpDialog", "Btn3", Q_NULLPTR));
        radioButton4->setText(QApplication::translate("HelpDialog", "Btn4", Q_NULLPTR));

        //hPushButton
        hPushButton1->setText(QApplication::translate("HelpDialog", "hButton1", Q_NULLPTR));
        hPushButton2->setText(QApplication::translate("HelpDialog", "hButton2", Q_NULLPTR));
        hPushButton3->setText(QApplication::translate("HelpDialog", "hButton3", Q_NULLPTR));

        //vPushButton
        vPushButton1->setText(QApplication::translate("HelpDialog", "vButton1", Q_NULLPTR));
        vPushButton2->setText(QApplication::translate("HelpDialog", "vButton2", Q_NULLPTR));
        vPushButton3->setText(QApplication::translate("HelpDialog", "vButton3", Q_NULLPTR));

        //check box
        checkBox1->setText(QApplication::translate("HelpDialog", "cb1", Q_NULLPTR));
        checkBox2->setText(QApplication::translate("HelpDialog", "cb2", Q_NULLPTR));
        checkBoxLabel->setText(QApplication::translate("HelpDialog", "Check Box blah blah abcd", Q_NULLPTR));

        // combo box
        // first clear the combo box
        comboBox->clear();
        // add the items
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("MaterialDialog", "AlloySteel", Q_NULLPTR)
         << QApplication::translate("MaterialDialog", "Ti6AIv", Q_NULLPTR)
         << QApplication::translate("MaterialDialog", "ABSPlastic", Q_NULLPTR)
         << QApplication::translate("MaterialDialog", "AI1060", Q_NULLPTR)
         << QApplication::translate("MaterialDialog", "AMS5862", Q_NULLPTR)
         << QApplication::translate("MaterialDialog", "Custom", Q_NULLPTR)
        );
        comboBoxLabel->setText(QApplication::translate("HelpDialog", "Combox Box Loooong name", Q_NULLPTR));

    }



    //
};

} // make HelpDialog a member of Ui namespace


QT_END_NAMESPACE // end Qt Namespace
#endif
