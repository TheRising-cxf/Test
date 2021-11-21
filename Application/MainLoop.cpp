
#include "MainLoop.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <math.h>
#include "opencv2/opencv.hpp"
ReadDevices* gpMainFrame = NULL;
void gettimeofday(struct timeval* tv) {

	auto time_now = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	auto duration_in_s = std::chrono::duration_cast<std::chrono::seconds>(time_now.time_since_epoch()).count();
	auto duration_in_us = std::chrono::duration_cast<std::chrono::microseconds>(time_now.time_since_epoch()).count();

	tv->tv_sec = duration_in_s;
	tv->tv_usec = duration_in_us;
}
//�ɼ�������Ƶͼ��ص� CALLBACK
int VideoCaptureCallback(AVStream * input_st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp)
{
    gpMainFrame->m_OutputStream.write_video_frame(input_st, pix_fmt, pframe, lTimeStamp);
	QImage  Qimg((uchar*)gpMainFrame->m_OutputStream.pFrameRGB->data[0] , pframe->width, pframe->height, QImage::Format_RGB32);
	emit gpMainFrame->return_QImage(Qimg.copy(), 0.0);
	return 0;
}

//�ɼ�������Ƶ���ݻص� CALLBACK
int AudioCaptureCallback(AVStream * input_st, AVFrame *pframe, int64_t lTimeStamp)
{
    gpMainFrame->m_OutputStream.write_audio_frame(input_st, pframe, lTimeStamp);
    return 0;
}


ReadDevices::ReadDevices()
{
    gpMainFrame = this;
 
    m_nChannelNum = -1;
    m_nFPS = 0;

     /* register all codecs, demux and protocols */
    avcodec_register_all();
    av_register_all();

    avdevice_register_all();
}

ReadDevices::~ReadDevices()
{

}

int ReadDevices::OnStartStream(){
    m_InputStream.SetAudioCaptureCB(AudioCaptureCallback);
	m_InputStream.SetVideoCaptureCB(VideoCaptureCallback);
    bool bRet;

    bRet = m_InputStream.OpenInputStream(); //��ʼ���ɼ��豸
    if(!bRet)
    {
        printf("�򿪲ɼ��豸ʧ��\n");
        return 1;
    }

    int cx, cy, fps;
    AVPixelFormat pixel_fmt;
    //GetVideoInputInfo,��ȡ���capacity
	if (m_InputStream.GetVideoInputInfo(cx, cy, fps, pixel_fmt)) //��ȡ��Ƶ�ɼ�Դ����Ϣ
	{
		//cx:width cy:height  //CBR���̶����ʿ��ƣ�, VBR�Ƕ�̬����,  ƽ������ABR,
		//��Ƶ���������õ����ʿ��Ʒ�ʽ: abr(ƽ������)��crf���������ʣ���cqp���̶�������
		m_OutputStream.SetVideoCodecProp(AV_CODEC_ID_H264, fps, 500000/*bps*/, 30/*GOP*/, cx, cy); //������Ƶ����������
	}

    int sample_rate = 0, channels = 0;
    AVSampleFormat  sample_fmt;
    if(m_InputStream.GetAudioInputInfo(sample_fmt, sample_rate, channels)) //��ȡ��Ƶ�ɼ�Դ����Ϣ
    {
        m_OutputStream.SetAudioCodecProp(AV_CODEC_ID_AAC, sample_rate, channels, 32000); //������Ƶ����������
    }


    bRet  = m_OutputStream.OpenOutputStream(m_szFilePath); //�������·��
    if(!bRet)
    {
        printf("��ʼ�����ʧ��\n");
        return 1;
    }

    //��ʼ��ʱ
    gettimeofday(&p_start);

    bRet = m_InputStream.StartCapture(); //��ʼ�ɼ�

    m_frmCount = 0;
    m_nFPS = 0;

    return 0;
}

int ReadDevices:: OnStopStream()
{
    m_InputStream.CloseInputStream();
    m_OutputStream.CloseOutput();

    //������ʱ
    gettimeofday(&p_end);
    long timeuse = ((p_end.tv_sec - p_start.tv_sec)*1000) + ((p_end.tv_usec - p_start.tv_usec)/1000);
    printf("�ɼ���ʱ��%ld ms\n", timeuse);

    return 0;
}
void ReadDevices::run()
{

	gpMainFrame->OnStartStream();
	//�߳̽�����ִ��
	gpMainFrame->OnStopStream();

	return ;
}



