#pragma once
#include <QDialog>
#include <QColorDialog>
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
#include <QtWidgets/QComboBox>

#include "LatticeViewer.hpp"

QT_BEGIN_NAMESPACE

//create all widgets in this class
class Ui_displayOptionsDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *geomLayout;
    QComboBox *geomComboBox;
    QLabel *geomLabel;

    QCheckBox *boundingBoxCheckBox;
    QCheckBox *showTrianglesCheckBox;
    QCheckBox *transparentGeomCheckBox;
    QCheckBox *axisCheckBox;
    QCheckBox *symmetryCheckBox;
    QCheckBox *elasticityBCCheckBox;
    QCheckBox *thermalBCCheckBox;
    QCheckBox *nonDesignGeomCheckBox;
    QCheckBox *deformationScaleCheckBox;

    QPushButton *animatePushButton;
    QPushButton *resetViewPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);
        mainLayout = new QVBoxLayout(Dialog);

        geomLayout = new QHBoxLayout(Dialog);
        geomComboBox = new QComboBox(Dialog);
        geomLabel = new QLabel(Dialog);
        geomLayout->addWidget(geomLabel);
        geomLayout->addWidget(geomComboBox);
        mainLayout->addLayout(geomLayout);



        boundingBoxCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(boundingBoxCheckBox);

        showTrianglesCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(showTrianglesCheckBox);
        showTrianglesCheckBox->hide();

        deformationScaleCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(deformationScaleCheckBox);
        deformationScaleCheckBox->hide();

        transparentGeomCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(transparentGeomCheckBox);
        transparentGeomCheckBox->hide();

        axisCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(axisCheckBox);

        elasticityBCCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(elasticityBCCheckBox);
        elasticityBCCheckBox->hide();

        thermalBCCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(thermalBCCheckBox);
        thermalBCCheckBox->hide();

        symmetryCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(symmetryCheckBox);
        symmetryCheckBox->hide();

        nonDesignGeomCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(nonDesignGeomCheckBox);
        nonDesignGeomCheckBox->hide();

        animatePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(animatePushButton);
        animatePushButton->hide();

        resetViewPushButton = new QPushButton(Dialog);
        mainLayout->addWidget(resetViewPushButton);

        closePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(closePushButton);
        closePushButton->hide();

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }
    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("DisplayDialog", "Display Options", Q_NULLPTR));

        geomLabel->setText(QApplication::translate("displayOptionsDialog", "Geometry", Q_NULLPTR));
        geomComboBox->clear();

        geomComboBox->insertItems(0, QStringList()
                                      << QApplication::translate("displayOptionsDialog", "None", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "LatticeGraph", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "STLGeometry", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "SolidLattice", Q_NULLPTR)
                                      );

        deformationScaleCheckBox->setText(QApplication::translate("displayOptionsDialog", "Scale deformation", Q_NULLPTR));
        boundingBoxCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show bounding box", Q_NULLPTR));
        showTrianglesCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show triangles", Q_NULLPTR));
        transparentGeomCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show transparent geometry", Q_NULLPTR));
        axisCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show axis", Q_NULLPTR));
        elasticityBCCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show structural loads", Q_NULLPTR));
        thermalBCCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show thermal loads", Q_NULLPTR));
        symmetryCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show symmetry planes", Q_NULLPTR));
        nonDesignGeomCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show non-design parts", Q_NULLPTR));

        animatePushButton->setText(QApplication::translate("displayOptionsDialog", "Animate for 3 cycles", Q_NULLPTR));
        resetViewPushButton->setText(QApplication::translate("displayOptionsDialog", "Reset View", Q_NULLPTR));

        closePushButton->setText(QApplication::translate("displayOptionsDialog", "Close", Q_NULLPTR));

    }
};

QT_END_NAMESPACE


//create the member functions and slots in this class
class DisplayOptionsDialog : public QDialog, public Ui_displayOptionsDialog {
    Q_OBJECT

public:
    DisplayOptionsDialog( QWidget *parent = 0);
    ~DisplayOptionsDialog();
    void    setViewer (TemplateViewer *v) { viewer = v; }

private slots:
    void updateDisplay();
    void closeDialog();
    void resetView();

private:
    virtual void showEvent(QShowEvent * event);
    TemplateViewer *viewer;
    void makeConnections();
    map<string,int> geomMap;
    map<string,int> orientationMap;
};
