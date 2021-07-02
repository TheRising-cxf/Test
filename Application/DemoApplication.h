#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DemoApplication.h"
#include "DealAudio.h"
#include "DealImage.h"
#include "DealGeo.h"
#include "DealVideo.h"
#include "DealOther.h"
#include "DealConnect.h"
class DemoApplication : public QMainWindow
{
    Q_OBJECT

public:
    DemoApplication(QWidget *parent = Q_NULLPTR);

private:
    Ui::DemoApplicationClass ui;
	DealAuido dealAudio;
	DealImage dealImage;
	DealGeo dealGeo;
	DealVideo dealVideo;
	DealOther dealOther;
	DealConnect dealConnect;

private slots:
	void DealImageClick();
	void DealAudioClick();
	void DealVideoClick();
	void DealGeo();
	void DealOther();
	void DealConnect();
};
