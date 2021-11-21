
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
//采集到的视频图像回调 CALLBACK
int VideoCaptureCallback(AVStream * input_st, enum AVPixelFormat pix_fmt, AVFrame *pframe, int64_t lTimeStamp)
{
    gpMainFrame->m_OutputStream.write_video_frame(input_st, pix_fmt, pframe, lTimeStamp);
	QImage  Qimg((uchar*)gpMainFrame->m_OutputStream.pFrameRGB->data[0] , pframe->width, pframe->height, QImage::Format_RGB32);
	emit gpMainFrame->return_QImage(Qimg.copy(), 0.0);
	return 0;
}

//采集到的音频数据回调 CALLBACK
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

    bRet = m_InputStream.OpenInputStream(); //初始化采集设备
    if(!bRet)
    {
        printf("打开采集设备失败\n");
        return 1;
    }

    int cx, cy, fps;
    AVPixelFormat pixel_fmt;
    //GetVideoInputInfo,获取相机capacity
	if (m_InputStream.GetVideoInputInfo(cx, cy, fps, pixel_fmt)) //获取视频采集源的信息
	{
		//cx:width cy:height  //CBR（固定码率控制）, VBR是动态码率,  平均码率ABR,
		//视频编码器常用的码率控制方式: abr(平均码率)，crf（限制码率），cqp（固定质量）
		m_OutputStream.SetVideoCodecProp(AV_CODEC_ID_H264, fps, 500000/*bps*/, 30/*GOP*/, cx, cy); //设置视频编码器属性
	}

    int sample_rate = 0, channels = 0;
    AVSampleFormat  sample_fmt;
    if(m_InputStream.GetAudioInputInfo(sample_fmt, sample_rate, channels)) //获取音频采集源的信息
    {
        m_OutputStream.SetAudioCodecProp(AV_CODEC_ID_AAC, sample_rate, channels, 32000); //设置音频编码器属性
    }


    bRet  = m_OutputStream.OpenOutputStream(m_szFilePath); //设置输出路径
    if(!bRet)
    {
        printf("初始化输出失败\n");
        return 1;
    }

    //开始计时
    gettimeofday(&p_start);

    bRet = m_InputStream.StartCapture(); //开始采集

    m_frmCount = 0;
    m_nFPS = 0;

    return 0;
}

int ReadDevices:: OnStopStream()
{
    m_InputStream.CloseInputStream();
    m_OutputStream.CloseOutput();

    //结束计时
    gettimeofday(&p_end);
    long timeuse = ((p_end.tv_sec - p_start.tv_sec)*1000) + ((p_end.tv_usec - p_start.tv_usec)/1000);
    printf("采集用时：%ld ms\n", timeuse);

    return 0;
}
void ReadDevices::run()
{

	gpMainFrame->OnStartStream();
	//线程结束后执行
	gpMainFrame->OnStopStream();

	return ;
}



