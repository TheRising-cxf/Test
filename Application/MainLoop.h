//pts: （int64_t）显示时间，结合AVStream->time_base转换成时间戳
//dts: （int64_t）解码时间，结合AVStream->time_base转换成时间戳

#ifndef MAINLOOP_H
#define MAINLOOP_H
#include <Windows.h>
#include <QThread>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include "AVInputStream.h"
#include "AVOutputStream.h"
extern "C"
{
#include <libavdevice/avdevice.h>
}
using namespace std;

#include<chrono>    //头文件包含

void gettimeofday(struct timeval* tv);
class ReadDevices :public QThread
{
	Q_OBJECT
public:
	ReadDevices();
    virtual ~ReadDevices();

public:
    void   CalculateFPS();
    bool   IsPreview() { return m_bPreview; }

    int OnStartStream();
    int OnStopStream();
	int OnCloseThread();
	void SetPath(string name);
    void OnDestroy();
	void run();
	bool             m_isSave;
protected:
    long              m_nChannelNum; //通道号
    uint64_t         m_frmCount;
    int              m_nFPS;
    bool             m_bPreview;
    string           m_szFilePath;

    timeval p_start;
    timeval p_end;

public:
    CAVInputStream    m_InputStream;
    CAVOutputStream   m_OutputStream;
signals:
	void return_QImage(QImage dstImage, double time,bool isFilp);
	void return_Finish();

};


#endif // MAINLOOP_H
