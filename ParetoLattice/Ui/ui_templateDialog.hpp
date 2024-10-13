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
class Ui_templateDialog
{
public:
    QVBoxLayout *mainLayout;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

        mainLayout = new QVBoxLayout(Dialog);
        retranslateUi(Dialog);
        QMetaObject::connectSlotsByName(Dialog);
    }
    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("templateDialog", "Dialog", Q_NULLPTR));
    }
};

QT_END_NAMESPACE


//create the member functions and slots in this class
class TemplateDialog : public QDialog, public Ui_templateDialog {
    Q_OBJECT

public:
    TemplateDialog( QWidget *parent = 0);
    ~TemplateDialog();
    void    setViewer (TopOptViewer *v) { viewer = v; }

private slots:
    void closeDialog();

private:
    TopOptViewer *viewer;
    void makeConnections();
};
