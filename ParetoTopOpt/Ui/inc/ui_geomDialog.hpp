/********************************************************************************
** Form generated from reading UI file 'ui_geomWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_GEOMWINDOW_H
#define UI_UI_GEOMWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QDialog>
#include <QColorDialog>
#include "TopOptViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_GeometryDialog
{
public:
    QVBoxLayout *mainLayout;
    QPushButton *loadSTLPushButton;
    QPushButton *updateSTLPushButton;

    QPushButton *createTetmeshButton;//research

    QHBoxLayout *geomScalingLayout;
    QLabel *geomScalingLabel;
    QDoubleSpinBox *geomScalingSpinBox;

    QPushButton *scaleGeometryButton;//research

    QPushButton *saveAsSTLButton;//research
    QPushButton *saveAsOffButton;//research
    QPushButton *closePushButton;

    void setupUi(QDialog *GeometryDialog)
    {
        if (GeometryDialog->objectName().isEmpty())
            GeometryDialog->setObjectName(QStringLiteral("GeometryDialog"));

        GeometryDialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(GeometryDialog);

        loadSTLPushButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(loadSTLPushButton);

        updateSTLPushButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(updateSTLPushButton);



        geomScalingLabel = new QLabel(GeometryDialog);
        geomScalingSpinBox = new QDoubleSpinBox(GeometryDialog);
        geomScalingSpinBox->setRange(0.0001, 10000);
        geomScalingSpinBox->setSingleStep(0.05);
        geomScalingSpinBox->setValue(1.0);
        geomScalingLayout = new QHBoxLayout(GeometryDialog);
        geomScalingLayout->addWidget(geomScalingLabel);
        geomScalingLayout->addWidget(geomScalingSpinBox);
        mainLayout->addLayout(geomScalingLayout);


        createTetmeshButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(createTetmeshButton);


        scaleGeometryButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(scaleGeometryButton);

        saveAsSTLButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(saveAsSTLButton);

        saveAsOffButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(saveAsOffButton);


        closePushButton = new QPushButton(GeometryDialog);
        mainLayout->addWidget(closePushButton);

        if  (!PARETO_RESEARCH){
            createTetmeshButton->hide();
            saveAsOffButton->hide();
        }

        retranslateUi(GeometryDialog);

        QMetaObject::connectSlotsByName(GeometryDialog);
    } // setupUi

    void retranslateUi(QDialog *GeometryDialog)
    {
        GeometryDialog->setWindowTitle(QApplication::translate("GeometryDialog", "Geometry", Q_NULLPTR));
        loadSTLPushButton->setText(QApplication::translate("GeometryDialog", "Load geometry", Q_NULLPTR));
        updateSTLPushButton->setText(QApplication::translate("GeometryDialog", "Update geometry", Q_NULLPTR));
        geomScalingLabel->setText(QApplication::translate("GeometryDialog", "Geometry Scale:", Q_NULLPTR));
        createTetmeshButton->setText(QApplication::translate("GeometryDialog", "Create Tetmesh", Q_NULLPTR));
        scaleGeometryButton->setText(QApplication::translate("GeometryDialog", "Apply Scaling", Q_NULLPTR));
        saveAsSTLButton->setText(QApplication::translate("GeometryDialog", "Save as .STL", Q_NULLPTR));
        saveAsOffButton->setText(QApplication::translate("GeometryDialog", "Save as .off", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("GeometryDialog", "Close", Q_NULLPTR));
        //saveSTLPushButton->setText(QApplication::translate("GeometryDialog", "Save STL", Q_NULLPTR));
        //saveLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class GeometryDialog: public Ui_GeometryDialog {};
} // namespace Ui

QT_END_NAMESPACE


class GeometryDialog : public QDialog, public Ui_GeometryDialog {
    Q_OBJECT

public:
    GeometryDialog( QWidget *parent = 0);
    ~GeometryDialog();

    void setViewer(TopOptViewer *v) { viewer = v; }
signals:
    void setGeometryComplete();
private slots:
     void loadSTL();
     void loadGeometry();
     void saveAsSTL();
     void updateSTL();
     void  createTetmesh();
     void scaleGeometry();
     void saveAsOff();
     void closeDialog();
private:
    TopOptViewer *viewer;
    void makeConnections();
    map<string,int> unitMap;
};
////////////////////////////////////////////////////////////////////////////////


#endif // UI_UI_GEOMWINDOW_H
