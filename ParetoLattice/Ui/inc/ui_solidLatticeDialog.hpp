#ifndef UI_UI_LATTICERADII_H
#define UI_UI_LATTICERADII_H

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

class Ui_LatticeRadiiDialog
{
public:
    QVBoxLayout *mainLayout;

    QHBoxLayout *averageRadiusLayout;
    QLabel *averageRadiusLabel;
    QLineEdit *averageRadius;

    QHBoxLayout *xGradientLayout;
    QLabel *xGradientLabel;
    QSpinBox *xGradientSpinBox;

    QHBoxLayout *yGradientLayout;
    QLabel *yGradientLabel;
    QSpinBox *yGradientSpinBox;

    QHBoxLayout *zGradientLayout;
    QLabel *zGradientLabel;
    QSpinBox *zGradientSpinBox;

    QPushButton *createLatticePushButton1;
    QLabel *separatorText;

    QHBoxLayout *circularDivision1Layout;
    QLabel *circularDivision1Label;
    QSpinBox *circularDivision1SpinBox;

    QPushButton *loadLatticeRadiiPushButton;
    QHBoxLayout *circularDivision2Layout;
    QLabel *circularDivision2Label;
    QSpinBox *circularDivision2SpinBox;

    QPushButton *createLatticePushButton2;
    QPushButton *closePushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("LatticeStructureDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);

        averageRadiusLayout = new QHBoxLayout(Dialog);
        averageRadiusLabel = new QLabel(Dialog);
        averageRadius = new QLineEdit(Dialog);
        averageRadius->setText(QString::number(1.0));;
        averageRadiusLayout->addWidget(averageRadiusLabel);
        averageRadiusLayout->addWidget(averageRadius);

        xGradientLayout = new QHBoxLayout(Dialog);
        xGradientLabel = new QLabel;
        xGradientSpinBox = new QSpinBox;
        xGradientSpinBox->setRange(-100, 100);
        xGradientSpinBox->setSingleStep(5);
        xGradientSpinBox->setValue(0);
        xGradientLayout->addWidget(xGradientLabel);
        xGradientLayout->addWidget(xGradientSpinBox);

        yGradientLayout = new QHBoxLayout(Dialog);
        yGradientLabel = new QLabel;
        yGradientSpinBox = new QSpinBox;
        yGradientSpinBox->setRange(-100, 100);
        yGradientSpinBox->setSingleStep(5);
        yGradientSpinBox->setValue(0);
        yGradientLayout->addWidget(yGradientLabel);
        yGradientLayout->addWidget(yGradientSpinBox);

        zGradientLayout = new QHBoxLayout(Dialog);
        zGradientLabel = new QLabel;
        zGradientSpinBox = new QSpinBox;
        zGradientSpinBox->setRange(-100, 100);
        zGradientSpinBox->setSingleStep(5);
        zGradientSpinBox->setValue(0);
        zGradientLayout->addWidget(zGradientLabel);
        zGradientLayout->addWidget(zGradientSpinBox);

        circularDivision1Layout = new QHBoxLayout(Dialog);
        circularDivision1Label = new QLabel;
        circularDivision1SpinBox = new QSpinBox;
        circularDivision1SpinBox->setRange(3, 100);
        circularDivision1SpinBox->setSingleStep(1);
        circularDivision1SpinBox->setValue(8);
        circularDivision1Layout->addWidget(circularDivision1Label);
        circularDivision1Layout->addWidget(circularDivision1SpinBox);

        createLatticePushButton1 = new QPushButton(Dialog);
        separatorText = new QLabel;

        // buttons
        loadLatticeRadiiPushButton = new QPushButton(Dialog);
        circularDivision2Layout = new QHBoxLayout(Dialog);
        circularDivision2Label = new QLabel;
        circularDivision2SpinBox = new QSpinBox;
        circularDivision2SpinBox->setRange(3, 100);
        circularDivision2SpinBox->setSingleStep(1);
        circularDivision2SpinBox->setValue(8);
        circularDivision2Layout->addWidget(circularDivision2Label);
        circularDivision2Layout->addWidget(circularDivision2SpinBox);

        createLatticePushButton2 = new QPushButton(Dialog);

        closePushButton = new QPushButton(Dialog);
        closePushButton->hide();

        mainLayout->addLayout(averageRadiusLayout);
        mainLayout->addLayout(xGradientLayout);
        mainLayout->addLayout(yGradientLayout);
        mainLayout->addLayout(zGradientLayout);
        mainLayout->addLayout(circularDivision1Layout);
        mainLayout->addWidget(createLatticePushButton1);
        mainLayout->addWidget(separatorText);
        mainLayout->addLayout(circularDivision2Layout);
        mainLayout->addWidget(loadLatticeRadiiPushButton);
        mainLayout->addWidget(createLatticePushButton2);

        mainLayout->addWidget(closePushButton);

        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("LatticeStructureDialog", "Solid Lattice", Q_NULLPTR));
        averageRadiusLabel->setText(QApplication::translate("LatticeStructureDialog", "Average Radius", Q_NULLPTR));
        xGradientLabel->setText(QApplication::translate("LatticeStructureDialog", "X Gradient", Q_NULLPTR));
        yGradientLabel->setText(QApplication::translate("LatticeStructureDialog", "Y Gradient", Q_NULLPTR));
        zGradientLabel->setText(QApplication::translate("LatticeStructureDialog", "Z Gradient", Q_NULLPTR));
        circularDivision1Label->setText(QApplication::translate("LatticeStructureDialog", "Circular Division", Q_NULLPTR));
        createLatticePushButton1->setText(QApplication::translate("LatticeStructureDialog", "Create Solid Lattice", Q_NULLPTR));
        separatorText->setText(QApplication::translate("LatticeStructureDialog", "Or", Q_NULLPTR));
        loadLatticeRadiiPushButton->setText(QApplication::translate("LatticeStructureDialog", "Load Varying Radii (.txt)", Q_NULLPTR));
        circularDivision2Label->setText(QApplication::translate("LatticeStructureDialog", "Circular Division", Q_NULLPTR));
        createLatticePushButton2->setText(QApplication::translate("LatticeStructureDialog", "Create Solid Lattice", Q_NULLPTR));
        closePushButton->setText(QApplication::translate("LatticeStructureDialog", "Close", Q_NULLPTR));
    }
};
QT_END_NAMESPACE

class LatticeRadiiDialog : public QDialog, public Ui_LatticeRadiiDialog {
    Q_OBJECT
public:
    LatticeRadiiDialog( QWidget *parent = 0);
    ~LatticeRadiiDialog();
    void setViewer(TemplateViewer *v) { viewer = v;}
signals:

private slots:
    void createSolidLattice1();
    void createSolidLattice2();
    void closeLatticeRadiiDialog();
    void loadLatticeRadiiFileDialog();
    void showEvent(QShowEvent *e);
private:
    TemplateViewer *viewer;
    void makeConnections();
};
////////////////////////////////////////////////////////////////////////////////

#endif
