#include "ui_templateDialog.hpp"
#include "CoreFunctions.hpp"
#include <iostream>

extern LiveVariables liveVar;
extern ParetoApplication topopt;
extern Voxelizer vox;
extern CoreFunctions coreFunctions;

void TemplateDialog::closeDialog()
{

}

/*****************************************************************/
TemplateDialog::TemplateDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
TemplateDialog::~TemplateDialog()
{

}
///////////////////////////////////////////////////////////////////////////////
void TemplateDialog :: makeConnections()
{
    ;
}
