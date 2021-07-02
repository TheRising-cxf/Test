#include "DealImage.h"
#include <QtWidgets/QFileDialog>
#include <QMessageBox>
#include <math.h>
#include "opencv.hpp"
using namespace std;
using namespace cv;
DealImage::DealImage(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.loadImage, &QPushButton::clicked, this, &DealImage::LoadImage);
	connect(ui.resize,&QSlider::valueChanged, this, &DealImage::SetLineEditValue);
	connect(ui.reset, &QPushButton::clicked, this, &DealImage::ResetImage);
	connect(ui.save, &QPushButton::clicked, this, &DealImage::SaveImage);
	connect(ui.sharpen, &QSlider::valueChanged, this, &DealImage::SharpenImage);
	imageFlag = false;
	leftMousePress = false;
	imageStartPosition_x = 0;
	imageStartPosition_y = 0;
}
void DealImage::closeEvent(QCloseEvent *)
{
	emit ExitWin();
}
void DealImage::LoadImage() {
	QString imgPath = QFileDialog::getOpenFileName(this,
		tr("选择图片！"),
		"F:",
		tr("图片文件(*png *jpg)"));
	if (!(orginImg.load(imgPath))) //加载图像
	{
		QMessageBox::information(this,
			tr("打开图像失败"),
			tr("打开图像失败!"));
		return;
	}
	curImage = orginImg.copy();
	QImage tmp = curImage.copy();
	imageScale = fmin(float(ui.label->width())/float(tmp.width()), float(ui.label->height())/float(tmp.height()));
	if (imageScale > 1) imageScale = 1;
	imageStartPosition_x = 0;
	imageStartPosition_y = 0;
	imageFlag = true;
	int pos = imageScale * 100;
	QString str = QString("放大倍数: %1\%").arg(pos);
	ui.label_2->setText(str);
	ui.resize->setValue(pos);
	ShowImage();

}
void DealImage::ResetImage() {
	curImage = orginImg.copy();
	QImage tmp = curImage.copy();
	imageScale = fmin(float(ui.label->width()) / float(tmp.width()), float(ui.label->height()) / float(tmp.height()));
	if (imageScale > 1) imageScale = 1;
	imageStartPosition_x = 0;
	imageStartPosition_y = 0;
	imageFlag = true;
	int pos = imageScale * 100;
	QString str = QString("放大倍数: %1\%").arg(pos);
	ui.label_2->setText(str);
	ui.resize->setValue(pos);
	ShowImage();

}
void DealImage::SetLineEditValue(int value)
{
	int pos = ui.resize->value();
	QString str = QString("放大倍数: %1\%").arg(pos);
	ui.label_2->setText(str);
	if (imageFlag) {
		QImage tmp = orginImg.copy();
		imageScale = float(pos) / 100.f;
		int pos = imageScale * 100;
		QString str = QString("放大倍数: %1\%").arg(pos);
		ui.label_2->setText(str);
		ShowImage();
	}
}
void DealImage::mousePressEvent(QMouseEvent *event) // 鼠标按下事件
{
	int x = event->pos().x() - ui.groupBox->x();
	int y = event->pos().y() - ui.groupBox->y();
	if (x<ui.label->x() || y< ui.label->y() || x > ui.label->x() + ui.label->width() || y > ui.label->y() + ui.label->height())return;
	if (event->button() == Qt::LeftButton) {       // 如果是鼠标左键按下
		QCursor cursor;
		cursor.setShape(Qt::ClosedHandCursor);
		QApplication::setOverrideCursor(cursor); // 使鼠标指针暂时改变形状
		leftMousePress = true;
		i_mouse_last_position_x = event->localPos().x();
		i_mouse_last_position_y = event->localPos().y();
	}
}

void DealImage::mouseMoveEvent(QMouseEvent *event) // 鼠标移动事件
{
	int x = event->pos().x() - ui.groupBox->x();
	int y = event->pos().y() - ui.groupBox->y();
	if (x<ui.label->x() || y< ui.label->y() || x > ui.label->x() + ui.label->width() || y > ui.label->y() + ui.label->height())return;
	if (!leftMousePress)return;
	if (imageFlag) {
		i_mouse_new_position_x = event->localPos().x();
		i_mouse_new_position_y = event->localPos().y();
		translate_image();
		i_mouse_last_position_x = i_mouse_new_position_x;
		i_mouse_last_position_y = i_mouse_new_position_y;
	}
}

void DealImage::mouseReleaseEvent(QMouseEvent *event) // 鼠标释放事件
{
	Q_UNUSED(event);
	QApplication::restoreOverrideCursor();         // 恢复鼠标指针形状
	leftMousePress = false;
}

void DealImage::wheelEvent(QWheelEvent *event)    // 滚轮事件
{
	int x = event->pos().x() - ui.groupBox->x();
	int y = event->pos().y() - ui.groupBox->y();
	if (x<ui.label->x() || y< ui.label->y() || x > ui.label->x() + ui.label->width() || y > ui.label->y() + ui.label->height())return;
	if (event->delta() > 0) {                    // 当滚轮远离使用者时
		if (imageFlag) {                        // 进行放大
			if (imageScale <= 2) {
				imageScale += 0.02;
				int pos = imageScale * 100;
				QString str = QString("放大倍数: %1\%").arg(pos);
				ui.label_2->setText(str);
				ui.resize->setValue(pos);
				ShowImage();
			}
		}
	}
	else {                                     // 当滚轮向使用者方向旋转时
		if (imageFlag) {                       // 进行缩小
			if (imageScale >=0) {
				imageScale -= 0.02;
				int pos = imageScale * 100;
				QString str = QString("放大倍数: %1\%").arg(pos);
				ui.label_2->setText(str);
				ui.resize->setValue(pos);
				ShowImage();
			}
		}              
	}
}
void DealImage::ShowImage() {
	QImage tmp = curImage.copy();
	tmp = tmp.scaled(tmp.width()*imageScale, tmp.height()*imageScale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	int x = -(ui.label->width() / 2 - tmp.width() / 2) + imageStartPosition_x;
	int y = -(ui.label->height() / 2 - tmp.height() / 2) + imageStartPosition_y;

	tmp = tmp.copy(x, y, ui.label->width(), ui.label->height());
	ui.label->setPixmap(QPixmap::fromImage(tmp));
}
void DealImage::translate_image() {
	if (!imageFlag)return;
	imageStartPosition_x = -1 * (i_mouse_new_position_x - i_mouse_last_position_x) + imageStartPosition_x;
	imageStartPosition_y = -1 * (i_mouse_new_position_y - i_mouse_last_position_y) + imageStartPosition_y;
	int pos = imageScale * 100;
	QString str = QString("放大倍数: %1\%").arg(pos);
	ui.label_2->setText(str);
	ui.resize->setValue(pos);
	ShowImage();
}
void DealImage::SaveImage() {
	if (!imageFlag)return;
	QString filename1 = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("Images (*.png *.bmp *.jpg)")); //选择路径
	curImage.save(filename1);
}

Mat SharpLaplacianRGB(Mat image,float threash)
{
	Mat newImage = image.clone();
	int kernel[3][3] = {
	{-1,-1,-1},
	{-1,8,-1},
	{-1,-1,-1} };
	int sizeKernel = 3;
	int sumKernel = 3;
	QColor color;
	for (int x = sizeKernel / 2; x < image.cols - sizeKernel / 2; x++)
	{
		for (int y = sizeKernel / 2; y < image.rows - sizeKernel / 2; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++)
			{
				for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++)
				{
					r += image.at<Vec4b>(y+i, x+j)[0]*kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					g += image.at<Vec4b>(y+i, x+j)[1]*kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					b += image.at<Vec4b>(y+i, x+j)[2]*kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
				}
			}
			r = r * threash + image.at<Vec4b>(y, x)[0];
			g = g * threash + image.at<Vec4b>(y, x)[1];
			b = b * threash + image.at<Vec4b>(y, x)[2];
			newImage.at<Vec4b>(y, x)[0] = max(0, min(255, r));
			newImage.at<Vec4b>(y, x)[1] = max(0, min(255, g));
			newImage.at<Vec4b>(y, x)[2] = max(0, min(255, b));
			newImage.at<Vec4b>(y, x)[3] = image.at<Vec4b>(y, x)[3];
		}
	}
	return newImage;
}
void DealImage::SharpenImage()
{
	if (!imageFlag)return;
	int pos = ui.sharpen->value();
	float thresh = pos / 20.0f;
	cv::Mat tmp(orginImg.height(), orginImg.width(), CV_8UC4, (uchar*)orginImg.bits(), orginImg.bytesPerLine());
	cv::Mat result = tmp.clone();
	result = SharpLaplacianRGB(result, thresh);
	QImage dest((const uchar *)result.data, result.cols, result.rows, result.step, QImage::Format_RGB32);   
	curImage = dest.copy();
	ui.label->clear();
	ShowImage();
}
