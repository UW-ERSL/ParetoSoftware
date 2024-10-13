#ifndef UI_UI_TPMS_H
#define UI_UI_TPMS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include "QDialog"
#include "LatticeViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_TPMSDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *tpmsTypeLayout;
    QHBoxLayout *structureTypeLayout;
    QHBoxLayout *voxelNumberLayout;
    QHBoxLayout *surfThicknessLayout;
    QHBoxLayout *surfSmoothingLayout;
    QHBoxLayout *edgeSharpnessLayout;
    QHBoxLayout *numTPMSLayout;


    QLabel *tpmsTypeLabel;
    QComboBox *tpmsTypeCombo;

    QLabel *structureTypeLabel;
    QComboBox *structureTypeCombo;

    QLabel *voxelNumberLabel;
    QSpinBox *voxelNumberSpinBox;

    QLabel *surfThicknessLabel;
    QDoubleSpinBox *surfThicknessSpinBox;

    QLabel *surfSmoothingLabel;
    QSpinBox *surfSmoothingSpinBox;

    QLabel *edgeSharpnessLabel;
    QSpinBox *edgeSharpnessSpinBox;

    QLabel *numTPMSLabel;
    QSpinBox *numTPMSSpinBox;




    QPushButton *createTPMSPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("TPMSDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);
        tpmsTypeLayout = new QHBoxLayout(Dialog);
        structureTypeLayout = new QHBoxLayout(Dialog);
        voxelNumberLayout = new QHBoxLayout(Dialog);

        surfThicknessLayout = new QHBoxLayout(Dialog);
        surfSmoothingLayout = new QHBoxLayout(Dialog);
        edgeSharpnessLayout = new QHBoxLayout(Dialog);
        numTPMSLayout = new QHBoxLayout(Dialog);

        //tpmsTYPE
        tpmsTypeLabel = new QLabel;
        tpmsTypeCombo = new QComboBox;
        tpmsTypeLayout->addWidget(tpmsTypeLabel);
        tpmsTypeLayout->addWidget(tpmsTypeCombo);
        tpmsTypeCombo->setEnabled(true);

        //structureTYPE
        structureTypeLabel = new QLabel;
        structureTypeCombo = new QComboBox;
        structureTypeLayout->addWidget(structureTypeLabel);
        structureTypeLayout->addWidget(structureTypeCombo);
        structureTypeCombo->setEnabled(true);

        //no. of voxels
        voxelNumberLabel = new QLabel;
        voxelNumberSpinBox = new QSpinBox;
        voxelNumberSpinBox->setRange(1,5);
        voxelNumberSpinBox->setSingleStep(1);
        voxelNumberSpinBox->setValue(1);
        voxelNumberLayout->addWidget(voxelNumberLabel);
        voxelNumberLayout->addWidget(voxelNumberSpinBox);
        voxelNumberSpinBox->setEnabled(true);

        // surface thickness
        surfThicknessLabel = new QLabel;
        surfThicknessSpinBox = new QDoubleSpinBox;
        surfThicknessSpinBox->setRange(0.1, 1);
        surfThicknessSpinBox->setSingleStep(0.05);
        surfThicknessSpinBox->setValue(0.5);
        surfThicknessLayout->addWidget(surfThicknessLabel);
        surfThicknessLayout->addWidget(surfThicknessSpinBox);

        // edge sharpness
        edgeSharpnessLabel = new QLabel;
        edgeSharpnessSpinBox = new QSpinBox;
        edgeSharpnessSpinBox->setRange(1,10);
        edgeSharpnessSpinBox->setSingleStep(1);
        edgeSharpnessSpinBox->setValue(5);
        edgeSharpnessLayout->addWidget(edgeSharpnessLabel);
        edgeSharpnessLayout->addWidget(edgeSharpnessSpinBox);


        // surface smoothness
        surfSmoothingLabel = new QLabel;
        surfSmoothingSpinBox = new QSpinBox;
        surfSmoothingSpinBox->setRange(1,10);
        surfSmoothingSpinBox->setSingleStep(1);
        surfSmoothingSpinBox->setValue(5);
        surfSmoothingLayout->addWidget(surfSmoothingLabel);
        surfSmoothingLayout->addWidget(surfSmoothingSpinBox);

        // no. of TPMS along X,Y,Z
        numTPMSLabel = new QLabel;
        numTPMSSpinBox = new QSpinBox;
        numTPMSSpinBox->setRange(1,100);
        numTPMSSpinBox->setSingleStep(1);
        numTPMSSpinBox->setValue(5);
        numTPMSLayout->addWidget(numTPMSLabel);
        numTPMSLayout->addWidget(numTPMSSpinBox);



        // buttons
        createTPMSPushButton = new QPushButton(Dialog);
        closePushButton = new QPushButton(Dialog);
        mainLayout->addLayout(tpmsTypeLayout);

        mainLayout -> addLayout(structureTypeLayout);

        mainLayout->addLayout(voxelNumberLayout);
        mainLayout->addLayout(edgeSharpnessLayout);
        mainLayout->addLayout(surfThicknessLayout);
        mainLayout->addLayout(surfSmoothingLayout);

        mainLayout->addLayout(numTPMSLayout);
        mainLayout->addWidget(createTPMSPushButton);
        mainLayout->addWidget(closePushButton);

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("TPMSDialog", "TPMS Design", Q_NULLPTR));

        tpmsTypeLabel->setText(QApplication::translate("TPMSTypeDialog",  "TPMS Type",Q_NULLPTR));
        tpmsTypeCombo->clear();
        tpmsTypeCombo->insertItems(0,QStringList()
                                   <<QApplication::translate("TPMSTypeDialog","Swartz Primitive",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","Swartz Diamond",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","Swartz Gyroid",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","Schoen IW",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","Lidinoid-Type",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","Sherk-Type",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","SmoothSphere",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","Neovius Surface",Q_NULLPTR)
                                   <<QApplication::translate("TPMSTypeDialog","D-Prime Surface",Q_NULLPTR)
                                   );

        structureTypeLabel->setText(QApplication::translate("StructureTypeDialog", "Structure Type",Q_NULLPTR));
        structureTypeCombo->clear();

        structureTypeCombo->insertItems(0,QStringList()
                                        <<QApplication::translate("StructureTypeDialog","Skeletal Type",Q_NULLPTR)
                                        <<QApplication::translate("StructureTypeDialog","Sheet Type",Q_NULLPTR)
                                        );

        voxelNumberLabel->setText(QApplication::translate("TPMSDialog", "Voxel Scaling", Q_NULLPTR));
        edgeSharpnessLabel->setText(QApplication::translate("TPMSDialog", "Edge Sharpness", Q_NULLPTR));
        surfThicknessLabel->setText(QApplication::translate("TPMSDialog", "Surface Thickness", Q_NULLPTR));
        surfSmoothingLabel->setText(QApplication::translate("TPMSDialog", "Surface Smoothing", Q_NULLPTR));

        numTPMSLabel->setText(QApplication::translate("TPMSDialog", "Num of TPMS", Q_NULLPTR));
        createTPMSPushButton->setText(QApplication::translate("TPMSDialog", "Create TPMS", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("TPMSDialog", "Close", Q_NULLPTR));
    }
};
QT_END_NAMESPACE

class TPMSDialog : public QDialog, public Ui_TPMSDialog {
    Q_OBJECT
public:
    TPMSDialog( QWidget *parent = 0);
    ~TPMSDialog();
    void setViewer(TemplateViewer *v) { viewer = v;}
signals:

private slots:
    void showEvent(QShowEvent *e);
    void createShell();
    void createTPMS();
    void closeTPMSDialog();
private:
    TemplateViewer *viewer;
    void makeConnections();

    //virtual void showEvent(QShowEvent * event);
    map<string ,int> surfaceSelectionMap;

};
////////////////////////////////////////////////////////////////////////////////

#endif
