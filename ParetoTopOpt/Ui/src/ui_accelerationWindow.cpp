#include "ui_accelerationWindow.hpp"

#include "CoreFunctions.hpp"

/////////////////////////////////////////////////////
AccelerationDialog::AccelerationDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/////////////////////////////////////////////////////
void AccelerationDialog::updateText()
{
    string accUnits;
    coreFunctions.getPropertyUnits(PROPERTY_ACCELERATION,accUnits);

    xAccelLabel->setText("X (" + QString::fromStdString(accUnits) +"):");
    yAccelLabel->setText("Y (" + QString::fromStdString(accUnits) +"):");
    zAccelLabel->setText("Z (" + QString::fromStdString(accUnits) +"):");
}
/*****************************************************/
AccelerationDialog::~AccelerationDialog()
{
updateText();
}
/////////////////////////////////////////////////////

void AccelerationDialog::setAcceleration()
{
    double scale;
    xaccLineEdit->setValidator(new QDoubleValidator(this));
    yaccLineEdit->setValidator(new QDoubleValidator(this));
    zaccLineEdit->setValidator(new QDoubleValidator(this));

    updateText();


    liveVar.XAcceleration = xaccLineEdit->text().toDouble();
    liveVar.YAcceleration = yaccLineEdit->text().toDouble();
    liveVar.ZAcceleration = zaccLineEdit->text().toDouble();


    liveVar.ElasticityAcceleration[liveVar.currentBCSet][0] = liveVar.XAcceleration;
    liveVar.ElasticityAcceleration[liveVar.currentBCSet][1] = liveVar.YAcceleration;
    liveVar.ElasticityAcceleration[liveVar.currentBCSet][2] = liveVar.ZAcceleration;
    liveVar.ElasticityAcceleration[liveVar.currentBCSet][3] = liveVar.radiansPerSecZAxis;

    viewer->update();
}
///////////////////////////////////////////////////////////////////////////////
void AccelerationDialog :: closeDialog()
{
    emit setAccelerationComplete();
    parentWidget()->show();
    close();
}

////////////////////////////////////////////////////////////
void AccelerationDialog::applyAcceleration()
{
    updateText();
    setAcceleration();
    liveVar.ACCELERATION_APPLIED = true;
    closeDialog();
}
////////////////////////////////////////////////////////////
void AccelerationDialog::makeConnections()
{
    updateText();
    LineEdit(xaccLineEdit,[=]{setAcceleration();});
    LineEdit(yaccLineEdit,[=]{setAcceleration();});
    LineEdit(zaccLineEdit,[=]{setAcceleration();});
    PushButton(applyPushButton,[=]{applyAcceleration();});
    PushButton(closePushButton,[=]{closeDialog();});

}

