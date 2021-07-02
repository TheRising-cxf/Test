#pragma once
#include <qmainwindow.h>
#include <ui_dealVideo.h>
#include <opencv2\opencv.hpp>
#include <QTimer>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QThread>
#include <iostream>
#include "FFmpegUse.h"
using namespace std;
#pragma execution_character_set("utf-8")
using namespace cv;

class DealVideo :
	public QMainWindow
{
	Q_OBJECT

public:
	DealVideo(QWidget *parent = Q_NULLPTR);
	~DealVideo();
	void closeEvent(QCloseEvent *);
signals:
	void ExitWin();//新建一个信号槽
private:
	Ui::VideoWindow ui;
	VideoCapture cap;
	Mat frame;
	int cameraIndex = -1;
	int audioIndex = -1;
	ReadAV* readAV;
	ReadCap* readCap;
	ReadAudio* readAudio;
	int currenOperate;
private slots:
	void on_comboBox_currentIndexChanged();
	void on_display_clicked();
	void on_stop_clicked();
	void show_image_1(QImage dstImage);
};