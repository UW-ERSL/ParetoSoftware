#include "ui_toolsDialog.hpp"
#include "CoreFunctions.hpp"



/***************************************************/
ToolsDialog::~ToolsDialog()
{

}
/***************************************************/
ToolsDialog :: ToolsDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
void ToolsDialog::captureScreenShot()
{
    static QString lastSelectedDirectory;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    *new QString("Select file name "),
                                                    lastSelectedDirectory,
                                                    *new QString( "Image (*.png)"));
    QImage screenGrab = viewer->grabFrameBuffer();
    int w = viewer->width();
    int h = viewer->height();
    screenGrab.scaled(w,h,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    screenGrab.save(fileName);
}
/***************************************************/
void ToolsDialog::closeDialog()
{
    parentWidget()->show();
    close();
}
/***************************************************/
void ToolsDialog::captureScreenVideo(double captureFrequency )
{
    //DEBUG THIS !
    if(captureFrequency < 0)
    {

        QString fileName;
        if(fileName.isEmpty())
        {
            return;
        }
        QString videoFrameName;
        for(int i = 0 ; i < 20; i++)
        {

            videoFrameName =  QString("_") +  QString::number(i) ;
            cout<<videoFrameName.toStdString() <<" : videoFrameName"<<endl;
            /* cout<<videoFrameName.toStdString()<<endl;
            QImage screenGrab = viewer->grabFrameBuffer();
            int w = viewer->width();
            int h = viewer->height();
            screenGrab.scaled(w,h,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            screenGrab.save(videoFrameName);*/
            //std::this_thread::sleep_for (std::chrono::seconds(1));
        }

    }

}

/***************************************************/
void ToolsDialog::makeConnections(){

    PushButton( captureVideoPushButton, [=]{ captureScreenVideo(-1);});
    PushButton( saveScreenShotPushButton, [=]{ captureScreenShot();});
    PushButton( closePushButton, [=]{ closeDialog();});

}
