#ifndef UI_UI_LATTICESTRUCTURE_H
#define UI_UI_LATTICESTRUCTURE_H

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
#include "AMBuildViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_LatticeStructureDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *latticeTypeLayout;
    QHBoxLayout *unitSizeLayout;
    QHBoxLayout *edgeDivisionLayout;
    QHBoxLayout *fillRatioLayout;
    QHBoxLayout *criticalSurfaceLayout;
    QHBoxLayout *smootheningLayout;

    QLabel *latticeTypeLabel;
    QComboBox *latticeTypeCombo;

    QLabel *unitSizeLabel;
    QSpinBox *unitSizeSpinBox;

    QLabel *edgeDivisionLabel;
    QSpinBox *edgeDivisionSpinBox;

    QLabel *fillRatioLabel;
    QDoubleSpinBox *fillRatioSpinBox;

    QLabel *criticalSurfaceLabel;
    QComboBox *criticalSurfaceCombo;

    QLabel *smootheningLabel;
    QSpinBox *smootheningSpinBox;

    QPushButton *createLatticePushButton;
    QPushButton *stopLatticeDesignPushButton;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("LatticeStructureDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);
        latticeTypeLayout = new QHBoxLayout(Dialog);
        unitSizeLayout = new QHBoxLayout(Dialog);
        edgeDivisionLayout = new QHBoxLayout(Dialog);
        fillRatioLayout = new QHBoxLayout(Dialog);
        criticalSurfaceLayout = new QHBoxLayout(Dialog);
        smootheningLayout = new QHBoxLayout(Dialog);



        // lattice type
        latticeTypeLabel = new QLabel;
        latticeTypeCombo = new QComboBox;
        latticeTypeLayout->addWidget(latticeTypeLabel);
        latticeTypeLayout->addWidget(latticeTypeCombo);

        // unit Size
        unitSizeLabel = new QLabel;
        unitSizeSpinBox = new QSpinBox;
        unitSizeSpinBox->setRange(1, 100);
        unitSizeSpinBox->setSingleStep(1);
        unitSizeSpinBox->setValue(5);
        unitSizeLayout->addWidget(unitSizeLabel);
        unitSizeLayout->addWidget(unitSizeSpinBox);

        // edge Division
        edgeDivisionLabel = new QLabel;
        edgeDivisionSpinBox = new QSpinBox;
        edgeDivisionSpinBox->setRange(4, 100);
        edgeDivisionSpinBox->setSingleStep(1);
        edgeDivisionSpinBox->setValue(10);
        edgeDivisionLayout->addWidget(edgeDivisionLabel);
        edgeDivisionLayout->addWidget(edgeDivisionSpinBox);

        // fill ratio
        fillRatioLabel = new QLabel;
        fillRatioSpinBox = new QDoubleSpinBox;
        fillRatioSpinBox->setRange(0.0, 1.0);
        fillRatioSpinBox->setSingleStep(0.05);
        fillRatioSpinBox->setValue(0.5);
        fillRatioLayout->addWidget(fillRatioLabel);
        fillRatioLayout->addWidget(fillRatioSpinBox);

        // critical surface
        criticalSurfaceLabel = new QLabel;
        criticalSurfaceCombo = new QComboBox;
        criticalSurfaceLayout->addWidget(criticalSurfaceLabel);
        criticalSurfaceLayout->addWidget(criticalSurfaceCombo);


        // smoothening
        smootheningLabel = new QLabel;
        smootheningSpinBox = new QSpinBox;
        smootheningSpinBox->setRange(1, 10);
        smootheningSpinBox->setSingleStep(1);
        smootheningSpinBox->setValue(2);
        smootheningLayout->addWidget(smootheningLabel);
        smootheningLayout->addWidget(smootheningSpinBox);

        // buttons
        createLatticePushButton = new QPushButton(Dialog);
        stopLatticeDesignPushButton = new QPushButton(Dialog);
        closePushButton = new QPushButton(Dialog);

        //add layouts
         mainLayout->addLayout(latticeTypeLayout);
         mainLayout->addLayout(unitSizeLayout);
         mainLayout->addLayout(edgeDivisionLayout);
         mainLayout->addLayout(fillRatioLayout);
         mainLayout->addLayout(criticalSurfaceLayout);
         mainLayout->addLayout(smootheningLayout);

        mainLayout->addWidget(createLatticePushButton);
        mainLayout->addWidget(stopLatticeDesignPushButton);
        mainLayout->addWidget(closePushButton);

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("LatticeStructureDialog", "Lattice Design", Q_NULLPTR));

        latticeTypeLabel->setText(QApplication::translate("LatticeStructureDialog", "Lattice Type", Q_NULLPTR));
        latticeTypeCombo->clear();

        latticeTypeCombo->insertItems(0, QStringList()
                                      << QApplication::translate("LatticeStructureDialog", "Box", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "BCC*", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "BCCz*", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "BOXBCC", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "FCC*", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "FCCz*", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "FCCBCC*", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "OCTET", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "C11", Q_NULLPTR)
                                      );

        unitSizeLabel->setText(QApplication::translate("LatticeStructureDialog", "Unit Size (mm)", Q_NULLPTR));
        edgeDivisionLabel->setText(QApplication::translate("LatticeStructureDialog", "Edge Division", Q_NULLPTR));
        fillRatioLabel->setText(QApplication::translate("LatticeStructureDialog", "Fill Ratio", Q_NULLPTR));

        criticalSurfaceLabel->setText(QApplication::translate("LatticeStructureDialog", "Critical Surface", Q_NULLPTR));
        criticalSurfaceCombo->clear();
        criticalSurfaceCombo->insertItems(0, QStringList()
                                      << QApplication::translate("LatticeStructureDialog", "None", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "Critical", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "All", Q_NULLPTR)
                                      );

        smootheningLabel->setText(QApplication::translate("LatticeStructureDialog", "Smoothening", Q_NULLPTR));
        createLatticePushButton->setText(QApplication::translate("LatticeStructureDialog", "Create Lattice", Q_NULLPTR));
        stopLatticeDesignPushButton->setText(QApplication::translate("LatticeStructureDialog", "STOP Design", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("LatticeStructureDialog", "Close", Q_NULLPTR));
    }

};


QT_END_NAMESPACE

class LatticeStructureDialog : public QDialog, public Ui_LatticeStructureDialog {
    Q_OBJECT
public:
    LatticeStructureDialog( QWidget *parent = 0);
    ~LatticeStructureDialog();
    void setViewer(AMBuildViewer *v) { viewer = v;}
signals:


private slots:
    void createLatticeDesign();
    void closeLatticeStructureDialog();
    void showEvent(QShowEvent *e);
    void updateLiveVariables(void);
    void stopLatticeStructureDesign(void);
private:
    AMBuildViewer *viewer;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////

#endif
