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
#include <iostream>
using namespace std;
extern int  g_stopAudio;
extern int  g_isFinish;//正常播放完退出
extern bool g_isStop;//暂停播放
extern bool g_isExitThread;//直接退出
extern bool g_exitReadAV;
extern std::mutex g_mutex;
DealVideo::DealVideo(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
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
	connect(readAV, SIGNAL(return_Finish()), this, SLOT(returnPlayFinish()));
	connect(readCap, SIGNAL(return_QImage(QImage)), this, SLOT(show_image_1(QImage)));
	g_stopAudio = 0;
	g_isFinish = 0;
	g_isStop = false;
	g_isExitThread = false;
	g_exitReadAV = false;
}
DealVideo::~DealVideo()
{
	g_mutex.lock();
	g_isExitThread = true;
	g_exitReadAV = true;
	g_mutex.unlock();
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
	g_mutex.lock();
	g_isExitThread = true;
	g_exitReadAV = true;
	g_mutex.unlock();
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

void DealVideo::on_playVideo_clicked()
{
	if (ui.playVideo->text() == "暂停") {
		ui.playVideo->setText("播放");
		g_mutex.lock();
		g_isStop = true;
		g_mutex.unlock();
	}
	else if (ui.playVideo->text() == "播放") {
		if (readAV->filePath != "") {
			g_mutex.lock();
			g_isExitThread = false;
			g_isStop = false;
			g_stopAudio = 0;
			g_mutex.unlock();
			ui.playVideo->setText("暂停");
			readAV->start();
		}
	}
}

void DealVideo::on_selectFile_clicked()
{
	QString vedioPath = QFileDialog::getOpenFileName(this,
		tr("选择视频！"),
		"F:",
		tr("视频文件(*.*)"));
	if (vedioPath != NULL) {
		ui.playVideo->setText("播放");
		std::string s = vedioPath.toStdString();
		readAV->setFilePath(s);
		g_mutex.lock();
			g_isExitThread = true;
			g_isStop = true;
		g_mutex.unlock();
	}
}
void DealVideo::returnPlayFinish() {
	ui.playVideo->setText("播放");
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
