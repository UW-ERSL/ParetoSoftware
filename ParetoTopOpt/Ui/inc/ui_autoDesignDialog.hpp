/********************************************************************************
** Form generated from reading UI file 'ui_topologyOptimizationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_AUTODESIGNDIALOG_H
#define UI_UI_AUTODESIGNDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

#include <QDialog>
#include <QColorDialog>
#include "TopOptViewer.hpp"


QT_BEGIN_NAMESPACE

class Ui_autoDesignDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *topologyControlLayout;
    QLabel *topologyControlLabel;
    QComboBox *topologyControlComboBox;

    QHBoxLayout *surfaceControlLayout;
    QLabel *surfaceControlLabel;
    QComboBox *surfaceControlComboBox;


    // contour smoothening
    QHBoxLayout *smootheningLayout;
    QLabel *smootheningLabel;
    QSpinBox *smootheningSpinBox;

    QCheckBox *transferLoadCheckBox;

    QPushButton *createGeometryPushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        mainLayout = new QVBoxLayout(Dialog);


        topologyControlLayout = new QHBoxLayout(Dialog);
        topologyControlLabel = new QLabel(Dialog);
        topologyControlComboBox =new QComboBox;
        topologyControlLayout->addWidget(topologyControlLabel);
        topologyControlLayout->addWidget(topologyControlComboBox);

        surfaceControlLayout = new QHBoxLayout(Dialog);
        surfaceControlLabel = new QLabel(Dialog);
        surfaceControlComboBox =new QComboBox;
        surfaceControlLayout->addWidget(surfaceControlLabel);
        surfaceControlLayout->addWidget(surfaceControlComboBox);

        smootheningLabel = new QLabel(Dialog);
        smootheningSpinBox = new QSpinBox(Dialog);
        smootheningSpinBox->setRange(1, 20);
        smootheningSpinBox->setSingleStep(1);
        smootheningSpinBox->setValue(10);
        smootheningLayout = new QHBoxLayout(Dialog);

        smootheningLayout->addWidget(smootheningLabel);
        smootheningLayout->addWidget(smootheningSpinBox);

        createGeometryPushButton = new QPushButton(Dialog);


        transferLoadCheckBox = new QCheckBox(Dialog);
        transferLoadCheckBox->hide();

        mainLayout->addLayout(topologyControlLayout);
        mainLayout->addLayout(surfaceControlLayout);
        mainLayout->addLayout(smootheningLayout);
        mainLayout->addWidget(createGeometryPushButton);
        mainLayout->addWidget(transferLoadCheckBox);

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("autoDesignDialog", "Auto Design", Q_NULLPTR));

        topologyControlComboBox->clear();
        topologyControlLabel->setText(QApplication::translate("autoDesignDialog", "Topology Control:", Q_NULLPTR));
        topologyControlComboBox->insertItems(0, QStringList()
                                      << QApplication::translate("autoDesignDialog", "Very Coarse", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Coarse", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Medium", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Fine", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Very Fine", Q_NULLPTR)
                                      );

        surfaceControlComboBox->clear();
        surfaceControlLabel->setText(QApplication::translate("autoDesignDialog", "Surface Control:", Q_NULLPTR));
        surfaceControlComboBox->insertItems(0, QStringList()
                                      << QApplication::translate("autoDesignDialog", "Very Coarse", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Coarse", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Medium", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Fine", Q_NULLPTR)
                                      << QApplication::translate("autoDesignDialog", "Very Fine", Q_NULLPTR)
                                      );


        smootheningLabel->setText(QApplication::translate("autoDesignDialog", "Smoothening:", Q_NULLPTR));
        transferLoadCheckBox->setText(QApplication::translate("displayOptionsDialog", "Transfer Loads", Q_NULLPTR));
        createGeometryPushButton->setText(QApplication::translate("autoDesignDialog", "Extract Design", Q_NULLPTR));

    } // retranslateUi

};

QT_END_NAMESPACE

class AutoDesignDialog : public QDialog, public Ui_autoDesignDialog {
    Q_OBJECT

public:
    AutoDesignDialog( QWidget *parent = 0);
    ~AutoDesignDialog();
    void setViewer (TopOptViewer *v) { viewer = v; }
signals:


private slots:
    void createAutoDesignGeometry();
    void closeDialog();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;

    void makeConnections();
};

#endif
