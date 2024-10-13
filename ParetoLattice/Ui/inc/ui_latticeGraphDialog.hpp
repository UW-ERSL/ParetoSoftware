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
#include "LatticeViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_LatticeStructureDialog
{
public:
    QVBoxLayout *mainLayout;

    QPushButton *readLatticeGraphPushButton;

    QLabel *text;
    QPushButton *readSTLPushButton;

    QHBoxLayout *nUnitCellsLayout;
    QLabel *nUnitCellsLabel;
    QSpinBox *nUnitCellsSpinBox;


    QHBoxLayout *latticeTypeLayout;
    QLabel *latticeTypeLabel;
    QComboBox *latticeTypeCombo;
    QPushButton *createLatticePushButton;
    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("LatticeStructureDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);

        readLatticeGraphPushButton = new QPushButton(Dialog);

        text = new QLabel;

        readSTLPushButton = new QPushButton(Dialog);
        latticeTypeLayout = new QHBoxLayout(Dialog);
        latticeTypeLabel = new QLabel;
        latticeTypeCombo = new QComboBox;
        latticeTypeLayout->addWidget(latticeTypeLabel);
        latticeTypeLayout->addWidget(latticeTypeCombo);
        latticeTypeCombo->setEnabled(true);


        nUnitCellsLayout = new QHBoxLayout(Dialog);
        nUnitCellsLabel = new QLabel;
        nUnitCellsSpinBox = new QSpinBox;
        nUnitCellsSpinBox->setRange(1, 1000000);
        nUnitCellsSpinBox->setSingleStep(100);
        nUnitCellsSpinBox->setValue(1000);
        nUnitCellsLayout->addWidget(nUnitCellsLabel);
        nUnitCellsLayout->addWidget(nUnitCellsSpinBox);

        // buttons
        createLatticePushButton = new QPushButton(Dialog);


        mainLayout->addWidget(readLatticeGraphPushButton);
        mainLayout->addWidget(text);
        mainLayout->addWidget(readSTLPushButton);
        mainLayout->addLayout(latticeTypeLayout);
        mainLayout->addLayout(nUnitCellsLayout);
        mainLayout->addWidget(createLatticePushButton);

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("LatticeStructureDialog", "Lattice Graph", Q_NULLPTR));

        readLatticeGraphPushButton->setText(QApplication::translate("LatticeStructureDialog", "Load Lattice Graph", Q_NULLPTR));

        text->setText(QApplication::translate("LatticeStructureDialog", "Or", Q_NULLPTR));
        readSTLPushButton->setText(QApplication::translate("LatticeStructureDialog", "Load 3D STL geometry", Q_NULLPTR));
        nUnitCellsLabel->setText(QApplication::translate("LatticeStructureDialog", "#Unit Cells", Q_NULLPTR));
        latticeTypeLabel->setText(QApplication::translate("LatticeStructureDialog", "Lattice Type", Q_NULLPTR));
        latticeTypeCombo->clear();
        latticeTypeCombo->insertItems(0, QStringList()
                                      << QApplication::translate("LatticeStructureDialog", "Box", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "BCC", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "BCCz", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "BOXBCC", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "FCC", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "FCCz", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "FCCBCC", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "OCTET", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "PILLARS", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "NO_OVERHANG", Q_NULLPTR)
                                      << QApplication::translate("LatticeStructureDialog", "VORONOI", Q_NULLPTR)
                                      );

        createLatticePushButton->setText(QApplication::translate("LatticeStructureDialog", "Create Lattice Graph", Q_NULLPTR));

    }
};
QT_END_NAMESPACE

class LatticeStructureDialog : public QDialog, public Ui_LatticeStructureDialog {
    Q_OBJECT
public:
    LatticeStructureDialog( QWidget *parent = 0);
    ~LatticeStructureDialog();
    void setViewer(TemplateViewer *v) { viewer = v;}
signals:

private slots:
    void loadLatticeGraph();
    void readSTL();
    void createLatticeGraphFromVoxelMesh();
    void showEvent(QShowEvent *e);
private:
    TemplateViewer *viewer;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////

#endif
