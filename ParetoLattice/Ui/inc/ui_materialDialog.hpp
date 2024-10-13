/********************************************************************************
** Form generated from reading UI file 'ui_materialDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UI_MATERIALDIALOG_H
#define UI_UI_MATERIALDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <QDialog>
#include <QColorDialog>
#include "LatticeViewer.hpp"
#include "MaterialHandler.hpp"

QT_BEGIN_NAMESPACE

extern MaterialHandler materialHandler;
class Ui_MaterialDialog
{
public:

    QVBoxLayout *mainLayout;
    QLabel *materialLabel;

    QHBoxLayout *materialComboLayout;
    QHBoxLayout *(propertyLayout[MH_NUM_MATERIAL_PROPERTIES]);
    QLabel *(propertyLabel[MH_NUM_MATERIAL_PROPERTIES]);
    QLineEdit *(propertyLineEdit[MH_NUM_MATERIAL_PROPERTIES]);

    QHBoxLayout *applyMaterialLayout;

    QComboBox *materialComboBox;
    QCheckBox *optimizeCheckBox;
    QPushButton *applyPushButton;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("MaterialDialog"));

        Dialog->setMinimumWidth(GUI_MIN_WIDTH);

        mainLayout = new QVBoxLayout(Dialog);

        materialComboLayout = new  QHBoxLayout(Dialog);
        materialLabel = new QLabel(Dialog);
        materialComboBox = new QComboBox(Dialog);
        materialComboLayout->addWidget(materialLabel);
        materialComboLayout->addWidget(materialComboBox);

        mainLayout->addLayout(materialComboLayout);
        string propertyName;
        MaterialProperties matProp;
        materialHandler.getPropertyValues(0,matProp);
        for (int i = 0; i < MH_NUM_MATERIAL_PROPERTIES; i++){
            propertyLayout[i] = new QHBoxLayout(Dialog);
            propertyLabel[i] = new QLabel(Dialog);
            propertyLineEdit[i] = new QLineEdit(Dialog);
            propertyLabel[i]->setMinimumWidth(150);
            propertyLineEdit[i]->setMaximumWidth(120);
            propertyLineEdit[i]->setText(QString::number(matProp[i]));
            materialHandler.getPropertyName(i,propertyName);
            propertyLabel[i]->setText(QApplication::translate("MaterialDialog", propertyName.c_str(), Q_NULLPTR));

            propertyLayout[i]->addWidget(propertyLabel[i]);
            propertyLayout[i]->addWidget(propertyLineEdit[i]);

            //propertyLineEdit[i]->setDisabled(true);
            mainLayout->addLayout(propertyLayout[i]);
        }


        applyMaterialLayout = new  QHBoxLayout(Dialog);
        optimizeCheckBox = new QCheckBox(Dialog);
        applyPushButton = new QPushButton(Dialog);
        applyMaterialLayout->addWidget(optimizeCheckBox);
        applyMaterialLayout->addWidget(applyPushButton);

        mainLayout->addLayout(applyMaterialLayout);
        retranslateUi(Dialog);

        materialComboBox->clear();
        string materialName;
        for (int i = 0; i < materialHandler.getNumOfMaterials(); i++){
            materialHandler.getMaterialName(i,materialName);
            materialComboBox->addItem(materialName.c_str());
        }
        //materialComboBox->addItem("New material");
        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("MaterialDialog", "Material", Q_NULLPTR));

        materialLabel->setText(QApplication::translate("MaterialDialog", "Material", Q_NULLPTR));


        optimizeCheckBox->setText(QApplication::translate("MaterialDialog", "Do not optimize", Q_NULLPTR));
        applyPushButton->setText(QApplication::translate("MaterialDialog", "Apply", Q_NULLPTR));
    } // retranslateUi

};

QT_END_NAMESPACE


class MaterialDialog : public QDialog, public Ui_MaterialDialog {
    Q_OBJECT

public:
    MaterialDialog( QWidget *parent = 0);
    ~MaterialDialog();
     void updatePropertyLineEdits();
    void setViewer (TemplateViewer *v) { viewer = v; }
     void updateMaterialLabelText();

private slots:
   void loadMaterialDialog();
   void applyMaterialDialog();
   void closeDialog();
   void updateComboBox();
signals:
   void setMaterialComplete();

private:
   virtual void showEvent(QShowEvent * event);
   TemplateViewer *viewer;
   void makeConnections();

};
////////////////////////////////////////////////////////////////////////////////

#endif // UI_UI_MATERIALDIALOG_H
