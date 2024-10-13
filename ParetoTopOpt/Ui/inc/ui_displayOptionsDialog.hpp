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

#include "TopOptViewer.hpp"

QT_BEGIN_NAMESPACE

//create all widgets in this class
class Ui_displayOptionsDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *geomLayout;
    QComboBox *geomComboBox;
    QLabel *geomLabel;

    QHBoxLayout *fieldLayout;
    QComboBox *fieldComboBox;
    QLabel *fieldLabel;

    QHBoxLayout *XCuttingPlaneLayout;
    QSpinBox *XCuttingPlaneSpinBox;
    QLabel *XCuttingPlaneLabel;

    QHBoxLayout *YCuttingPlaneLayout;
    QSpinBox *YCuttingPlaneSpinBox;
    QLabel *YCuttingPlaneLabel;

    QHBoxLayout *ZCuttingPlaneLayout;
    QSpinBox *ZCuttingPlaneSpinBox;
    QLabel *ZCuttingPlaneLabel;

    QHBoxLayout *EigenNumberLayout;
    QSpinBox *EigenNumberSpinBox;
    QLabel *EigenNumberLabel;

    QCheckBox *boundingBoxCheckBox;
    QCheckBox *showTrianglesCheckBox;
    QCheckBox *transparentGeomCheckBox;
    QCheckBox *axisCheckBox;
    QCheckBox *showTextCheckBox;
    QCheckBox *topoptConstraintsCheckBox;
    QCheckBox *showAMSupportsCheckBox;
    QCheckBox *elasticityBCCheckBox;
    QCheckBox *thermalBCCheckBox;
    QCheckBox *nonDesignGeomCheckBox;
    QCheckBox *deformationScaleCheckBox;

    QPushButton *animatePushButton;
    QPushButton *hideSelectedPartPushButton;
    QPushButton *saveImagePushButton;
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


        fieldLayout = new QHBoxLayout(Dialog);
        fieldComboBox = new QComboBox(Dialog);
        fieldLabel = new QLabel(Dialog);
        fieldLayout->addWidget(fieldLabel);
        fieldLayout->addWidget(fieldComboBox);
        mainLayout->addLayout(fieldLayout);

        XCuttingPlaneLayout = new QHBoxLayout(Dialog);
        XCuttingPlaneSpinBox = new QSpinBox(Dialog);
        XCuttingPlaneSpinBox->setRange(0, 100);
        XCuttingPlaneSpinBox->setSingleStep(5);
        XCuttingPlaneSpinBox->setValue(0);
        XCuttingPlaneLabel = new QLabel(Dialog);
        XCuttingPlaneLayout->addWidget(XCuttingPlaneLabel);
        XCuttingPlaneLayout->addWidget(XCuttingPlaneSpinBox);
        mainLayout->addLayout(XCuttingPlaneLayout);

        YCuttingPlaneLayout = new QHBoxLayout(Dialog);
        YCuttingPlaneSpinBox = new QSpinBox(Dialog);
        YCuttingPlaneSpinBox->setRange(0, 100);
        YCuttingPlaneSpinBox->setSingleStep(5);
        YCuttingPlaneSpinBox->setValue(0);
        YCuttingPlaneLabel = new QLabel(Dialog);
        YCuttingPlaneLayout->addWidget(YCuttingPlaneLabel);
        YCuttingPlaneLayout->addWidget(YCuttingPlaneSpinBox);
        mainLayout->addLayout(YCuttingPlaneLayout);

        ZCuttingPlaneLayout = new QHBoxLayout(Dialog);
        ZCuttingPlaneSpinBox = new QSpinBox(Dialog);
        ZCuttingPlaneSpinBox->setRange(0, 100);
        ZCuttingPlaneSpinBox->setSingleStep(5);
        ZCuttingPlaneSpinBox->setValue(0);
        ZCuttingPlaneLabel = new QLabel(Dialog);
        ZCuttingPlaneLayout->addWidget(ZCuttingPlaneLabel);
        ZCuttingPlaneLayout->addWidget(ZCuttingPlaneSpinBox);
        mainLayout->addLayout(ZCuttingPlaneLayout);


        EigenNumberLayout = new QHBoxLayout(Dialog);
        EigenNumberSpinBox = new QSpinBox(Dialog);
        EigenNumberSpinBox->setRange(1, MAX_EIGEN_MODES);
        EigenNumberSpinBox->setSingleStep(1);
        EigenNumberSpinBox->setValue(1);
        EigenNumberLabel = new QLabel(Dialog);
        EigenNumberLayout->addWidget(EigenNumberLabel);
        EigenNumberLayout->addWidget(EigenNumberSpinBox);
        mainLayout->addLayout(EigenNumberLayout);


        boundingBoxCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(boundingBoxCheckBox);

        showTrianglesCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(showTrianglesCheckBox);

        showTextCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(showTextCheckBox);

        deformationScaleCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(deformationScaleCheckBox);

        transparentGeomCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(transparentGeomCheckBox);

        showAMSupportsCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(showAMSupportsCheckBox);
        showAMSupportsCheckBox->hide();

        axisCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(axisCheckBox);

        elasticityBCCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(elasticityBCCheckBox);

        thermalBCCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(thermalBCCheckBox);

        topoptConstraintsCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(topoptConstraintsCheckBox);

        nonDesignGeomCheckBox = new QCheckBox(Dialog);
        mainLayout->addWidget(nonDesignGeomCheckBox);

        animatePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(animatePushButton);

        hideSelectedPartPushButton = new QPushButton(Dialog);
        mainLayout->addWidget(hideSelectedPartPushButton);

        saveImagePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(saveImagePushButton);

        resetViewPushButton = new QPushButton(Dialog);
        mainLayout->addWidget(resetViewPushButton);

        closePushButton = new QPushButton(Dialog);
        mainLayout->addWidget(closePushButton);

        if (ONLY_FEA){
            topoptConstraintsCheckBox->hide();
            hideSelectedPartPushButton->hide();
            nonDesignGeomCheckBox->hide();
        }
        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }
    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("DisplayDialog", "Display Options", Q_NULLPTR));

        geomLabel->setText(QApplication::translate("displayOptionsDialog", "Geometry", Q_NULLPTR));
        geomComboBox->clear();
        if (PARETO_RESEARCH){
            geomComboBox->insertItems(0, QStringList()
                                      << QApplication::translate("displayOptionsDialog", "None", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "InitialDesign", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "FinalTopology", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "Mesh", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "FinalDesign", Q_NULLPTR)
                                      << QApplication::translate("displayOptionsDialog", "TempGeom", Q_NULLPTR)
                                      );
        }
        else {
            geomComboBox->insertItems(0, QStringList()
                                  << QApplication::translate("displayOptionsDialog", "None", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "InitialDesign", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "FinalTopology", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "Mesh", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "FinalDesign", Q_NULLPTR)
                                  );
        }

        fieldLabel->setText(QApplication::translate("displayOptionsDialog", "Field", Q_NULLPTR));
        fieldComboBox->clear();

        XCuttingPlaneLabel->setText(QApplication::translate("displayOptionsDialog", "XCuttingPercent", Q_NULLPTR));

        YCuttingPlaneLabel->setText(QApplication::translate("displayOptionsDialog", "YCuttingPercent", Q_NULLPTR));

        ZCuttingPlaneLabel->setText(QApplication::translate("displayOptionsDialog", "ZCuttingPercent", Q_NULLPTR));

        EigenNumberLabel->setText(QApplication::translate("displayOptionsDialog", "EigenNumber", Q_NULLPTR));

         if (PARETO_RESEARCH){
             fieldComboBox->insertItems(0, QStringList()
                                  << QApplication::translate("displayOptionsDialog", "None", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "Displacement", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "Stress", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "EigenMode", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "Temperature", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "SIMP", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "TSField", Q_NULLPTR)
                                  );
         }
         else {
                fieldComboBox->insertItems(0, QStringList()
                                     << QApplication::translate("displayOptionsDialog", "None", Q_NULLPTR)
                                     << QApplication::translate("displayOptionsDialog", "Displacement", Q_NULLPTR)
                                     << QApplication::translate("displayOptionsDialog", "Stress", Q_NULLPTR)
                                     << QApplication::translate("displayOptionsDialog", "EigenMode", Q_NULLPTR)
                                     << QApplication::translate("displayOptionsDialog", "Temperature", Q_NULLPTR)
                                     );
         }

        /*

        viewLabel->setText(QApplication::translate("displayOptionsDialog", "Views", Q_NULLPTR));
        viewComboBox->clear();

        viewComboBox->insertItems(0, QStringList()
                                  << QApplication::translate("displayOptionsDialog", "Iso", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "XY+", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "XY-", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "ZY+", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "ZY-", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "ZX+", Q_NULLPTR)
                                  << QApplication::translate("displayOptionsDialog", "ZX-", Q_NULLPTR)
                                  );

                                  */
        deformationScaleCheckBox->setText(QApplication::translate("displayOptionsDialog", "Scale deformation", Q_NULLPTR));
        boundingBoxCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show bounding box", Q_NULLPTR));
        showTrianglesCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show triangles", Q_NULLPTR));
        showTextCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show text", Q_NULLPTR));
        transparentGeomCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show transparent geometry", Q_NULLPTR));
        axisCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show axis", Q_NULLPTR));
        elasticityBCCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show structural loads", Q_NULLPTR));
        thermalBCCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show thermal loads", Q_NULLPTR));
        topoptConstraintsCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show TopOpt Constraints", Q_NULLPTR));
        nonDesignGeomCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show non-design parts", Q_NULLPTR));
        showAMSupportsCheckBox->setText(QApplication::translate("displayOptionsDialog", "Show AM Supports", Q_NULLPTR));
        animatePushButton->setText(QApplication::translate("displayOptionsDialog", "Animate for 3 cycles", Q_NULLPTR));
        hideSelectedPartPushButton->setText(QApplication::translate("displayOptionsDialog", "Hide Selected Part", Q_NULLPTR));
        saveImagePushButton->setText(QApplication::translate("displayOptionsDialog", "Save Image", Q_NULLPTR));
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
    void    setViewer (TopOptViewer *v) { viewer = v; }

private slots:
    void updateCuttingPlane();
    void updateEigenModeToDisplay();
    void AMSupportMessage();
    void updateDisplay();
    void closeDialog();
    void animate();
    void hideSelectedPart();
    void saveImage();
    void resetView();

private:
    virtual void showEvent(QShowEvent * event);
    TopOptViewer *viewer;
    void makeConnections();
    map<string,int> geomMap;
    map<string,int> fieldMap;
    map<string,int> orientationMap;
};
