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

#include "ParetoMETALSViewer.hpp"

QT_BEGIN_NAMESPACE

//create all widgets in this class
class Ui_constraintsDialog
{
public:
    QVBoxLayout *mainLayout;

    // we will need a layout, a label and a widget for each control

    //draw direction
    QHBoxLayout *drawDirectionLayout;
    QLabel *drawDirectionLabel;
    QComboBox *drawDirectionComboBox;

    //through cut
    QHBoxLayout *throughCutLayout;
    QLabel *throughCutLabel;
    QComboBox *throughCutComboBox;

    //cyclic Symmetry
    QHBoxLayout *cyclicSymmetryLayout;
    QLabel *cyclicSymmetryLabel;
    QComboBox *cyclicSymmetryComboBox;


    //minimum feature size
    QHBoxLayout *relativeMinFeatureSizeLayout;
    QLabel *relativeMinFeatureSizeLabel;
    QSpinBox *relativeMinFeatureSizeSpinBox;

    //Stress Safety Factor
    QHBoxLayout *stressSafetyFactorLayout;
    QLabel *stressSafetyFactorLabel;
    QDoubleSpinBox *stressSafetyFactorSpinBox;

    //displacement limit
    QHBoxLayout *displacementLimitLayout;
    QLabel *displacementLimitLabel;
    QDoubleSpinBox *displacementLimit;

    //modal limit
    QHBoxLayout *modalLimitLayout;
    QLabel *modalLimitLabel;
    QDoubleSpinBox  *modalLimit;


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

        drawDirectionLabel = new QLabel(Dialog);
        drawDirectionComboBox = new QComboBox(Dialog);
        drawDirectionLayout = new QHBoxLayout(Dialog);

        throughCutLabel = new QLabel(Dialog);
        throughCutComboBox = new QComboBox(Dialog);
        throughCutLayout = new QHBoxLayout(Dialog);

        cyclicSymmetryLabel = new QLabel(Dialog);
        cyclicSymmetryComboBox = new QComboBox(Dialog);
        cyclicSymmetryLayout = new QHBoxLayout(Dialog);

        relativeMinFeatureSizeLabel = new QLabel(Dialog);
        relativeMinFeatureSizeSpinBox = new QSpinBox(Dialog);
        relativeMinFeatureSizeSpinBox->setRange(1, 5);
        relativeMinFeatureSizeSpinBox->setSingleStep(1);
        relativeMinFeatureSizeSpinBox->setValue(1);
        relativeMinFeatureSizeLayout = new QHBoxLayout(Dialog);

        displacementLimitLabel = new QLabel(Dialog);
        displacementLimit = new QDoubleSpinBox(Dialog);
        displacementLimit->setRange(1e-12,1e12);
        displacementLimit->setSingleStep(0.01);
        displacementLimit->setValue(0.01);
        displacementLimit->setDecimals(6);
        displacementLimitLayout = new QHBoxLayout(Dialog);

        stressSafetyFactorLabel = new QLabel(Dialog);
        stressSafetyFactorSpinBox = new QDoubleSpinBox(Dialog);
        stressSafetyFactorSpinBox->setRange(1, 100);
        stressSafetyFactorSpinBox->setSingleStep(0.1);
        stressSafetyFactorSpinBox->setValue(1.0);
        stressSafetyFactorLayout = new QHBoxLayout(Dialog);

        modalLimitLabel = new QLabel(Dialog);
        modalLimit = new QDoubleSpinBox(Dialog);
        modalLimit->setRange(-1,10000000);
        modalLimit->setSingleStep(10);
        modalLimit->setValue(-1);
        modalLimitLayout = new QHBoxLayout(Dialog);


        // fixed faces
        keepFixedFacesCheckBox =  new QCheckBox(Dialog);

        // assign to corresponding layouts
        drawDirectionLayout->addWidget(drawDirectionLabel);
        drawDirectionLayout->addWidget(drawDirectionComboBox);

        throughCutLayout->addWidget(throughCutLabel);
        throughCutLayout->addWidget(throughCutComboBox);

        cyclicSymmetryLayout->addWidget(cyclicSymmetryLabel);
        cyclicSymmetryLayout->addWidget(cyclicSymmetryComboBox);

        relativeMinFeatureSizeLayout->addWidget(relativeMinFeatureSizeLabel);
        relativeMinFeatureSizeLayout->addWidget(relativeMinFeatureSizeSpinBox);

        displacementLimitLayout->addWidget(displacementLimitLabel);
        displacementLimitLayout->addWidget(displacementLimit);

        stressSafetyFactorLayout->addWidget(stressSafetyFactorLabel);
        stressSafetyFactorLayout->addWidget(stressSafetyFactorSpinBox);

        modalLimitLayout->addWidget(modalLimitLabel);
        modalLimitLayout->addWidget(modalLimit);

        // push to main
        mainLayout->addLayout(drawDirectionLayout);
        mainLayout->addLayout(throughCutLayout);
        mainLayout->addLayout(cyclicSymmetryLayout);
        mainLayout->addLayout(relativeMinFeatureSizeLayout);
        mainLayout->addLayout(stressSafetyFactorLayout);
        mainLayout->addLayout(displacementLimitLayout);
        mainLayout->addLayout(modalLimitLayout);


        mainLayout->addWidget(keepFixedFacesCheckBox);

        applyPushButton = new QPushButton(Dialog);
        mainLayout->addWidget(applyPushButton);

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }
    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("ConstraintsDialog", "TopOpt Constraints", Q_NULLPTR));

        drawDirectionLabel->setText(QApplication::translate("ConstraintsDialog", "Draw Direction", Q_NULLPTR));
        drawDirectionComboBox->clear();
        drawDirectionComboBox->insertItems(0, QStringList()
                                           << QApplication::translate("ConstraintsDialog", "None", Q_NULLPTR)
                                           << QApplication::translate("ConstraintsDialog", "XDir", Q_NULLPTR)
                                           << QApplication::translate("ConstraintsDialog", "YDir", Q_NULLPTR)
                                           << QApplication::translate("ConstraintsDialog", "ZDir", Q_NULLPTR)
                                           );

        throughCutLabel->setText(QApplication::translate("ConstraintsDialog", "Through Cut", Q_NULLPTR));
        throughCutComboBox->clear();
        throughCutComboBox->insertItems(0, QStringList()
                                        << QApplication::translate("ConstraintsDialog", "None", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "XDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "YDir", Q_NULLPTR)
                                        << QApplication::translate("ConstraintsDialog", "ZDir", Q_NULLPTR)
                                        );

        cyclicSymmetryLabel->setText(QApplication::translate("ConstraintsDialog", "Cyclic Sym (Z)", Q_NULLPTR));
        cyclicSymmetryComboBox->clear();
        cyclicSymmetryComboBox->insertItems(0, QStringList()
                                            << QApplication::translate("ConstraintsDialog", "None", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(3) 120 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(4) 90 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(5) 72 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(6) 60 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(7) 51 deg", Q_NULLPTR)
                                            << QApplication::translate("ConstraintsDialog", "(8) 45 deg", Q_NULLPTR)
                                            );

        relativeMinFeatureSizeLabel->setText(QApplication::translate("ConstraintsDialog", "RelMinFeatSize", Q_NULLPTR));

        displacementLimitLabel->setText(QApplication::translate("ConstraintsDialog", "Upperlimit Disp", Q_NULLPTR));

        stressSafetyFactorLabel->setText(QApplication::translate("ConstraintsDialog", "Stress Safety Factor", Q_NULLPTR));

        modalLimitLabel->setText(QApplication::translate("ConstraintsDialog", "Lowerlimit 1st Mode (Hz)", Q_NULLPTR));

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
    void    setViewer (ParetoMETALSViewer *v) { viewer = v; }
signals:
    void setConstraintsComplete();
private slots:
    void updateConstraints();
    void closeDialog();
    void showEvent(QShowEvent *e);

private:
    ParetoMETALSViewer *viewer;
    void makeConnections();
};
