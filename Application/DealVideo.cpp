#include "DealVideo.h"
#include <QtWidgets/QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <qaudiooutput.h>
#include <SDL.h>
#include <SDL_thread.h>

#include "PacketQueue.h"
#include "Audio.h"
#include "Media.h"
#include "VideoDisplay.h"
#include <iostream>
using namespace std;

bool Quit = false;
DealVideo::DealVideo(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.comboBox->addItem("视频输入");
	QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
	foreach(const QCameraInfo &cameraInfo, cameras) {
		ui.comboBox->addItem(cameraInfo.description());
	}
	QList<QAudioDeviceInfo> audios = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	foreach(const QAudioDeviceInfo &audiosInfo, audios) {
		ui.audioList->addItem(audiosInfo.deviceName());
	}
	readAV = new ReadAV();
	readCap = new ReadCap();
	readAudio = new ReadAudio();
	connect(readAV, SIGNAL(return_QImage(QImage)), this, SLOT(show_image_1(QImage)));
	connect(readCap, SIGNAL(return_QImage(QImage)), this, SLOT(show_image_1(QImage)));
}
DealVideo::~DealVideo()
{
	cap.release();
	if (readAV != NULL) {
		delete readAV;
	}
	if (readCap != NULL) {
		delete readCap;
	}
	if (readAudio != NULL) {
		delete readAudio;
	}
}
void DealVideo::closeEvent(QCloseEvent *)
{
	readAV->quit();
	readCap->quit();
	readAudio->quit();
	emit ExitWin();
}
void DealVideo::on_comboBox_currentIndexChanged() {
	cameraIndex = ui.comboBox->currentIndex()-1;

}
void DealVideo::on_display_clicked()
{
	if (cameraIndex == -1)
	{
		QString vedioPath = QFileDialog::getOpenFileName(this,
			tr("选择视频！"),
			"F:",
			tr("视频文件(*.*)"));

		std::string s = vedioPath.toStdString();
		readAV->setFilePath(s);
		readAV->start();
	}
	else
	{
		std::string s = ui.comboBox->currentText().toStdString();
		s = "video=" + s;
		printf("%s\n", s);
		readCap->setCapName(s);
		readCap->start();
		s = ui.audioList->currentText().toStdString();
		s = "audio=" + s;
		readAudio->setAudioName(s);
		readAudio->start();
	}
}

void DealVideo::on_stop_clicked()
{
	if (ui.stop->text() == "暂停") {
		ui.stop->setText("继续");
	}
	else {
		ui.stop->setText("暂停");
	}
}

void DealVideo::show_image_1(QImage dstImage)
{
	QImage img = dstImage;
	float imageScale = 1.0f;
	imageScale = fmin((float)ui.label->width() / (float)img.width(), (float)ui.label->height() / (float)img.height());
	if (imageScale > 1)imageScale = 1;
	img = img.scaled(img.width()*imageScale, img.height()*imageScale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label->setPixmap(QPixmap::fromImage(img)); // 在label上播放视频图片
}
