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
#include "MainLoop.h"
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
	float playSpeed = 1.0f;
	ReadAV* readAV;
	ReadDevices * readDevices;

	int currenOperate;
	int m_m, m_s;
private slots:
	void on_comboBox_currentIndexChanged();
	void on_display_clicked();
	void on_playVideo_clicked();
	void on_selectFile_clicked();
	void on_photo_clicked();
	void on_recordVideo_clicked();
	void returnPlayFinish();
	void GetTime(double time);
	void show_image_1(QImage dstImage, double time,bool isFilp);
	void show_image_1(QImage dstImage);
};