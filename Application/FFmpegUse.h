#pragma once
#include <opencv2\opencv.hpp>
#include <QTimer>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QThread>
// 调用FFmpeg的头文件
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
using namespace std;
class ReadAV :public QThread {

	Q_OBJECT
public:
	ReadAV() {};
	~ReadAV();
	string filePath;
	void setFilePath(string s);
	void run();
signals:
	int return_QImage(QImage dstImage);
};
class ReadCap :public QThread {

	Q_OBJECT
public:
	ReadCap() {};
	~ReadCap();
	string capName;
	void setCapName(string s);
	void run();
signals:
	int return_QImage(QImage dstImage);
};

class ReadAudio :public QThread {

	Q_OBJECT
public:
	ReadAudio() {};
	~ReadAudio();
	string audioName;
	void setAudioName(string s);
	void run();
	//signals:
	//	int return_QImage(QImage dstImage);
};