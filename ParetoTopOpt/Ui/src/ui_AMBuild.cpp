#include "ui_AMBuild.hpp"
#include "CoreFunctions.hpp"

extern CoreFunctions           coreFunctions;

/************************************************/
AMBuildDialog::AMBuildDialog( QWidget *parent) : QDialog(parent)
{


    setupUi(this);
    makeConnections();

}

/************************************************/
AMBuildDialog::~AMBuildDialog()
{

}
/************************************************/
void AMBuildDialog::cellSelected(int nRow, int nCol)
{

    QTableWidgetItem *itab = m_pTableWidget->item(nRow,1);// read build direction
    QString itabtext = itab->text();
    cout << itabtext.toStdString() << endl;
}
/************************************************/
void AMBuildDialog::identifyBuildDirections()
{
    int nBuildDirections;
    std::vector<Point3d> directions;
    double dirTolerance,creaseAngleInDegrees;

    creaseAngleInDegrees = 5.0;
    dirTolerance = 0.01;
    nBuildDirections = vox.segmentModelAndFindCanonicalDirections(creaseAngleInDegrees,dirTolerance,directions);

    nBuildDirections = directions.size();
    m_pTableWidget = new QTableWidget(this);
    m_pTableWidget->setRowCount(nBuildDirections);
    m_pTableWidget->setColumnCount(4);
    m_TableHeader<<"#"<<"Direction"<<"Est. build time (min)" <<"Est. support vol. (cm^3)";
    m_pTableWidget->setMinimumHeight(10);
    m_pTableWidget->setHorizontalHeaderLabels(m_TableHeader);
    m_pTableWidget->verticalHeader()->setVisible(false);
    m_pTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pTableWidget->setShowGrid(true);
    m_pTableWidget->setStyleSheet("QTableView {selection-background-color: red;}");

    m_pTableWidget->horizontalHeader()->setStyleSheet("color: blue");
    for (int i = 0; i < nBuildDirections; i++){
        char dirStr[256],buildNumStr[10];
        Point3d dir;
        dir = directions.at(i);
        sprintf(buildNumStr,"%d",i);
        sprintf(dirStr,"(%.2f, %.2f, %.2f)",dir.x,dir.y,dir.z);
        m_pTableWidget->setItem(i, 0, new QTableWidgetItem(buildNumStr));
        m_pTableWidget->setItem(i, 1, new QTableWidgetItem(dirStr));

    }
    m_pTableWidget->resizeColumnsToContents();
    m_pTableWidget->resizeRowsToContents();

    m_pTableWidget->setGeometry(QApplication::desktop()->screenGeometry());
    m_pTableWidget->setVisible(true);


    connect( m_pTableWidget, SIGNAL( cellDoubleClicked (int, int) ),this, SLOT( cellSelected( int, int ) ) );

    updateDisplay();

}
/************************************************/
void AMBuildDialog::identifyOverhang()
{
    // call the public function to identify the overhang triangles
    //int overhangAngle = overhangAngleLineEdit->text().toDouble();
    //paretoApps.identifyOverhangTriangles(overhangAngle);
    updateDisplay();

}
/************************************************/
void AMBuildDialog::updateDisplay()
{
    viewer->updateGL();
}

/************************************************/
void AMBuildDialog::closeDialog()
{
    parentWidget()->show();
    close();

}

/************************************************/
void AMBuildDialog::makeConnections()
{
    //PushButton(identifyOverhangTrianglesPushButton,[=]{identifyOverhang();});
    PushButton(identifyBuildDirectionsPushButton,[=]{identifyBuildDirections();});
    PushButton(closePushButton,[=]{closeDialog();});
}
/************************************************/
