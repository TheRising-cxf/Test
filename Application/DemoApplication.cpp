#include "DemoApplication.h"
DemoApplication::DemoApplication(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	connect(ui.dealImage, &QPushButton::clicked, this, &DemoApplication::DealImageClick);
	connect(ui.dealAudio, &QPushButton::clicked, this, &DemoApplication::DealAudioClick);
	connect(ui.dealVideo, &QPushButton::clicked, this, &DemoApplication::DealVideoClick);
	connect(ui.dealGeo,   &QPushButton::clicked, this, &DemoApplication::DealGeo);
	connect(ui.dealOther, &QPushButton::clicked, this, &DemoApplication::DealOther);
	connect(ui.dealConnect, &QPushButton::clicked, this, &DemoApplication::DealConnect);
}

void DemoApplication::DealImageClick()
{
	this->hide();
	dealImage.show();
	connect(&dealImage, SIGNAL(ExitWin()), this, SLOT(show()));
}
void DemoApplication::DealAudioClick()
{
	this->hide();
	dealAudio.show();
	connect(&dealAudio, SIGNAL(ExitWin()), this, SLOT(show()));
}
void DemoApplication::DealVideoClick()
{
	this->hide();
	dealVideo.show();
	connect(&dealVideo, SIGNAL(ExitWin()), this, SLOT(show()));
}
void DemoApplication::DealGeo()
{
	this->hide();
	dealGeo.show();
	connect(&dealGeo, SIGNAL(ExitWin()), this, SLOT(show()));
}
void DemoApplication::DealOther()
{
	this->hide();
	dealOther.show();
	connect(&dealOther, SIGNAL(ExitWin()), this, SLOT(show()));
}

void DemoApplication::DealConnect()
{
	this->hide();
	dealConnect.show();
	connect(&dealConnect, SIGNAL(ExitWin()), this, SLOT(show()));
}
