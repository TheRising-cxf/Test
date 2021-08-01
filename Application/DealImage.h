#pragma once
#pragma execution_character_set("utf-8")
#include <qmainwindow.h>
#include <ui_dealImage.h>
#include<QWheelEvent>
#include<QKeyEvent>
#include "ImageProcessAlgorithom.h"
class DealImage :
	public QMainWindow
{
	Q_OBJECT
signals:
	void ExitWin();//新建一个信号槽
public:
	DealImage(QWidget *parent = Q_NULLPTR);
	~DealImage();
	void closeEvent(QCloseEvent *);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void ShowImage();
	void translate_image();
	void ResetImage();
	void SaveImage();
	void SharpenImage();
	void ContrastImage();
	void BrightnessImage();
	void FilterImage();
	void SaturationImage();
	void ChromaImage();
	bool eventFilter(QObject *watched, QEvent *event);
private:
	float imageScale;//图像缩放
	float sharpenThresh;//图像锐化
	float contrastThresh;//图像对比度
	float brightnessThresh;//图像亮度
	float saturationThresh;//图像饱和度
	int   chromaThresh;
	int   filterSize;//平滑滤波
	int   widthFFT, heightFFT;
	complex<double>* pFrequencyData;
	Ui::ImageWindow ui;
	QString imgPath;
	QImage orginImg;
	QImage curImage;
	bool imageFlag;
	bool leftMousePress;
	int i_mouse_last_position_x;
	int i_mouse_last_position_y;
	int i_mouse_new_position_x;
	int i_mouse_new_position_y;
	int imageStartPosition_x;
	int imageStartPosition_y;
private slots:
	void LoadImage();
	void SetLineEditValue(int value);
	void on_actionFFT_triggered();
	void on_actionIFFT_triggered();
	void on_action_low_1_triggered();
	void on_action_low_2_triggered();
	void on_action_low_3_triggered();
	void on_action_low_4_triggered();
	void on_action_low_5_triggered();
	void on_action_high_1_triggered();
	void on_action_high_2_triggered();
	void on_action_high_3_triggered();
	void on_action_high_4_triggered();
	void on_action_high_5_triggered();
	void on_action_high_6_triggered();
	void on_action_hisEqu_triggered();
	void on_action_hisSML_triggered();
	void on_action_hisGML_triggered();
	void on_action_lpls_triggered();
	void on_action_grad_triggered();
	void on_action_prewitt_triggered();
	void on_action_sobel_triggered();
	void on_action_qualc_triggered();
	void on_action_log_triggered();
	void on_action_logT_triggered();
	void on_action_expT_triggered();
};

