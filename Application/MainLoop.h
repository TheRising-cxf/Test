//pts: ��int64_t����ʾʱ�䣬���AVStream->time_baseת����ʱ���
//dts: ��int64_t������ʱ�䣬���AVStream->time_baseת����ʱ���

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

#include<chrono>    //ͷ�ļ�����

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

    void OnDestroy();
	void run();
protected:
    long              m_nChannelNum; //ͨ����

    uint64_t              m_frmCount;
    int              m_nFPS;
    bool              m_bPreview;
    const char* m_szFilePath = "myout.flv";

    timeval p_start;
    timeval p_end;

public:
    CAVInputStream    m_InputStream;
    CAVOutputStream   m_OutputStream;
signals:
	void return_QImage(QImage dstImage, double time);

};


#endif // MAINLOOP_H
