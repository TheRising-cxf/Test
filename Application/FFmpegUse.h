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
#include <libavutil\time.h>
}
using namespace std;
static const double SYNC_THRESHOLD = 0.01;
static const double NOSYNC_THRESHOLD = 10.0;
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
	int return_Finish();
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
};