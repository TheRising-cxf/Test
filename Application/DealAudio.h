#pragma once
#include<QChart>
using namespace QtCharts;
#include <qmainwindow.h>
#include <ui_dealAudio.h>
#include    <QMainWindow>
#include    <QtCharts>
#include    <QAudioDeviceInfo>
#include    <QAudioInput>
#include    <qaudiooutput.h>
#include    "qmydisplaydevice.h"
#include <qchartview.h>
#pragma execution_character_set("utf-8")
class DealAuido :public QMainWindow
{
	Q_OBJECT
public:
	DealAuido(QWidget *parent = Q_NULLPTR);
	void closeEvent(QCloseEvent *);
	FILE* outputFile;
	QAudioOutput *outAudio;
signals:
	void ExitWin();//新建一个信号槽
private:
	const qint64  displayPointsCount = 4000;

	QLineSeries *lineSeries;//曲线序列

	QList<QAudioDeviceInfo> deviceList;  //音频录入设备列表

	QAudioDeviceInfo curDevice;//当前输入设备

	QmyDisplayDevice    *displayDevice; //用于显示的IODevice

	QAudioInput         *audioInput;//音频输入设备

	QString SampleTypeString(QAudioFormat::SampleType sampleType);

	QString ByteOrderString(QAudioFormat::Endian endian);

private slots:
	//自定义槽函数
	void    on_IODevice_UpdateBlockSize(qint64 blockSize);

	//
	void on_comboDevices_currentIndexChanged(int index);

	void on_actStart_triggered();

	void on_actStop_triggered();

	void on_actDeviceTest_triggered();

	void on_actSave_triggered();

private:
	Ui::AudioWindow ui;
};

