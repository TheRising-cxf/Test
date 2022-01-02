#include <iostream>
#include <QtWidgets/QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <qaudiooutput.h>
#include <SDL.h>
#include <SDL_thread.h>

#include "DealVideo.h"
#include "MainLoop.h"
#include "PacketQueue.h"
#include "Audio.h"
#include "Media.h"
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
	readDevices = new ReadDevices();
	connect(readAV, SIGNAL(return_QImage(QImage,double, bool)), this, SLOT(show_image_1(QImage,double,bool)));
	connect(readDevices, SIGNAL(return_QImage(QImage, double,bool)), this, SLOT(show_image_1(QImage, double,bool)));
	//connect(readDevices, SIGNAL(return_Finish()), this, SLOT(returnFinish()));
	connect(readAV, SIGNAL(return_Finish()), this, SLOT(returnPlayFinish()));
	connect(readAV, SIGNAL(return_videoTime(double)), this, SLOT(GetTime(double)));
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
}
void DealVideo::closeEvent(QCloseEvent *)
{
	g_mutex.lock();
	g_isExitThread = true;
	g_exitReadAV = true;
	g_mutex.unlock();
	readAV->quit();
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

		s = ui.audioList->currentText().toStdString();
		s = "audio=" + s;
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
		if (readAV->isRunning()) {
			g_mutex.lock();
			g_isExitThread = true;
			g_isStop = false;
			g_mutex.unlock();
		}
		else 
			readAV->start();
	}
}
char *dup_wchar_to_utf8(wchar_t *w)
{
	char *s = NULL;
	int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	s = (char *)av_malloc(l);
	if (s)
		WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
	return s;
}
void DealVideo::on_photo_clicked()
{
	if (ui.photo->text() == "打开摄像头") {
		std::string s = ui.comboBox->currentText().toStdString();
		readDevices->SetPath("");
		if (s != "") {
			s = "video=" + s;
			readDevices->m_InputStream.SetVideoCaptureDevice(s);//m_strVideoDevice
			readDevices->start();
			ui.photo->setText("关闭摄像头");
		}
	}
	else if(ui.photo->text() == "关闭摄像头"){
		ui.photo->setText("打开摄像头");
		readDevices->OnCloseThread();
		ui.recordVideo->setText("开始录制");
	}
}
void DealVideo::on_recordVideo_clicked()
{
	if (ui.recordVideo->text() == "开始录制") {
		std::string sV = ui.comboBox->currentText().toStdString();
		std::string sA= ui.audioList->currentText().toStdString();
		if (sV != "") {
			sV = "video=" + sV;
			readDevices->m_InputStream.SetVideoCaptureDevice(sV);//m_strVideoDevice
		}
		if (sA != "") {
			sA = "audio=" + sA;
			readDevices->m_InputStream.SetAudioCaptureDevice(sA);
		}
		if (sA != ""||sV != "") {
			time_t t = time(0);
			char tmp[64];
			strftime(tmp, sizeof(tmp), "%Y-%m-%d-%H_%M_%S.mp4", localtime(&t));
			readDevices->SetPath(tmp);
			readDevices->start();
			ui.recordVideo->setText("结束录制");
		}
	}
	else if(ui.recordVideo->text() == "结束录制"){
		ui.recordVideo->setText("开始录制");
		readDevices->OnCloseThread();
	}
}
void DealVideo::returnPlayFinish() {
	ui.playVideo->setText("播放");
}
void DealVideo::GetTime(double time) {
	m_s = (int)time % 60;
	m_m = (int)time / 60;
	QString str = QString("00:00/%1:%2").arg(m_m).arg(m_s);
	ui.timeLabel->setText(str);
	ui.horizontalSlider->setMaximum(int(time));
}
void DealVideo::show_image_1(QImage dstImage,double time,bool isFilp = false)
{
	QImage img = dstImage;
	float imageScale = 1.0f;
	imageScale = fmin((float)ui.label->width() / (float)img.width(), (float)ui.label->height() / (float)img.height());
	if (imageScale > 1)imageScale = 1;
	img = img.scaled(img.width()*imageScale, img.height()*imageScale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	img = img.mirrored(isFilp,false);
	ui.label->setPixmap(QPixmap::fromImage(img)); // 在label上播放视频图片
	int m = (int)time / 60;
	int s = (int)time % 60;
	ui.horizontalSlider->setValue((int)time);
	QString str = QString("%1:%2/%3:%4").arg(m).arg(s).arg(m_m).arg(m_s);
	ui.timeLabel->setText(str);
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
