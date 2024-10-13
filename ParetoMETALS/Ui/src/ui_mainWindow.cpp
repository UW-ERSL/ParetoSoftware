#include "ui_mainWindow.hpp"
#include <QNetworkInterface>
#include "MacAddresses.txt"
using namespace std;

TopOptMainWindow * thisMainWindow = NULL;

#define MAX_ADDRESSES  10


/****************************************************************************/
bool getYesNoResponse(const char format[],...)
{
    va_list(args);
    char tempStr[1024*16];
    va_start(args, format);
    vsprintf(tempStr,format,args);
    va_end (args);


    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(thisMainWindow, "LatticeParams", tempStr,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        return true;
    else
        return false;
}
/****************************************************************************/
void writeWarningToLog(const char format[],...)
{
    va_list(args);
    char tempStr[1024*16];
    va_start(args, format);
    vsprintf(tempStr,format,args);
    va_end (args);

    if (thisMainWindow == NULL) return;
    thisMainWindow->logMessageBox->setTextColor(QColor(255,0,0));
    thisMainWindow->logMessageBox->append(tempStr);

    QMessageBox msgBox;
    msgBox.setText(tempStr);
    msgBox.setMinimumSize(GUI_MIN_WIDTH,20);
    msgBox.exec();

    if (liveVar.logFilePtr == NULL)return;

    fprintf(liveVar.logFilePtr,"%s \n", tempStr);
    fflush(liveVar.logFilePtr);

}
/****************************************************************************/
void writeWarningToGUIWindow(const char format[],...)
{
    va_list(args);
    char tempStr[1024*16];
    va_start(args, format);
    vsprintf(tempStr,format,args);
    va_end (args);

    if (thisMainWindow == NULL) return;
    thisMainWindow->logMessageBox->setTextColor(QColor(255,0,0));
    thisMainWindow->logMessageBox->append(tempStr);
    QMessageBox msgBox;
    msgBox.setText(tempStr);
    msgBox.setMinimumSize(GUI_MIN_WIDTH,20);
    msgBox.exec();
}
/****************************************************************************/
void writeMessageToGUIWindow(const char format[],...)
{
    va_list(args);
    char tempStr[1024*16];
    va_start(args, format);
    vsprintf(tempStr,format,args);
    va_end (args);

    if (thisMainWindow == NULL) return;

    thisMainWindow->logMessageBox->setTextColor(QColor(0,0,0));
    thisMainWindow->logMessageBox->append(tempStr);

    if (liveVar.logFilePtr == NULL)return;

    fprintf(liveVar.logFilePtr,"%s \n", tempStr);
    fflush(liveVar.logFilePtr);
}
/****************************************************************************/
void writeMessageToLog(const char format[],...)
{
    va_list(args);
    char tempStr[1024*16];
    va_start(args, format);
    vsprintf(tempStr,format,args);
    va_end (args);


    if (thisMainWindow == NULL) return;

    thisMainWindow->logMessageBox->setTextColor(QColor(0,0,0));
    thisMainWindow->logMessageBox->append(tempStr);

    if (liveVar.logFilePtr == NULL)return;

    fprintf(liveVar.logFilePtr,"%s \n", tempStr);
    fflush(liveVar.logFilePtr);
}
/*********************************************************************************/
void writeSettingsToLog(const char format[],...)
{
    // write project settings to log in green
    va_list(args);
    char tempStr[1024*16];
    va_start(args, format);
    vsprintf(tempStr,format,args);
    va_end (args);

    if (thisMainWindow == NULL) return;

    thisMainWindow->logMessageBox->setTextColor(QColor(150,0,255));
    thisMainWindow->logMessageBox->append(tempStr);

}
/*********************************************************************************/
void setStatusMessage(const char str[])
{
    thisMainWindow->statusBar()->showMessage(QString(str));
}
/*********************************************************************************/
void resetProgressBar(const char title[])
{
    setStatusMessage(title);
    thisMainWindow->progressBar->setValue(0);
    thisMainWindow->viewer->updateGL();
    QCoreApplication::processEvents();
}
/*********************************************************************************/
void clearUIBuffer(void)
{
    QCoreApplication::processEvents();
    thisMainWindow->viewer->updateGL();
}
/*********************************************************************************/
void updateProgressBar(const char title[], int percentComplete)
{
    setStatusMessage(title);
    percentComplete = MAX(MIN(percentComplete,100),0);
    thisMainWindow->progressBar->setValue(percentComplete);
    thisMainWindow->viewer->updateGL();
    QCoreApplication::processEvents();
}
/**********************************************************************/
void getDateMonthYear(int dateMonthYear[3])
{
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    dateMonthYear[2] = (now->tm_year + 1900);
    dateMonthYear[1] = (now->tm_mon + 1);
    dateMonthYear[0] =  now->tm_mday;

}
/**********************************************************************/
bool checkTemporaryLicense(char temporaryLicense[])
{
    bool tempValid;
    int currentDateMonthYear[3];

    getDateMonthYear(currentDateMonthYear);


    tempValid = false;
    if (currentDateMonthYear[2] < PARETO_TEMP_EXPIRY[2])
        tempValid = true;
    else if (currentDateMonthYear[2] > PARETO_TEMP_EXPIRY[2])
        ;
    else { // year matches, check month
        if (currentDateMonthYear[1] < PARETO_TEMP_EXPIRY[1])
            tempValid = true;
        else if (currentDateMonthYear[1] > PARETO_TEMP_EXPIRY[1])
            ;
        else {// year and month match, check date
            if (currentDateMonthYear[0] < PARETO_TEMP_EXPIRY[0]){
                tempValid = true;
                writeMessageToLog("Your Pareto license will expire in %d days.\nContact support@sciartsoft.com for extension \n",PARETO_TEMP_EXPIRY[0]-currentDateMonthYear[0]);
            }
            else if  (currentDateMonthYear[0] > PARETO_TEMP_EXPIRY[0])
                ;
        }
    }
    if (!tempValid){
        writeWarningToLog("Your Pareto license has expired.\nContact support@sciartsoft.com for extension \n");
        return false;
    }

    if (tempValid) {
        writeWarningToLog("Your temporary license is valid until %d/%d/%d.\nContact support@sciartsoft.com for extension ",(PARETO_TEMP_EXPIRY[0]),(PARETO_TEMP_EXPIRY[1]),(PARETO_TEMP_EXPIRY[2]));
        return true;
    }
    writeWarningToLog("Invalid temporary License.\nContact support@sciartsoft.com");
    return false;
}

/**********************************************************************/
bool checkUserMacAddress(char macAddress[256])
{
    int compareSerial;
    int nMacAddresses;
    int expiryDateMonthYear[3],currentDateMonthYear[3];
    getDateMonthYear(currentDateMonthYear);

    for (int j = 0; j < NUM_PARETO_USERS; j++) {
        compareSerial = strncmp(macAddress,allowedMacAddresses[j],12);
        sscanf(&(allowedMacAddresses[j][13]),"%d %d %d \n",&(expiryDateMonthYear[0]),&(expiryDateMonthYear[1]),&(expiryDateMonthYear[2]));// expiry date
        if (compareSerial == 0) {
            if (currentDateMonthYear[2] < expiryDateMonthYear[2])
                compareSerial = 0;// OK
            else if (currentDateMonthYear[2] > expiryDateMonthYear[2])
                compareSerial = 1;// failed
            else { // year matches, check month
                if (currentDateMonthYear[1] < expiryDateMonthYear[1])
                    compareSerial = 0;// OK
                else if (currentDateMonthYear[1] > expiryDateMonthYear[1])
                    compareSerial = 1;// failed
                else {// year and month match, check date
                    if (currentDateMonthYear[0] < expiryDateMonthYear[0]){
                        compareSerial = 0;// OK
                        writeWarningToLog("Your Pareto license will expiry in %d days.\nContact support@sciartsoft.com \n",expiryDateMonthYear[0]-currentDateMonthYear[0]);
                    }
                    else if  (currentDateMonthYear[0] > expiryDateMonthYear[0])
                        compareSerial = 1;// failed
                }
            }
            if (compareSerial == 1)
                writeWarningToLog("Your Pareto license has expired.\nContact support@sciartsoft.com\n");
        }
        if (compareSerial == 0) return true;
    }

    return false;
}
/****************************************************************************/
int getMacAddress(char macAddress[MAX_ADDRESSES][256])
{
    int nMacAddresses = 0;

    foreach(QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        // Return only the first non-loopback MAC Address
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack)){
            QString qstr = netInterface.hardwareAddress();
            std::string str = qstr.toUtf8().constData();
            if (!str.empty()){
                str.erase(std::remove(str.begin(), str.end(), ':'), str.end());
                strcpy(macAddress[nMacAddresses],str.c_str());
                nMacAddresses++;
            }
        }
    }
    return nMacAddresses;
}
/****************************************************************************/
TopOptMainWindow :: TopOptMainWindow( QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);

    openControlDialog();
    makeConnections();

    thisMainWindow = this;

    writeMessageToLog("Pareto Version %.2f",PARETO_VERSION);
    writeMessageToLog("Pareto Build Date %.2f",PARETO_BUILD_DATE);

    char macAddresses[MAX_ADDRESSES][256];
    int nAddresses;
    bool validUser,validTempUser;
    char tempLicense[256];

    nAddresses = getMacAddress(macAddresses);

    validUser = false;
    validTempUser = false;
    for (int i = 0; i < nAddresses; i++){
        if (checkUserMacAddress(macAddresses[i])){
            writeMessageToLog("Valid MAC Address: %s",macAddresses[i]);
            validUser = true;
            break;
        }
    }
    if (!validUser) {// permanent license not valid
        writeWarningToLog("MAC Address %s:  not recognized.\nContact support@sciartsoft.com.\nUse Control-C to copy this text content.",macAddresses[0]);
        validUser = checkTemporaryLicense(tempLicense);
        if (!validUser) {
            exit(0);
        }
    }
    setAcceptDrops(true);
    if (ACADEMIC_LICENSE)
        writeWarningToLog("This is an academic license, and should not be used for commercial purposes.");
}
/****************************************************************************/
TopOptMainWindow :: ~TopOptMainWindow()
{
}
/****************************************************************************/
void TopOptMainWindow :: resizeEvent( QResizeEvent *)
{
    int w = centralwidget->width();
    int h = centralwidget->height();
    viewer->resize(w,h);
}
/****************************************************************************/
void TopOptMainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString fileNameQt = urls.first().toLocalFile();
    if (fileNameQt.isEmpty())
        return;


    string fileName = fileNameQt.toUtf8().constData();
    if (fileName.empty()){
        return;
    }
    // check for .stl and .off
    const char *fileNameStr = fileName.c_str();
    if ( (strstr(fileNameStr,".off") != NULL) || (strstr(fileNameStr,".stl") != NULL)
             || (strstr(fileNameStr,".STL") != NULL)  ){
        liveVar.geomFileName[liveVar.nSTLFilesLoaded] = fileName;
        bool successfulLoad = coreFunctions.loadGeom(liveVar.assemblyMode);
        if(successfulLoad) {
            liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
            liveVar.GEOMETRY_UPLOADED = true;
        }
    }
    else if (strstr(fileNameStr,".prj") != NULL){
        liveVar.prjFileName = fileName;
        coreFunctions.loadPRJ("prj");
    }
    else {
        writeWarningToLog("File format not recognized; acceptable extensions: .off or .stl, or .prj");
        return;
    }

    viewer->updateGL();

}
/****************************************************************************/
void TopOptMainWindow::dragEnterEvent(QDragEnterEvent *event)
{

    event->acceptProposedAction();
}
/****************************************************************************/
void TopOptMainWindow :: closeEvent( QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Pareto",
                                                                tr("Are you sure you want to exit Pareto?\n"),
                                                                QMessageBox::Cancel | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
         liveVar.stopOptimization = true;
        event->accept();
    }
}
/****************************************************************************/

void TopOptMainWindow :: openControlDialog()
{
    if( nullptr == controlDialog )
        controlDialog = new ControlDialog( this );
    controlDialog->setViewer( viewer );
    controlDialog->show();
    dockWidget2->setWidget(controlDialog);
    dockWidget2->setVisible(true);

}
/******************************************************************************/
void TopOptMainWindow::closeMainWindow()
{
    QMessageBox msgBox;
    QString exitString = QString("Are you sure you want to exit?");
    msgBox.setText(exitString);
    QPushButton *exitButton = msgBox.addButton(tr("Exit "), QMessageBox::ActionRole);
    QPushButton *abortExitButton = msgBox.addButton(tr("Cancel  "), QMessageBox::ActionRole);
    msgBox.exec();
    if(msgBox.clickedButton() == exitButton)
        close();
    else if (msgBox.clickedButton() == abortExitButton)
        ;
}
/****************************************************************************/
void TopOptMainWindow::openLogMessageBox()
{
    dockWidget->setWidget(logMessageBox);
    dockWidget->setVisible(true);
}

/****************************************************************************/
void TopOptMainWindow :: makeConnections()
{
    connect( actionControl,  SIGNAL(triggered() ), this, SLOT( openControlDialog() ) );
    connect( actionExit,  SIGNAL(triggered() ), this, SLOT( closeMainWindow() ) );
    connect(actionLogMessage, SIGNAL(triggered()), SLOT(openLogMessageBox()));
}
/****************************************************************************/
