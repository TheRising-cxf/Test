#pragma once
#pragma execution_character_set("utf-8")
#include <qmainwindow.h>
#include <ui_dealImage.h>
#include<QWheelEvent>
#include<QKeyEvent>
class DealImage :
	public QMainWindow
{
	Q_OBJECT
signals:
	void ExitWin();//新建一个信号槽
public:
	DealImage(QWidget *parent = Q_NULLPTR);
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
private:
	float imageScale;
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
};

