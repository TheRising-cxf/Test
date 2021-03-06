#include "DealImage.h"
#include <QtWidgets/QFileDialog>
#include <QMessageBox>
#include <math.h>
#include "opencv.hpp"

using namespace std;
using namespace cv;
unsigned char bySaturationMap[256 * 256];
void CreateSaturationMap(int value)
{
	int nTmp;
	for (int g = 0; g < 256; ++g)
	{
		for (int i = 0; i < 256; ++i)
		{
			nTmp = g + value * (i - g) / 128;
			nTmp = min(255, max(0, nTmp));
			bySaturationMap[g * 256 + i] = nTmp;
		}
	}
}
Mat Img_ISP(Mat image)
{
	Mat newImage = image.clone();
	for (int x = 0; x < image.cols; x++)
	{
		for (int y = 0; y < image.rows; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			r = image.at<Vec4b>(y, x)[0];
			g = image.at<Vec4b>(y, x)[1];
			b = image.at<Vec4b>(y, x)[2];
			newImage.at<Vec4b>(y, x)[0] = bySaturationMap[g * 256 + r];
			newImage.at<Vec4b>(y, x)[2] = bySaturationMap[g * 256 + b];
		}
	}
	return newImage;
}
Mat SharpLaplacianRGB(Mat image, float threash)
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
					r += image.at<Vec4b>(y + i, x + j)[0] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					g += image.at<Vec4b>(y + i, x + j)[1] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					b += image.at<Vec4b>(y + i, x + j)[2] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
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
Mat ContrastRGB(Mat image, float contrastThresh)
{
	Mat newImage = image.clone();
	for (int x = 0; x < image.cols; x++)
	{
		for (int y = 0; y < image.rows; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			r = pow(image.at<Vec4b>(y, x)[0] / 255.0f, contrastThresh) * 255.0f;
			g = pow(image.at<Vec4b>(y, x)[1] / 255.0f, contrastThresh) * 255.0f;
			b = pow(image.at<Vec4b>(y, x)[2] / 255.0f, contrastThresh) * 255.0f;
			newImage.at<Vec4b>(y, x)[0] = max(0, min(255, r));
			newImage.at<Vec4b>(y, x)[1] = max(0, min(255, g));
			newImage.at<Vec4b>(y, x)[2] = max(0, min(255, b));
			newImage.at<Vec4b>(y, x)[3] = image.at<Vec4b>(y, x)[3];
		}
	}
	return newImage;
}
Mat BrightnessRGB(Mat image, float brightnessThresh)
{
	Mat newImage = image.clone();
	for (int x = 0; x < image.cols; x++)
	{
		for (int y = 0; y < image.rows; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			r = image.at<Vec4b>(y, x)[0] + brightnessThresh;
			g = image.at<Vec4b>(y, x)[1] + brightnessThresh;
			b = image.at<Vec4b>(y, x)[2] + brightnessThresh;
			newImage.at<Vec4b>(y, x)[0] = max(0, min(255, r));
			newImage.at<Vec4b>(y, x)[1] = max(0, min(255, g));
			newImage.at<Vec4b>(y, x)[2] = max(0, min(255, b));
		}
	}
	return newImage;
}

Mat ChromaRGB(Mat& img, int  hue)
{

	Mat temp;
	temp = img.clone();
	
	cvtColor(temp, temp, CV_RGBA2RGB);
	cvtColor(temp, temp, CV_RGB2HSV);

	for (int x = 0; x < temp.cols; x++)
	{
		for (int y = 0; y < temp.rows; y++)
		{
			int h = 0;
			h = temp.at<Vec3b>(y, x)[0] + hue;
			temp.at<Vec3b>(y, x)[0] = max(0, min(180, h));
		}
	}
	cvtColor(temp, temp, CV_HSV2RGB);
	cvtColor(temp, temp, CV_RGB2RGBA);
	return temp;
}
DealImage::DealImage(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.compare->installEventFilter(this);
	connect(ui.loadImage, &QPushButton::clicked, this, &DealImage::LoadImage);
	connect(ui.resize,&QSlider::valueChanged, this, &DealImage::SetLineEditValue);
	connect(ui.reset, &QPushButton::clicked, this, &DealImage::ResetImage);
	connect(ui.save, &QPushButton::clicked, this, &DealImage::SaveImage);
	connect(ui.sharpen, &QSlider::valueChanged, this, &DealImage::SharpenImage);
	connect(ui.contrast, &QSlider::valueChanged,    this, &DealImage::ContrastImage);
	connect(ui.brightness, &QSlider::valueChanged, this, &DealImage::BrightnessImage);
	connect(ui.filter, &QSlider::valueChanged, this, &DealImage::FilterImage);
	connect(ui.saturation, &QSlider::valueChanged, this, &DealImage::SaturationImage);
	connect(ui.chroma, &QSlider::valueChanged, this, &DealImage::ChromaImage);
	imageFlag = false;
	leftMousePress = false;
	imageStartPosition_x = 0;
	imageStartPosition_y = 0;
	imageScale = 1.0f;
	sharpenThresh = 0.0f;//????????
	contrastThresh = 1.0f;//??????????
	brightnessThresh = 0.0f;//????????
	saturationThresh = 128.0f;//??????????
	chromaThresh = 0;
	filterSize = 1;//????????
	pFrequencyData = NULL;
}
DealImage::~DealImage()
{
	if (pFrequencyData != NULL)
	{
		delete[]pFrequencyData;
		pFrequencyData = NULL;
	}
}
void DealImage::closeEvent(QCloseEvent *)
{
	emit ExitWin();
}
void DealImage::LoadImage() {
	QString imgPath = QFileDialog::getOpenFileName(this,
		tr("??????????"),
		"F:",
		tr("????????(*png *jpg)"));
	if (!(orginImg.load(imgPath))) //????????
	{
		QMessageBox::information(this,
			tr("????????????"),
			tr("????????????!"));
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
	QString str = QString("????????: %1\%").arg(pos);
	ui.label_2->setText(str);
	ui.resize->setValue(pos);
	ShowImage();

}
void DealImage::ResetImage() {
	curImage = orginImg.copy();
	imageScale = 1.0f;
	sharpenThresh = 0.0f;//????????
	contrastThresh = 1.0f;//??????????
	brightnessThresh = 0.0f;//????????
	saturationThresh = 128.0f;//??????????
	chromaThresh = 0;
	filterSize = 1;//????????
	ui.resize->setValue(imageScale*100);
	ui.sharpen->setValue(sharpenThresh);
	ui.contrast->setValue(contrastThresh*100);
	ui.brightness->setValue(brightnessThresh);
	ShowImage();

}
void DealImage::SetLineEditValue(int value)
{
	int pos = ui.resize->value();
	QString str = QString("????????: %1\%").arg(pos);
	ui.label_2->setText(str);
	if (imageFlag) {
		QImage tmp = orginImg.copy();
		imageScale = float(pos) / 100.f;
		int pos = imageScale * 100;
		QString str = QString("????????: %1\%").arg(pos);
		ui.label_2->setText(str);
		ShowImage();
	}
}
void DealImage::on_actionFFT_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	if (pFrequencyData != NULL)
	{
		delete []pFrequencyData;
		pFrequencyData = NULL;
	}
	pFrequencyData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	imshow("fft", result);
}
void DealImage::on_actionIFFT_triggered()
{
	if (!imageFlag)return;
	if (pFrequencyData != NULL) {
		Mat IFFTImage = Mat::zeros(Size(curImage.width(),curImage.height()), CV_8UC4);
		ImageIFFT((unsigned char*)IFFTImage.datastart, pFrequencyData, widthFFT, heightFFT, IFFTImage.cols, IFFTImage.rows);
		imshow("ifft", IFFTImage);
		delete []pFrequencyData;
		pFrequencyData = NULL;
	}
}
void DealImage::on_action_low_1_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat ILPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageILPF((unsigned char*)ILPFImage.datastart, tmpData, widthFFT, heightFFT, ILPFImage.cols, ILPFImage.rows,120);
	imshow("ilpf", ILPFImage);
	delete []tmpData;
}
void DealImage::on_action_low_2_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat BLPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageBLPF((unsigned char*)BLPFImage.datastart, tmpData, widthFFT, heightFFT, BLPFImage.cols, BLPFImage.rows, 100,3);
	imshow("blpf", BLPFImage);
	delete[]tmpData;
}
void DealImage::on_action_low_3_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat ELPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageELPF((unsigned char*)ELPFImage.datastart, tmpData, widthFFT, heightFFT, ELPFImage.cols, ELPFImage.rows, 10, 3);
	imshow("elpf", ELPFImage);
	delete[]tmpData;
}
void DealImage::on_action_low_4_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat TLPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageTLPF((unsigned char*)TLPFImage.datastart, tmpData, widthFFT, heightFFT, TLPFImage.cols, TLPFImage.rows, 100, 150);
	imshow("tlpf", TLPFImage);
	delete[]tmpData;
}
void DealImage::on_action_low_5_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat GLPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageGLPF((unsigned char*)GLPFImage.datastart, tmpData, widthFFT, heightFFT, GLPFImage.cols, GLPFImage.rows, 10);
	imshow("glpf", GLPFImage);
	delete[]tmpData;
}
void DealImage::on_action_high_1_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat ILPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageILPF((unsigned char*)ILPFImage.datastart, tmpData, widthFFT, heightFFT, ILPFImage.cols, ILPFImage.rows, 10,true);
	imshow("ilpfh", ILPFImage);
	delete[]tmpData;
}
void DealImage::on_action_high_2_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat BLPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageBLPF((unsigned char*)BLPFImage.datastart, tmpData, widthFFT, heightFFT, BLPFImage.cols, BLPFImage.rows, 100, 3,true);
	imshow("blpfh", BLPFImage);
	delete[]tmpData;
}
void DealImage::on_action_high_3_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat ELPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageELPF((unsigned char*)ELPFImage.datastart, tmpData, widthFFT, heightFFT, ELPFImage.cols, ELPFImage.rows, 10, 3,true);
	imshow("elpfh", ELPFImage);
	delete[]tmpData;
}
void DealImage::on_action_high_4_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat TLPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageTLPF((unsigned char*)TLPFImage.datastart, tmpData, widthFFT, heightFFT, TLPFImage.cols, TLPFImage.rows, 100, 150,true);
	imshow("tlpfh", TLPFImage);
	delete[]tmpData;
}
void DealImage::on_action_high_5_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat GLPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageGLPF((unsigned char*)GLPFImage.datastart, tmpData, widthFFT, heightFFT, GLPFImage.cols, GLPFImage.rows, 10,true);
	imshow("glpfh", GLPFImage);
	delete[]tmpData;
}
void DealImage::on_action_high_6_triggered()
{
	if (!imageFlag)return;

	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	complex<double>* tmpData;
	tmpData = ImageFFT((unsigned char*)result.datastart, result.cols, result.rows, widthFFT, heightFFT);
	Mat GLPFImage = Mat::zeros(Size(curImage.width(), curImage.height()), CV_8UC4);
	ImageGLPFP((unsigned char*)GLPFImage.datastart, tmpData, widthFFT, heightFFT, GLPFImage.cols, GLPFImage.rows, 10, 1 ,0.5);
	imshow("glpfhp", GLPFImage);
	delete[]tmpData;
}
void DealImage::on_action_hisEqu_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImageHisEqu((unsigned char*)result.datastart, curImage.width(), curImage.height());
	imshow("hisEqu", result);
}
void DealImage::on_action_hisSML_triggered(){
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	double zhist[3][256] = { 0 };
	for (int i = 0; i <= 255; i++) {
		for (int k = 0; k < 3; k++) {
			zhist[k][i] = i / 255.0f;
		}
	}
	ImageHisSML((unsigned char*)result.datastart, curImage.width(), curImage.height(),zhist);
	imshow("hisSML", result);
}
void DealImage::on_action_hisGML_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	double zhist[3][256] = { 0 };
	for (int i = 0; i <= 255; i++) {
		for (int k = 0; k < 3; k++) {
			zhist[k][i] = i / 255.0f;
		}
	}
	ImageHisGML((unsigned char*)result.datastart, curImage.width(), curImage.height(), zhist);
	imshow("hisGML", result);
}
void DealImage::on_action_lpls_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	SharpLaplacian((unsigned char*)result.datastart, curImage.width(), curImage.height(),0.15);
	imshow("lpls", result);
}
void DealImage::on_action_grad_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImageGrad((unsigned char*)result.datastart, curImage.width(), curImage.height(),0.3, 10,0);
	imshow("grad", result);
}
void DealImage::on_action_prewitt_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImagePrewitt((unsigned char*)result.datastart, curImage.width(), curImage.height(), 0.3, 10, 0);
	imshow("prewitt", result);
}
void DealImage::on_action_sobel_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImageSobel((unsigned char*)result.datastart, curImage.width(), curImage.height(), 0.3, 10, 0);
	imshow("sobel", result);
}
void DealImage::on_action_qualc_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImageQualc((unsigned char*)result.datastart, curImage.width(), curImage.height(), 0.3);
	imshow("qualc", result);
}
void DealImage::on_action_log_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImageLog((unsigned char*)result.datastart, curImage.width(), curImage.height(), 0.15);
	imshow("log", result);
}
void DealImage::on_action_logT_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImageLogT((unsigned char*)result.datastart, curImage.width(), curImage.height(), 0.15);
	imshow("logT", result);
}
void DealImage::on_action_expT_triggered()
{
	if (!imageFlag)return;
	cv::Mat tmpImg(curImage.height(), curImage.width(), CV_8UC4, (uchar*)curImage.bits(), curImage.bytesPerLine());
	Mat result = tmpImg.clone();
	ImageExpT((unsigned char*)result.datastart, curImage.width(), curImage.height(), 0.15);
	imshow("expT", result);
}
void DealImage::mousePressEvent(QMouseEvent *event) // ????????????
{
	int x = event->pos().x() - ui.groupBox->x();
	int y = event->pos().y() - ui.groupBox->y();
	if (x<ui.label->x() || y< ui.label->y() || x > ui.label->x() + ui.label->width() || y > ui.label->y() + ui.label->height())return;
	if (event->button() == Qt::LeftButton) {       // ??????????????????
		QCursor cursor;
		cursor.setShape(Qt::ClosedHandCursor);
		QApplication::setOverrideCursor(cursor); // ??????????????????????
		leftMousePress = true;
		i_mouse_last_position_x = event->localPos().x();
		i_mouse_last_position_y = event->localPos().y();
	}
}

void DealImage::mouseMoveEvent(QMouseEvent *event) // ????????????
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

void DealImage::mouseReleaseEvent(QMouseEvent *event) // ????????????
{
	Q_UNUSED(event);
	QApplication::restoreOverrideCursor();         // ????????????????
	leftMousePress = false;
}

void DealImage::wheelEvent(QWheelEvent *event)    // ????????
{
	int x = event->pos().x() - ui.groupBox->x();
	int y = event->pos().y() - ui.groupBox->y();
	if (x<ui.label->x() || y< ui.label->y() || x > ui.label->x() + ui.label->width() || y > ui.label->y() + ui.label->height())return;
	if (event->delta() > 0) {                    // ??????????????????
		if (imageFlag) {                        // ????????
			if (imageScale <= 2) {
				imageScale += 0.02;
				int pos = imageScale * 100;
				QString str = QString("????????: %1\%").arg(pos);
				ui.label_2->setText(str);
				ui.resize->setValue(pos);
				ShowImage();
			}
		}
	}
	else {                                     // ????????????????????????
		if (imageFlag) {                       // ????????
			if (imageScale >=0) {
				imageScale -= 0.02;
				int pos = imageScale * 100;
				QString str = QString("????????: %1\%").arg(pos);
				ui.label_2->setText(str);
				ui.resize->setValue(pos);
				ShowImage();
			}
		}              
	}
}
void DealImage::ShowImage() {
	//????????
	QImage tmp = orginImg.copy();
	Mat result;
	tmp = tmp.scaled(tmp.width()*imageScale, tmp.height()*imageScale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	int x = -(ui.label->width() / 2 - tmp.width() / 2) + imageStartPosition_x;
	int y = -(ui.label->height() / 2 - tmp.height() / 2) + imageStartPosition_y;
	cv::Mat tmpImg(tmp.height(), tmp.width(), CV_8UC4, (uchar*)tmp.bits(), tmp.bytesPerLine());
	result = tmpImg.clone();
   //??????????
	if (contrastThresh != 1) {
		result = ContrastRGB(result, contrastThresh);
	}
	//????????
	if (brightnessThresh != 0) {
		result = BrightnessRGB(result, brightnessThresh);
	}
	//??????????
	if (saturationThresh != 128) {
		result = Img_ISP(result);
	}
	//????????
	if (chromaThresh != 0) {
		result = ChromaRGB(result, chromaThresh);
	}
	//????????
	GaussianBlur(result, result, Size(filterSize, filterSize), 0, 0);
	//????????
	if (sharpenThresh != 0) {
		result = SharpLaplacianRGB(result, sharpenThresh);
	}
	tmp = QImage((const uchar *)result.data, result.cols, result.rows, result.step, QImage::Format_RGB32).copy();
	curImage = tmp.copy();
	tmp = tmp.copy(x, y, ui.label->width(), ui.label->height());
	ui.label->setPixmap(QPixmap::fromImage(tmp));
}
void DealImage::translate_image() {
	if (!imageFlag)return;
	imageStartPosition_x = -1 * (i_mouse_new_position_x - i_mouse_last_position_x) + imageStartPosition_x;
	imageStartPosition_y = -1 * (i_mouse_new_position_y - i_mouse_last_position_y) + imageStartPosition_y;
	int pos = imageScale * 100;
	QString str = QString("????????: %1\%").arg(pos);
	ui.label_2->setText(str);
	ui.resize->setValue(pos);
	ShowImage();
}
void DealImage::SaveImage() {
	if (!imageFlag)return;
	QString filename1 = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("Images (*.png *.bmp *.jpg)")); //????????
	curImage.save(filename1);
}

void DealImage::SharpenImage()
{
	if (!imageFlag)return;
	int pos = ui.sharpen->value();
	sharpenThresh = pos / 100.0f;
	ShowImage();
}

void DealImage::ContrastImage()
{
	if (!imageFlag)return;
	int pos = ui.contrast->value();
	contrastThresh = pos / 100.0f;
	ShowImage();
}

void DealImage::BrightnessImage()
{
	if (!imageFlag)return;
	int pos = ui.brightness->value();
	brightnessThresh= pos;
	ShowImage();
}

void DealImage::FilterImage()
{
	if (!imageFlag)return;
	int pos = ui.filter->value();
	filterSize = pos*2+1;
	ShowImage();
}

void DealImage::SaturationImage()
{
	if (!imageFlag)return;
	int pos = ui.saturation->value();
	saturationThresh = pos;
	CreateSaturationMap(saturationThresh);
	ShowImage();
}
 
void DealImage::ChromaImage()
{
	if (!imageFlag)return;
	int pos = ui.chroma->value();
	chromaThresh = pos;
	ShowImage();
}

bool DealImage::eventFilter(QObject * watched, QEvent * event)
{
	if (watched == ui.compare)
	{
		if (event->type() == QEvent::Enter)
		{
			/*????????????????*/
			//printf("cc");
			return true;
		}
		else if (event->type() == QEvent::Leave)
		{
			/*????????????????*/
			// printf("bb");
			return true;
		}
		else if (event->type() == QEvent::MouseButtonPress)
		{
			/*????????????????*/
			QImage tmp = curImage.copy();
			tmp = tmp.scaled(tmp.width()*imageScale, tmp.height()*imageScale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			int x = -(ui.label->width() / 2 - tmp.width() / 2) + imageStartPosition_x;
			int y = -(ui.label->height() / 2 - tmp.height() / 2) + imageStartPosition_y;
			tmp = tmp.copy(x, y, ui.label->width(), ui.label->height());

			ui.label->setPixmap(QPixmap::fromImage(tmp));
			return true;
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{
			/*????????????????*/
			ShowImage();
			return true;
		}

	}
	return QWidget::eventFilter(watched, event);
}
