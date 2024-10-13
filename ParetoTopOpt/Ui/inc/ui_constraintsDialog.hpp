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

#include "TopOptViewer.hpp"

QT_BEGIN_NAMESPACE

//create all widgets in this class
class Ui_constraintsDialog
{
public:
    QVBoxLayout *mainLayout;

    // we will need a layout, a label and a widget for each control

    //draw direction
    QCheckBox *drawDirectionCheckBox;
    QHBoxLayout *drawDirectionLayout;
    QLabel *drawDirectionLabel;
    QComboBox *drawDirectionComboBox;

    //through cut
    QCheckBox *extrudeCheckBox;
    QHBoxLayout *extrudeLayout;
    QLabel *extrudeLabel;
    QComboBox *extrudeComboBox;

    //AMBuild direction
    QCheckBox *AMBuildCheckBox;
    QHBoxLayout *AMBuildLayout;
    QLabel *AMBuildLabel;
    QComboBox *AMBuildComboBox;

    //cyclic Symmetry
    QCheckBox *cyclicSymmetryCheckBox;
    QHBoxLayout *cyclicSymmetryLayout;
    QLabel *cyclicSymmetryLabel;
    QComboBox *cyclicSymmetryComboBox;

    //X Grid Pattern
    QCheckBox *xGridPatternCheckBox;
    QHBoxLayout *xGridPatternLayout;
    QLabel *xGridPatternLabel;
    QSpinBox *xGridPatternSpinBox;

    //Y Grid Pattern
    QCheckBox *yGridPatternCheckBox;
    QHBoxLayout *yGridPatternLayout;
    QLabel *yGridPatternLabel;
    QSpinBox *yGridPatternSpinBox;

    //Z Grid Pattern
    QCheckBox *zGridPatternCheckBox;
    QHBoxLayout *zGridPatternLayout;
    QLabel *zGridPatternLabel;
    QSpinBox *zGridPatternSpinBox;

    //minimum feature size
    QCheckBox *minFeatureSizeCheckBox;
    QHBoxLayout *minFeatureSizeLayout;
    QLabel *minFeatureSizeLabel;
    QDoubleSpinBox *minFeatureSizeSpinBox;

    //maximum feature size
    QCheckBox *maxFeatureSizeCheckBox;
    QHBoxLayout *maxFeatureSizeLayout;
    QLabel *maxFeatureSizeLabel;
    QDoubleSpinBox *maxFeatureSizeSpinBox;

    //Stress Safety Factor
    QCheckBox *stressSafetyFactorCheckBox;
    QHBoxLayout *stressSafetyFactorLayout;
    QLabel *stressSafetyFactorLabel;
    QDoubleSpinBox *stressSafetyFactorSpinBox;

    //displacement limit
    QCheckBox *displacementLimitCheckBox;
    QHBoxLayout *displacementLimitLayout;
    QLabel *displacementLimitLabel;
    QDoubleSpinBox *displacementLimitSpinBox;

    //modal limit
    QCheckBox *modalLimitCheckBox;
    QHBoxLayout *modalLimitLayout;
    QLabel *modalLimitLabel;
    QDoubleSpinBox  *modalLimitSpinBox;

    //temperature limit
    QCheckBox *temperatureLimitCheckBox;
    QHBoxLayout *temperatureLimitLayout;
    QLabel *temperatureLimitLabel;
    QDoubleSpinBox  *temperatureLimitSpinBox;


    QCheckBox *xSymmetryCheckBox;
    QCheckBox *ySymmetryCheckBox;
    QCheckBox *zSymmetryCheckBox;

    //fixed faces
    QCheckBox *keepTopologyConnected;

    //fixed faces
    QCheckBox *keepFixedFacesCheckBox;
    //close
    QPushButton *applyPushButton;
    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        mainLayout = new QVBoxLayout(Dialog);

        // create widgets and layouts

        drawDirectionCheckBox = new QCheckBox(Dialog);
        drawDirectionCheckBox->setMaximumWidth(20);
        drawDirectionLabel = new QLabel(Dialog);
        drawDirectionComboBox = new QComboBox(Dialog);
        drawDirectionComboBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        drawDirectionComboBox->setEnabled(false);
        drawDirectionLayout = new QHBoxLayout(Dialog);

        extrudeCheckBox = new QCheckBox(Dialog);
        extrudeCheckBox->setMaximumWidth(20);
        extrudeLabel = new QLabel(Dialog);
        extrudeComboBox = new QComboBox(Dialog);
        extrudeComboBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        extrudeComboBox->setEnabled(false);
        extrudeLayout = new QHBoxLayout(Dialog);

        AMBuildCheckBox = new QCheckBox(Dialog);
        AMBuildCheckBox->setMaximumWidth(20);
        AMBuildLabel = new QLabel(Dialog);
        AMBuildComboBox = new QComboBox(Dialog);
        AMBuildComboBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        AMBuildComboBox->setEnabled(false);
        AMBuildLayout = new QHBoxLayout(Dialog);

        cyclicSymmetryCheckBox = new QCheckBox(Dialog);
        cyclicSymmetryCheckBox->setMaximumWidth(20);
        cyclicSymmetryLabel = new QLabel(Dialog);
        cyclicSymmetryComboBox = new QComboBox(Dialog);
        cyclicSymmetryComboBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        cyclicSymmetryComboBox->setEnabled(false);
        cyclicSymmetryLayout = new QHBoxLayout(Dialog);

        xGridPatternCheckBox = new QCheckBox(Dialog);
        xGridPatternCheckBox->setMaximumWidth(20);
        xGridPatternLabel = new QLabel(Dialog);
        xGridPatternSpinBox = new QSpinBox(Dialog);
        xGridPatternSpinBox->setRange(2, 50);
        xGridPatternSpinBox->setSingleStep(1);
        xGridPatternSpinBox->setValue(2);
        xGridPatternSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        xGridPatternSpinBox->setEnabled(false);
        xGridPatternLayout = new QHBoxLayout(Dialog);

        yGridPatternCheckBox = new QCheckBox(Dialog);
        yGridPatternCheckBox->setMaximumWidth(20);
        yGridPatternLabel = new QLabel(Dialog);
        yGridPatternSpinBox = new QSpinBox(Dialog);
        yGridPatternSpinBox->setRange(2, 50);
        yGridPatternSpinBox->setSingleStep(1);
        yGridPatternSpinBox->setValue(2);
        yGridPatternSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        yGridPatternSpinBox->setEnabled(false);
        yGridPatternLayout = new QHBoxLayout(Dialog);

        zGridPatternCheckBox = new QCheckBox(Dialog);
        zGridPatternCheckBox->setMaximumWidth(20);
        zGridPatternLabel = new QLabel(Dialog);
        zGridPatternSpinBox = new QSpinBox(Dialog);
        zGridPatternSpinBox->setRange(2, 50);
        zGridPatternSpinBox->setSingleStep(1);
        zGridPatternSpinBox->setValue(2);
        zGridPatternSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        zGridPatternSpinBox->setEnabled(false);
        zGridPatternSpinBox->setValue(1);
        zGridPatternLayout = new QHBoxLayout(Dialog);

        minFeatureSizeCheckBox = new QCheckBox(Dialog);
        minFeatureSizeCheckBox->setMaximumWidth(20);
        minFeatureSizeLabel = new QLabel(Dialog);
        minFeatureSizeSpinBox = new QDoubleSpinBox(Dialog);
        minFeatureSizeSpinBox->setRange(1e-10,1e10);
        minFeatureSizeSpinBox->setSingleStep(0.001);
        minFeatureSizeSpinBox->setValue(0.001);
        minFeatureSizeSpinBox->setDecimals(6);
        minFeatureSizeSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        minFeatureSizeSpinBox->setEnabled(false);
        minFeatureSizeLayout = new QHBoxLayout(Dialog);

        maxFeatureSizeCheckBox = new QCheckBox(Dialog);
        maxFeatureSizeCheckBox->setMaximumWidth(20);
        maxFeatureSizeLabel = new QLabel(Dialog);
        maxFeatureSizeSpinBox = new QDoubleSpinBox(Dialog);
        maxFeatureSizeSpinBox->setRange(1e-10,1e10);
        maxFeatureSizeSpinBox->setSingleStep(0.01);
        maxFeatureSizeSpinBox->setValue(0.1);
        maxFeatureSizeSpinBox->setDecimals(6);
        maxFeatureSizeSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        maxFeatureSizeSpinBox->setEnabled(false);
        maxFeatureSizeLayout = new QHBoxLayout(Dialog);

        displacementLimitCheckBox = new QCheckBox(Dialog);
        displacementLimitCheckBox->setMaximumWidth(20);
        displacementLimitLabel = new QLabel(Dialog);
        displacementLimitSpinBox = new QDoubleSpinBox(Dialog);
        displacementLimitSpinBox->setRange(1e-12,1e12);
        displacementLimitSpinBox->setSingleStep(0.01);
        displacementLimitSpinBox->setValue(0.01);
        displacementLimitSpinBox->setDecimals(6);
        displacementLimitSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        displacementLimitSpinBox->setEnabled(false);
        displacementLimitLayout = new QHBoxLayout(Dialog);

        stressSafetyFactorCheckBox = new QCheckBox(Dialog);
        stressSafetyFactorCheckBox->setMaximumWidth(20);
        stressSafetyFactorLabel = new QLabel(Dialog);
        stressSafetyFactorSpinBox = new QDoubleSpinBox(Dialog);
        stressSafetyFactorSpinBox->setRange(1, 1000);
        stressSafetyFactorSpinBox->setSingleStep(0.1);
        stressSafetyFactorSpinBox->setValue(1.0);
        stressSafetyFactorSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        stressSafetyFactorSpinBox->setEnabled(false);
        stressSafetyFactorLayout = new QHBoxLayout(Dialog);

        modalLimitCheckBox = new QCheckBox(Dialog);
        modalLimitCheckBox->setMaximumWidth(20);
        modalLimitLabel = new QLabel(Dialog);
        modalLimitSpinBox = new QDoubleSpinBox(Dialog);
        modalLimitSpinBox->setRange(0,10000000);
        modalLimitSpinBox->setSingleStep(10);
        modalLimitSpinBox->setValue(1000);
        modalLimitSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        modalLimitSpinBox->setEnabled(false);
        modalLimitLayout = new QHBoxLayout(Dialog);


        temperatureLimitCheckBox = new QCheckBox(Dialog);
        temperatureLimitCheckBox->setMaximumWidth(20);
        temperatureLimitLabel = new QLabel(Dialog);
        temperatureLimitSpinBox = new QDoubleSpinBox(Dialog);
        temperatureLimitSpinBox->setRange(0,10000000);
        temperatureLimitSpinBox->setSingleStep(10);
        temperatureLimitSpinBox->setValue(2000);
        temperatureLimitSpinBox->setMaximumWidth(SPINBOX_MAX_WIDTH);
        temperatureLimitSpinBox->setEnabled(false);
        temperatureLimitLayout = new QHBoxLayout(Dialog);



        xSymmetryCheckBox =  new QCheckBox(Dialog);
        ySymmetryCheckBox =  new QCheckBox(Dialog);
        zSymmetryCheckBox =  new QCheckBox(Dialog);

        // fixed faces
        keepTopologyConnected =  new QCheckBox(Dialog);
        keepTopologyConnected->setChecked(true);

        // fixed faces
        keepFixedFacesCheckBox =  new QCheckBox(Dialog);

        // assign to corresponding layouts
        drawDirectionLayout->addWidget(drawDirectionCheckBox);
        drawDirectionLayout->addWidget(drawDirectionLabel);
        drawDirectionLayout->addWidget(drawDirectionComboBox);

        extrudeLayout->addWidget(extrudeCheckBox);
        extrudeLayout->addWidget(extrudeLabel);
        extrudeLayout->addWidget(extrudeComboBox);

        AMBuildLayout->addWidget(AMBuildCheckBox);
        AMBuildLayout->addWidget(AMBuildLabel);
        AMBuildLayout->addWidget(AMBuildComboBox);

        cyclicSymmetryLayout->addWidget(cyclicSymmetryCheckBox);
        cyclicSymmetryLayout->addWidget(cyclicSymmetryLabel);
        cyclicSymmetryLayout->addWidget(cyclicSymmetryComboBox);

        xGridPatternLayout->addWidget(xGridPatternCheckBox);
        xGridPatternLayout->addWidget(xGridPatternLabel);
        xGridPatternLayout->addWidget(xGridPatternSpinBox);

        yGridPatternLayout->addWidget(yGridPatternCheckBox);
        yGridPatternLayout->addWidget(yGridPatternLabel);
        yGridPatternLayout->addWidget(yGridPatternSpinBox);

        zGridPatternLayout->addWidget(zGridPatternCheckBox);
        zGridPatternLayout->addWidget(zGridPatternLabel);
        zGridPatternLayout->addWidget(zGridPatternSpinBox);

        minFeatureSizeLayout->addWidget(minFeatureSizeCheckBox);
        minFeatureSizeLayout->addWidget(minFeatureSizeLabel);
        minFeatureSizeLayout->addWidget(minFeatureSizeSpinBox);

        maxFeatureSizeLayout->addWidget(maxFeatureSizeCheckBox);
        maxFeatureSizeLayout->addWidget(maxFeatureSizeLabel);
        maxFeatureSizeLayout->addWidget(maxFeatureSizeSpinBox);

        displacementLimitLayout->addWidget(displacementLimitCheckBox);
        displacementLimitLayout->addWidget(displacementLimitLabel);
        displacementLimitLayout->addWidget(displacementLimitSpinBox);

        stressSafetyFactorLayout->addWidget(stressSafetyFactorCheckBox);
        stressSafetyFactorLayout->addWidget(stressSafetyFactorLabel);
        stressSafetyFactorLayout->addWidget(stressSafetyFactorSpinBox);

        modalLimitLayout->addWidget(modalLimitCheckBox);
        modalLimitLayout->addWidget(modalLimitLabel);
        modalLimitLayout->addWidget(modalLimitSpinBox);

        temperatureLimitLayout->addWidget(temperatureLimitCheckBox);
        temperatureLimitLayout->addWidget(temperatureLimitLabel);
        temperatureLimitLayout->addWidget(temperatureLimitSpinBox);

        // push to main
        mainLayout->addLayout(extrudeLayout);
        mainLayout->addLayout(AMBuildLayout);
        mainLayout->addLayout(drawDirectionLayout);     
        mainLayout->addLayout(cyclicSymmetryLayout);
        mainLayout->addLayout(xGridPatternLayout);
        mainLayout->addLayout(yGridPatternLayout);
        mainLayout->addLayout(zGridPatternLayout);    
        mainLayout->addLayout(stressSafetyFactorLayout);
        mainLayout->addLayout(displacementLimitLayout);
        mainLayout->addLayout(modalLimitLayout);
        mainLayout->addLayout(temperatureLimitLayout);

        mainLayout->addLayout(minFeatureSizeLayout);
        mainLayout->addLayout(maxFeatureSizeLayout);
        mainLayout->addWidget(xSymmetryCheckBox);
        mainLayout->addWidget(ySymmetryCheckBox);
        mainLayout->addWidget(zSymmetryCheckBox);


        mainLayout->addWidget(keepTopologyConnected);
        mainLayout->addWidget(keepFixedFacesCheckBox);

        applyPushButton = new QPushButton(Dialog);
        mainLayout->addWidget(applyPushButton);

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }
    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("ConstraintsDialog", "Constraints", Q_NULLPTR));

        drawDirectionLabel->setText(QApplication::translate("ConstraintsDialog", "DrawDirection", Q_NULLPTR));
        drawDirectionComboBox->clear();
        drawDirectionComboBox->insertItems(0, QStringList()
                                           << QApplication::translate("ConstraintsDialog", "XDir", Q_NULLPTR)
                                           << QApplication::translate("ConstraintsDialog", "YDir", Q_NULLPTR)
                                           << QApplication::translate("ConstraintsDialog", "ZDir", Q_NULLPTR)
                                           );

        extrudeLabel->setText(QApplication::translate("ConstraintsDialog", "Extrude", Q_NULLPTR));
        extrudeComboBox->clear();
        extrudeComboBox->insertItems(0, QStringList()
                                        << QApplication::translate("ConstraintsDialog", "XDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "YDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "ZDir", Q_NULLPTR)
                                        );

        AMBuildLabel->setText(QApplication::translate("ConstraintsDialog", "AM Build", Q_NULLPTR));
        AMBuildComboBox->clear();
        AMBuildComboBox->insertItems(0, QStringList()
                                        << QApplication::translate("ConstraintsDialog", "+XDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "-XDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "+YDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "-YDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "+ZDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "-ZDir", Q_NULLPTR)
                                        );

        cyclicSymmetryLabel->setText(QApplication::translate("ConstraintsDialog", "CyclicSym(Z)", Q_NULLPTR));
        cyclicSymmetryComboBox->clear();
        cyclicSymmetryComboBox->insertItems(0, QStringList() 
                                            << QApplication::translate("ConstraintsDialog", "(2)180 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(3)120 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(4)90 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(5)72 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(6)60 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(7)51 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(8)45 deg", Q_NULLPTR)
                                            );

        minFeatureSizeLabel->setText(QApplication::translate("ConstraintsDialog", "MinFeatureSize", Q_NULLPTR));
        maxFeatureSizeLabel->setText(QApplication::translate("ConstraintsDialog", "MaxFeatureSize", Q_NULLPTR));

        xGridPatternLabel->setText(QApplication::translate("ConstraintsDialog", "XGridPattern", Q_NULLPTR));
        yGridPatternLabel->setText(QApplication::translate("ConstraintsDialog", "YGridPattern", Q_NULLPTR));
        zGridPatternLabel->setText(QApplication::translate("ConstraintsDialog", "ZGridPattern", Q_NULLPTR));

        displacementLimitLabel->setText(QApplication::translate("ConstraintsDialog", "MaxDisp", Q_NULLPTR));
        stressSafetyFactorLabel->setText(QApplication::translate("ConstraintsDialog", "StressSafety", Q_NULLPTR));
        modalLimitLabel->setText(QApplication::translate("ConstraintsDialog", "MinFreq(Hz)", Q_NULLPTR));
        temperatureLimitLabel->setText(QApplication::translate("ConstraintsDialog", "MaxTemp(K)", Q_NULLPTR));
        xSymmetryCheckBox->setText(QApplication::translate("ConstraintsDialog", "X-Symmetry", Q_NULLPTR));
        ySymmetryCheckBox->setText(QApplication::translate("ConstraintsDialog", "Y-Symmetry", Q_NULLPTR));
        zSymmetryCheckBox->setText(QApplication::translate("ConstraintsDialog", "Z-Symmetry", Q_NULLPTR));
        keepTopologyConnected->setText(QApplication::translate("ConstraintsDialog", "Connected Topology", Q_NULLPTR));
        keepFixedFacesCheckBox->setText(QApplication::translate("ConstraintsDialog", "Keep Fixed Faces", Q_NULLPTR));
        applyPushButton->setText(QApplication::translate("ControlDialog", "Apply", Q_NULLPTR));

    }
};

QT_END_NAMESPACE


//create the member functions and slots in this class
class ConstraintsDialog : public QDialog, public Ui_constraintsDialog {
    Q_OBJECT

public:
    ConstraintsDialog( QWidget *parent = 0);
    ~ConstraintsDialog();
    void    setViewer (TopOptViewer *v) { viewer = v; }
signals:
    void setConstraintsComplete();
private slots:
    void updateCheckbox();
    void updateConstraints();
    void AMBuildMessage();
    void closeDialog();
    void showEvent(QShowEvent *e);

private:
    TopOptViewer *viewer;
    void makeConnections();
};
