/********************************************************************************
** Form generated from reading UI file 'ui_StructuralAnalysisDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_FrequencyAnalysisDialog_H
#define UI_UI_FrequencyAnalysisDialog_H

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
#include <QtCharts>

#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QValueAxis>
#include <QDialog>



#include "QDialog"
#include "TopOptViewer.hpp"

QT_BEGIN_NAMESPACE

class Ui_FrequencyAnalysisDialog
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *qualityLayout;
    QHBoxLayout *elementsLayout;

    QLabel *elemQualityLabel;
    QComboBox *elemQualityCombo;

    QLabel *numElemLabel;
    QSpinBox *numElemsSpinBox;


    QHBoxLayout *alphaMLayout;
    QLabel *alphaMLabel;
    QDoubleSpinBox *alphaMSpinBox;

    QHBoxLayout *betaKLayout;
    QLabel *betaKLabel;
    QDoubleSpinBox *betaKSpinBox;


    QHBoxLayout *frequencyStartLayout;
    QLabel *frequencyStartLabel;
    QDoubleSpinBox *frequencyStartSpinBox;

    QHBoxLayout *frequencyEndLayout;
    QLabel *frequencyEndLabel;
    QDoubleSpinBox *frequencyEndSpinBox;


    QHBoxLayout *frequencyStepLayout;
    QLabel *frequencyStepLabel;
    QDoubleSpinBox *frequencyStepSpinBox;

    QCheckBox *nodeBasedCheckBox;
    QPushButton *frequencyResponseSolvePushButton;
    QPushButton *frequencyResponseStopPushButton;
    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("FreqAnalysisDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);


        // elem quality
        elemQualityLabel = new QLabel;
        elemQualityCombo = new QComboBox;

        // Number of elements
        numElemLabel = new QLabel;
        numElemsSpinBox = new QSpinBox;
        numElemsSpinBox->setRange(100, 100000000);
        numElemsSpinBox->setSingleStep(1000);
        numElemsSpinBox->setValue(10000);

        alphaMLabel = new QLabel;
        alphaMSpinBox = new QDoubleSpinBox;
        alphaMSpinBox->setRange(0, 0.1);
        alphaMSpinBox->setSingleStep(0.001);
        alphaMSpinBox->setValue(0.001);
        alphaMSpinBox->setDecimals(5);

        betaKLabel = new QLabel;
        betaKSpinBox = new QDoubleSpinBox;
        betaKSpinBox->setRange(0, 0.1);
        betaKSpinBox->setSingleStep(0.00001);
        betaKSpinBox->setValue(0.00001);
        betaKSpinBox->setDecimals(5);


        frequencyStartLabel = new QLabel;
        frequencyStartSpinBox = new QDoubleSpinBox;
        frequencyStartSpinBox->setRange(0.01, 1000000);
        frequencyStartSpinBox->setSingleStep(25);
        frequencyStartSpinBox->setValue(100);
        frequencyStartSpinBox->setDecimals(1);

        frequencyEndLabel = new QLabel;
        frequencyEndSpinBox = new QDoubleSpinBox;
        frequencyEndSpinBox->setRange(0.01, 1000000);
        frequencyEndSpinBox->setSingleStep(25);
        frequencyEndSpinBox->setValue(350);
        frequencyEndSpinBox->setDecimals(1);


        frequencyStepLabel = new QLabel;
        frequencyStepSpinBox = new QDoubleSpinBox;
        frequencyStepSpinBox->setRange(0.01, 1000);
        frequencyStepSpinBox->setSingleStep(5);
        frequencyStepSpinBox->setValue(25);
        frequencyStepSpinBox->setDecimals(1);

        nodeBasedCheckBox = new QCheckBox(Dialog);
        frequencyResponseSolvePushButton = new QPushButton(Dialog);
        frequencyResponseStopPushButton = new QPushButton(Dialog);

        // Layouts
        mainLayout = new QVBoxLayout(Dialog);
        qualityLayout = new QHBoxLayout(Dialog);
        elementsLayout = new QHBoxLayout(Dialog);
        alphaMLayout= new QHBoxLayout(Dialog);
        betaKLayout= new QHBoxLayout(Dialog);

        frequencyStartLayout= new QHBoxLayout(Dialog);
        frequencyEndLayout= new QHBoxLayout(Dialog);
        frequencyStepLayout= new QHBoxLayout(Dialog);

        qualityLayout->addWidget(elemQualityLabel);
        qualityLayout->addWidget(elemQualityCombo);

        elementsLayout->addWidget(numElemLabel);
        elementsLayout->addWidget(numElemsSpinBox);

        alphaMLayout->addWidget(alphaMLabel);
        alphaMLayout->addWidget(alphaMSpinBox);
        betaKLayout->addWidget(betaKLabel);
        betaKLayout->addWidget(betaKSpinBox);

        frequencyStartLayout->addWidget(frequencyStartLabel);
        frequencyStartLayout->addWidget(frequencyStartSpinBox);
        frequencyEndLayout->addWidget(frequencyEndLabel);
        frequencyEndLayout->addWidget(frequencyEndSpinBox);
        frequencyStepLayout->addWidget(frequencyStepLabel);
        frequencyStepLayout->addWidget(frequencyStepSpinBox);


        mainLayout->addLayout(qualityLayout);
        mainLayout->addLayout(elementsLayout);
        mainLayout->addLayout(alphaMLayout);
        mainLayout->addLayout(betaKLayout);
        mainLayout->addLayout(frequencyStartLayout);
        mainLayout->addLayout(frequencyEndLayout);
        mainLayout->addLayout(frequencyStepLayout);
        mainLayout->addWidget(nodeBasedCheckBox);
        mainLayout->addWidget(frequencyResponseSolvePushButton);
        mainLayout->addWidget(frequencyResponseStopPushButton);

        nodeBasedCheckBox->hide();
        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("FreqAnalysisDialog", "Frequency Analysis", Q_NULLPTR));

        elemQualityLabel->setText(QApplication::translate("FreqAnalysisDialog", "Mesh Quality", Q_NULLPTR));
        elemQualityCombo->clear();
        elemQualityCombo->insertItems(0, QStringList()
                                      << QApplication::translate("FreqAnalysisDialog", "Very Coarse", Q_NULLPTR)
                                      << QApplication::translate("FreqAnalysisDialog", "Coarse", Q_NULLPTR)
                                      << QApplication::translate("FreqAnalysisDialog", "Medium", Q_NULLPTR)
                                      << QApplication::translate("FreqAnalysisDialog", "Fine", Q_NULLPTR)
                                      << QApplication::translate("FreqAnalysisDialog", "Very Fine", Q_NULLPTR)
                                      );

        numElemLabel->setText(QApplication::translate("FreqAnalysisDialog", "#Elements", Q_NULLPTR));
        alphaMLabel->setText(QApplication::translate("FreqAnalysisDialog", "alpha-M", Q_NULLPTR));
        betaKLabel->setText(QApplication::translate("FreqAnalysisDialog", "beta-K", Q_NULLPTR));
        frequencyStartLabel->setText(QApplication::translate("FreqAnalysisDialog", "Start (Hz)", Q_NULLPTR));
        frequencyEndLabel->setText(QApplication::translate("FreqAnalysisDialog", "End (Hz)", Q_NULLPTR));
        frequencyStepLabel->setText(QApplication::translate("FreqAnalysisDialog", "Step (Hz)", Q_NULLPTR));

        nodeBasedCheckBox->setText(QApplication::translate("FreqAnalysisDialog", "Node Based? ", Q_NULLPTR));
        frequencyResponseSolvePushButton->setText(QApplication::translate("FreqAnalysisDialog", "Run Frequency Response", Q_NULLPTR));
        frequencyResponseStopPushButton->setText(QApplication::translate("FreqAnalysisDialog", "STOP!", Q_NULLPTR));

    } // retranslateUi
};


QT_END_NAMESPACE

class FrequencyAnalysisDialog : public QDialog, public Ui_FrequencyAnalysisDialog {
    Q_OBJECT
public:
    FrequencyAnalysisDialog( QWidget *parent = 0);
    ~FrequencyAnalysisDialog();
    void setViewer(TopOptViewer *v) { viewer = v;}
signals:


private slots:
    void solveFrequencyResponse();
    void plotFrequencyResponse();
    void stopFrequencyResponse();
    void plotData(int nSamples, double *xData,double *yData,
                  const char *Title, const char *XLabel, const char *YLabel,QMainWindow &dataWindow);
    void onFEAFinished();
    void showEvent(QShowEvent *e);
    void closeFrequencyAnalysisDialog();
    void updateLoadSetCallBack(void);
    void updateLiveVariables(void);
    void updateNumberOfElements(void);
private:
    TopOptViewer *viewer;
    void makeConnections();
    int meshElementsMap[5];
    QMainWindow deltaPlotWindow;
    QMainWindow phasePlotWindow;
};
////////////////////////////////////////////////////////////////////////////////

#endif // UI_UI_FrequencyAnalysisDialog_H
