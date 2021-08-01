#include "ImageProcessAlgorithom.h"
#include <algorithm>
/*************************************************************************
 *
 * \函数名称：
 *   FFT_1D()
 *
 * \输入参数:
 *   complex<double> * pCTData - 指向时域数据的指针，输入的需要变换的数据
 *   complex<double> * pCFData - 指向频域数据的指针，输出的经过变换的数据
 *   nLevel －傅立叶变换蝶形算法的级数，2的幂数，
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   一维快速傅立叶变换。
 *
 *************************************************************************
 */

void FFT_1D(complex<double> * pCTData, complex<double> * pCFData, int nLevel)
{
	// 循环控制变量 
	int     i;
	int     j;
	int     k;

	// 傅立叶变换点数 
	int nCount = 0;

	// 计算傅立叶变换点数 
	nCount = (int)pow(2.0, nLevel);

	// 某一级的长度 
	int nBtFlyLen;
	nBtFlyLen = 0;

	// 变换系数的角度 ＝2 * PI * i / nCount 
	double dAngle;

	complex<double> *pCW;

	// 分配内存，存储傅立叶变化需要的系数表 
	pCW = new complex<double>[nCount];

	// 计算傅立叶变换的系数 
	for (i = 0; i < nCount; i++)
	{
		dAngle = -2 * PI * i / nCount;
		pCW[i] = complex<double>(cos(dAngle), sin(dAngle));
	}

	// 变换需要的工作空间 
	complex<double> *pCWork1, *pCWork2;

	// 分配工作空间 
	pCWork1 = new complex<double>[nCount];
	pCWork2 = new complex<double>[nCount];

	// 临时变量 
	complex<double> *pCTmp;

	// 初始化，写入数据 
	memcpy(pCWork1, pCTData, sizeof(complex<double>) * nCount);

	// 临时变量 
	int nInter;
	nInter = 0;

	// 蝶形算法进行快速傅立叶变换 
	for (k = 0; k < nLevel; k++)
	{
		for (j = 0; j < (int)pow(2.0, k); j++)
		{
			//计算长度 
			nBtFlyLen = (int)pow(2.0, (nLevel - k));

			//倒序重排，加权计算 
			for (i = 0; i < nBtFlyLen / 2; i++)
			{
				nInter = j * nBtFlyLen;
				pCWork2[i + nInter] = pCWork1[i + nInter] + pCWork1[i + nInter + nBtFlyLen / 2];
				pCWork2[i + nInter + nBtFlyLen / 2] = (pCWork1[i + nInter] - pCWork1[i + nInter + nBtFlyLen / 2]) * pCW[(int)(i * pow(2.0, k))];
			}
		}

		// 交换 pCWork1和pCWork2的数据 
		pCTmp = pCWork1;
		pCWork1 = pCWork2;
		pCWork2 = pCTmp;
	}

	// 重新排序 
	for (j = 0; j < nCount; j++)
	{
		nInter = 0;
		for (i = 0; i < nLevel; i++)
		{
			if (j&(1 << i))
			{
				nInter += 1 << (nLevel - i - 1);
			}
		}
		pCFData[j] = pCWork1[nInter];
	}

	// 释放内存空间 
	delete pCW;
	delete pCWork1;
	delete pCWork2;
	pCW = NULL;
	pCWork1 = NULL;
	pCWork2 = NULL;
}

/*************************************************************************
 *
 * \函数名称：
 *   FFT_2D()
 *
 * \输入参数:
 *   complex<double> * pCTData - 图像数据
 *   int    nWidth - 数据宽度
 *   int    nHeight - 数据高度
 *   complex<double> * pCFData - 傅立叶变换后的结果
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   二维傅立叶变换。
 *
 ************************************************************************
 */
void FFT_2D(complex<double> * pSpaceData, complex<double> * pFrequencyData, int nWidthFFT, int nHeightFFT)
{
	// 循环控制变量 
	int x;
	int y;

	// x，y（行列）方向上的迭代次数 
	int nXLev = (int)(log((double)nWidthFFT) / log(2.0) + 0.5);
	int nYLev = (int)(log((double)nHeightFFT) / log(2.0) + 0.5);

	for (y = 0; y < nHeightFFT; ++y)
	{
		// x方向进行快速傅立叶变换 
		FFT_1D(&pSpaceData[nWidthFFT * y], &pFrequencyData[nWidthFFT * y], nXLev);
	}

	// pCFData中目前存储了pCTData经过行变换的结果 
	// 为了直接利用FFT_1D，需要把pCFData的二维数据转置，再一次利用FFT_1D进行 

	// 傅立叶行变换（实际上相当于对列进行傅立叶变换） 
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceData[nHeightFFT * x + y] = pFrequencyData[nWidthFFT * y + x];
		}
	}

	for (x = 0; x < nWidthFFT; ++x)
	{
		// 对x方向进行快速傅立叶变换，实际上相当于对原来的图象数据进行列方向的 
		// 傅立叶变换 
		FFT_1D(&pSpaceData[x * nHeightFFT], &pFrequencyData[x * nHeightFFT], nYLev);
	}

	// pCFData中目前存储了pCTData经过二维傅立叶变换的结果，但是为了方便列方向 
	// 的傅立叶变换，对其进行了转置，现在把结果转置回来 
	for (y = 0; y < nHeightFFT; y++)
	{
		for (x = 0; x < nWidthFFT; x++)
		{
			//pSpaceData[nHeightFFT * x + y] = pFrequencyData[nWidthFFT * y + x];
			pSpaceData[nWidthFFT * y + x] = pFrequencyData[nHeightFFT * x + y];
		}
	}

	memcpy(pFrequencyData, pSpaceData, sizeof(complex<double>) * nHeightFFT * nWidthFFT);
}

/***********************************************************************
 * \函数名称：
 *    IFFT_1D()
 *
 * \输入参数:
 *   complex<double> * pCTData - 指向时域数据的指针，输入的需要反变换的数据
 *   complex<double> * pCFData - 指向频域数据的指针，输出的经过反变换的数据
 *   nLevel －傅立叶变换蝶形算法的级数，2的幂数，
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   一维快速傅立叶反变换。
 *
 ************************************************************************
 */
void IFFT_1D(complex<double> * pCFData, complex<double> * pCTData, int nLevel)
{
	// 循环控制变量 
	int i;

	// 傅立叶反变换点数 
	int nCount;

	// 计算傅立叶变换点数 
	nCount = (int)pow(2.0, nLevel);

	// 变换需要的工作空间 
	complex<double> *pCWork;

	// 分配工作空间 
	pCWork = new complex<double>[nCount];

	// 将需要反变换的数据写入工作空间pCWork 
	memcpy(pCWork, pCFData, sizeof(complex<double>) * nCount);

	// 为了利用傅立叶正变换,可以把傅立叶频域的数据取共轭 
	// 然后直接利用正变换，输出结果就是傅立叶反变换结果的共轭 
	for (i = 0; i < nCount; i++)
	{
		pCWork[i] = complex<double>(pCWork[i].real(), -pCWork[i].imag());
	}

	// 调用快速傅立叶变换实现反变换，结果存储在pCTData中 
	FFT_1D(pCWork, pCTData, nLevel);

	// 求时域点的共轭，求得最终结果 
	// 根据傅立叶变换原理，利用这样的方法求得的结果和实际的时域数据 
	// 相差一个系数nCount 
	for (i = 0; i < nCount; i++)
	{
		pCTData[i] =
			complex<double>(pCTData[i].real() / nCount, -pCTData[i].imag() / nCount);
	}

	// 释放内存 
	delete pCWork;
	pCWork = NULL;
}

/*************************************************
Function:       IFFT_2D
Description:    二维傅立叶逆变换
input:
*   complex<double> * pSpaceData		- 图像数据的复数形式（空间域）
*   int    nWidthFFT						- 数据宽度
*   int    nHeightFFT						- 数据高度
output:
*   complex<double> * pFrequencyData	- 傅立叶变换后的结果（频率域）
Return:
Others:	 nWidthFFT和nHeightFFT要求是2的幂数
*************************************************/
void IFFT_2D(complex<double> *pFrequencyData, int nWidthFFT, int nHeightFFT, complex<double> *pSpaceData)
{
	// 循环控制变量
	int	x;
	int	y;

	// 计算x，y方向上的迭代次数
	int	nXLev = (int)(log((double)nWidthFFT) / log(2.0) + 0.5);
	int	nYLev = (int)(log((double)nHeightFFT) / log(2.0) + 0.5);

	// 1　行方向进行快速傅立叶逆变换	
	for (y = 0; y < nHeightFFT; ++y)
	{
		IFFT_1D(&pFrequencyData[nWidthFFT * y], &pSpaceData[nWidthFFT * y], nXLev);
	}

	// 2　转置
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
			pFrequencyData[nHeightFFT * x + y] = pSpaceData[nWidthFFT * y + x];
	}

	// 3　列方向进行快速傅立叶逆变换，	
	for (x = 0; x < nWidthFFT; ++x)
	{
		IFFT_1D(&pFrequencyData[nHeightFFT * x], &pSpaceData[nHeightFFT * x], nYLev);
	}

	// 4　转置回来
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
			pFrequencyData[nWidthFFT * y + x] = pSpaceData[nHeightFFT * x + y];
			//pFrequencyData[nHeightFFT * x + y] = pSpaceData[nWidthFFT * y + x];
	}

	// 5　给空间域赋值
	memcpy(pSpaceData, pFrequencyData, sizeof(complex<double>) * nHeightFFT * nWidthFFT);
}
complex<double>* ImageFFT(unsigned char* imageData, int m_nPicWidth, int m_nPicHeight, int& nWidthFFT,int& nHeightFFT) {

	/*计算傅立叶变换的宽度和高度，是2的幂数*/
	double dTemp = log((double)m_nPicWidth) / log(2.0);
	dTemp = ceil(dTemp);//返回大于或者等于指定表达式的最小整数
	dTemp = pow(2.0, dTemp);
	nWidthFFT = (int)dTemp;

	dTemp = log((double)m_nPicHeight) / log(2.0);
	dTemp = ceil(dTemp);
	dTemp = pow(2.0, dTemp);
	nHeightFFT = (int)dTemp;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//指向频域数据的指针
	complex<double>* pFrequencyData, *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	pFrequencyData = new complex<double>[nWidthFFT * nHeightFFT*3];
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	int x = 0;
	int y = 0;
	int pixel = 0;

	/*初始化空域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
		}
	}

	//把图像数据传给pSpaceData
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			//空域乘以pow(-1, x+y)，频谱移至中心
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(imageData[pixel] * pow(-1, x + y), 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(imageData[pixel + 1] * pow(-1, x + y), 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(imageData[pixel + 2] * pow(-1, x + y), 0);
		}
	}

	FFT_2D(pSpaceDataB, pFrequencyData1, nWidthFFT, nHeightFFT);
	FFT_2D(pSpaceDataG, pFrequencyData2, nWidthFFT, nHeightFFT);
	FFT_2D(pSpaceDataR, pFrequencyData3, nWidthFFT, nHeightFFT);

	pixel = 0;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			//取模
			dTemp = pFrequencyData1[y*nWidthFFT + x].real() * pFrequencyData1[y*nWidthFFT + x].real() + pFrequencyData1[y*nWidthFFT + x].imag()*pFrequencyData1[y*nWidthFFT + x].imag();
			dTemp = sqrt(dTemp) / (double)m_nPicWidth;//除以因子N
			//图像数据小于255
			if (dTemp > 255) dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pFrequencyData2[y*nWidthFFT + x].real() * pFrequencyData2[y*nWidthFFT + x].real() + pFrequencyData2[y*nWidthFFT + x].imag()*pFrequencyData2[y*nWidthFFT + x].imag();
			dTemp = sqrt(dTemp) / (double)m_nPicWidth;
			if (dTemp > 255) dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pFrequencyData3[y*nWidthFFT + x].real() * pFrequencyData3[y*nWidthFFT + x].real() + pFrequencyData3[y*nWidthFFT + x].imag()*pFrequencyData3[y*nWidthFFT + x].imag();
			dTemp = sqrt(dTemp) / (double)m_nPicWidth;
			if (dTemp > 255) dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
	return pFrequencyData;
}
void ImageIFFT(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight) {


	int x = 0;
	int y = 0;
	int pixel = 0;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	complex<double> *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	/*初始化空间域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
		}
	}

	//快速傅立叶变换
	IFFT_2D(pFrequencyData1, nWidthFFT, nHeightFFT, pSpaceDataB);
	IFFT_2D(pFrequencyData2, nWidthFFT, nHeightFFT, pSpaceDataG);
	IFFT_2D(pFrequencyData3, nWidthFFT, nHeightFFT, pSpaceDataR);

	pixel = 0;
	double dTemp;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			dTemp = pSpaceDataB[y * nWidthFFT + x].real() * pSpaceDataB[y * nWidthFFT + x].real() +
				pSpaceDataB[y * nWidthFFT + x].imag() * pSpaceDataB[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pSpaceDataG[y * nWidthFFT + x].real() * pSpaceDataG[y * nWidthFFT + x].real() +
				pSpaceDataG[y * nWidthFFT + x].imag() * pSpaceDataG[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pSpaceDataR[y * nWidthFFT + x].real() * pSpaceDataR[y * nWidthFFT + x].real() +
				pSpaceDataR[y * nWidthFFT + x].imag() * pSpaceDataR[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
}
void ImageILPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight,int thresh, bool useHigh) {


	int x = 0;
	int y = 0;
	int pixel = 0;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	complex<double> *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	/*初始化空间域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
			int tmp = sqrt((y - nHeightFFT / 2)*(y - nHeightFFT / 2) + (x - nWidthFFT / 2)*(x - nWidthFFT / 2));
			float h = 1;
			if (tmp > thresh) {
				h = 0;
			}
			else {
				h = 1;
			}
			if (useHigh)h = 1 - h;
			pFrequencyData1[y*nWidthFFT + x] *= h;
			pFrequencyData2[y*nWidthFFT + x] *= h;
			pFrequencyData3[y*nWidthFFT + x] *= h;

		}
	}

	//快速傅立叶变换
	IFFT_2D(pFrequencyData1, nWidthFFT, nHeightFFT, pSpaceDataB);
	IFFT_2D(pFrequencyData2, nWidthFFT, nHeightFFT, pSpaceDataG);
	IFFT_2D(pFrequencyData3, nWidthFFT, nHeightFFT, pSpaceDataR);

	pixel = 0;
	double dTemp;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			dTemp = pSpaceDataB[y * nWidthFFT + x].real() * pSpaceDataB[y * nWidthFFT + x].real() +
				pSpaceDataB[y * nWidthFFT + x].imag() * pSpaceDataB[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pSpaceDataG[y * nWidthFFT + x].real() * pSpaceDataG[y * nWidthFFT + x].real() +
				pSpaceDataG[y * nWidthFFT + x].imag() * pSpaceDataG[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pSpaceDataR[y * nWidthFFT + x].real() * pSpaceDataR[y * nWidthFFT + x].real() +
				pSpaceDataR[y * nWidthFFT + x].imag() * pSpaceDataR[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
}
void ImageBLPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, float sigma, int n, bool useHigh) {


	int x = 0;
	int y = 0;
	int pixel = 0;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	complex<double> *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	/*初始化空间域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
			float d = sqrt(pow(float(x - nWidthFFT / 2), 2) + pow(float(y - nHeightFFT / 2), 2));//分子,计算pow必须为float型
			float h = 1.0f / (1.0f + pow(d / sigma, 2 * n));
			if (useHigh)h = 1 - h;
			pFrequencyData1[y*nWidthFFT + x] *= h;
			pFrequencyData2[y*nWidthFFT + x] *= h;
			pFrequencyData3[y*nWidthFFT + x] *= h;
		}
	}

	//快速傅立叶变换
	IFFT_2D(pFrequencyData1, nWidthFFT, nHeightFFT, pSpaceDataB);
	IFFT_2D(pFrequencyData2, nWidthFFT, nHeightFFT, pSpaceDataG);
	IFFT_2D(pFrequencyData3, nWidthFFT, nHeightFFT, pSpaceDataR);

	pixel = 0;
	double dTemp;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			dTemp = pSpaceDataB[y * nWidthFFT + x].real() * pSpaceDataB[y * nWidthFFT + x].real() +
				pSpaceDataB[y * nWidthFFT + x].imag() * pSpaceDataB[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pSpaceDataG[y * nWidthFFT + x].real() * pSpaceDataG[y * nWidthFFT + x].real() +
				pSpaceDataG[y * nWidthFFT + x].imag() * pSpaceDataG[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pSpaceDataR[y * nWidthFFT + x].real() * pSpaceDataR[y * nWidthFFT + x].real() +
				pSpaceDataR[y * nWidthFFT + x].imag() * pSpaceDataR[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
}
void ImageELPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, float sigma, int n, bool useHigh) {


	int x = 0;
	int y = 0;
	int pixel = 0;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	complex<double> *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	/*初始化空间域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
			float d = sqrt(pow(float(x - nWidthFFT / 2), 2) + pow(float(y - nHeightFFT / 2), 2));//分子,计算pow必须为float型
			float h = exp(-pow(d / sigma,n));
			if (useHigh)h = 1 - h;
			pFrequencyData1[y*nWidthFFT + x] *= h;
			pFrequencyData2[y*nWidthFFT + x] *= h;
			pFrequencyData3[y*nWidthFFT + x] *= h;
		}
	}

	//快速傅立叶变换
	IFFT_2D(pFrequencyData1, nWidthFFT, nHeightFFT, pSpaceDataB);
	IFFT_2D(pFrequencyData2, nWidthFFT, nHeightFFT, pSpaceDataG);
	IFFT_2D(pFrequencyData3, nWidthFFT, nHeightFFT, pSpaceDataR);

	pixel = 0;
	double dTemp;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			dTemp = pSpaceDataB[y * nWidthFFT + x].real() * pSpaceDataB[y * nWidthFFT + x].real() +
				pSpaceDataB[y * nWidthFFT + x].imag() * pSpaceDataB[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pSpaceDataG[y * nWidthFFT + x].real() * pSpaceDataG[y * nWidthFFT + x].real() +
				pSpaceDataG[y * nWidthFFT + x].imag() * pSpaceDataG[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pSpaceDataR[y * nWidthFFT + x].real() * pSpaceDataR[y * nWidthFFT + x].real() +
				pSpaceDataR[y * nWidthFFT + x].imag() * pSpaceDataR[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
}
void ImageTLPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, int lowThresh, int highThresh, bool useHigh) {


	int x = 0;
	int y = 0;
	int pixel = 0;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	complex<double> *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	/*初始化空间域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
			float d = sqrt(pow(float(x - nWidthFFT / 2), 2) + pow(float(y - nHeightFFT / 2), 2));//分子,计算pow必须为float型
			float h = 1;
			if (d < highThresh) {
				if(d>lowThresh) {
					h = (d - highThresh) / (lowThresh - highThresh);
				}
			}
			else {
				h = 0;
			}
			if (useHigh)h = 1 - h;
			pFrequencyData1[y*nWidthFFT + x] *= h;
			pFrequencyData2[y*nWidthFFT + x] *= h;
			pFrequencyData3[y*nWidthFFT + x] *= h;
		}
	}

	//快速傅立叶变换
	IFFT_2D(pFrequencyData1, nWidthFFT, nHeightFFT, pSpaceDataB);
	IFFT_2D(pFrequencyData2, nWidthFFT, nHeightFFT, pSpaceDataG);
	IFFT_2D(pFrequencyData3, nWidthFFT, nHeightFFT, pSpaceDataR);

	pixel = 0;
	double dTemp;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			dTemp = pSpaceDataB[y * nWidthFFT + x].real() * pSpaceDataB[y * nWidthFFT + x].real() +
				pSpaceDataB[y * nWidthFFT + x].imag() * pSpaceDataB[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pSpaceDataG[y * nWidthFFT + x].real() * pSpaceDataG[y * nWidthFFT + x].real() +
				pSpaceDataG[y * nWidthFFT + x].imag() * pSpaceDataG[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pSpaceDataR[y * nWidthFFT + x].real() * pSpaceDataR[y * nWidthFFT + x].real() +
				pSpaceDataR[y * nWidthFFT + x].imag() * pSpaceDataR[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
}
void ImageGLPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, int thresh, bool useHigh) {


	int x = 0;
	int y = 0;
	int pixel = 0;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	complex<double> *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	/*初始化空间域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
			float d = sqrt(pow(float(x - nWidthFFT / 2), 2) + pow(float(y - nHeightFFT / 2), 2));//分子,计算pow必须为float型
			float h = exp(-0.5*(d / thresh)*(d / thresh));
			if (useHigh)h = 1 - h;
			pFrequencyData1[y*nWidthFFT + x] *= h;
			pFrequencyData2[y*nWidthFFT + x] *= h;
			pFrequencyData3[y*nWidthFFT + x] *= h;
		}
	}

	//快速傅立叶变换
	IFFT_2D(pFrequencyData1, nWidthFFT, nHeightFFT, pSpaceDataB);
	IFFT_2D(pFrequencyData2, nWidthFFT, nHeightFFT, pSpaceDataG);
	IFFT_2D(pFrequencyData3, nWidthFFT, nHeightFFT, pSpaceDataR);

	pixel = 0;
	double dTemp;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			dTemp = pSpaceDataB[y * nWidthFFT + x].real() * pSpaceDataB[y * nWidthFFT + x].real() +
				pSpaceDataB[y * nWidthFFT + x].imag() * pSpaceDataB[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pSpaceDataG[y * nWidthFFT + x].real() * pSpaceDataG[y * nWidthFFT + x].real() +
				pSpaceDataG[y * nWidthFFT + x].imag() * pSpaceDataG[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pSpaceDataR[y * nWidthFFT + x].real() * pSpaceDataR[y * nWidthFFT + x].real() +
				pSpaceDataR[y * nWidthFFT + x].imag() * pSpaceDataR[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
}
void ImageGLPFP(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, int thresh, float k,float c) {


	int x = 0;
	int y = 0;
	int pixel = 0;
	//指向空域数据的指针
	complex<double>* pSpaceDataB, *pSpaceDataG, *pSpaceDataR;
	//分配存储空间
	pSpaceDataB = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataG = new complex<double>[nWidthFFT * nHeightFFT];
	pSpaceDataR = new complex<double>[nWidthFFT * nHeightFFT];

	complex<double> *pFrequencyData1, *pFrequencyData2, *pFrequencyData3;
	pFrequencyData1 = pFrequencyData;
	pFrequencyData2 = pFrequencyData + nWidthFFT * nHeightFFT;
	pFrequencyData3 = pFrequencyData2 + nWidthFFT * nHeightFFT;
	/*初始化空间域数据*/
	for (y = 0; y < nHeightFFT; ++y)
	{
		for (x = 0; x < nWidthFFT; ++x)
		{
			pSpaceDataB[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataG[y*nWidthFFT + x] = complex<double>(0, 0);
			pSpaceDataR[y*nWidthFFT + x] = complex<double>(0, 0);
			float d = sqrt(pow(float(x - nWidthFFT / 2), 2) + pow(float(y - nHeightFFT / 2), 2));//分子,计算pow必须为float型
			float h = exp(-0.5*(d / thresh)*(d / thresh));
			h = 1 - h;
			h = h * k + c;
			pFrequencyData1[y*nWidthFFT + x] *= h;
			pFrequencyData2[y*nWidthFFT + x] *= h;
			pFrequencyData3[y*nWidthFFT + x] *= h;
		}
	}

	//快速傅立叶变换
	IFFT_2D(pFrequencyData1, nWidthFFT, nHeightFFT, pSpaceDataB);
	IFFT_2D(pFrequencyData2, nWidthFFT, nHeightFFT, pSpaceDataG);
	IFFT_2D(pFrequencyData3, nWidthFFT, nHeightFFT, pSpaceDataR);

	pixel = 0;
	double dTemp;
	for (y = 0; y < m_nPicHeight; ++y)
	{
		for (x = 0; x < m_nPicWidth; ++x, pixel += 4)
		{
			dTemp = pSpaceDataB[y * nWidthFFT + x].real() * pSpaceDataB[y * nWidthFFT + x].real() +
				pSpaceDataB[y * nWidthFFT + x].imag() * pSpaceDataB[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel] = (unsigned char)dTemp;

			dTemp = pSpaceDataG[y * nWidthFFT + x].real() * pSpaceDataG[y * nWidthFFT + x].real() +
				pSpaceDataG[y * nWidthFFT + x].imag() * pSpaceDataG[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 1] = (unsigned char)dTemp;

			dTemp = pSpaceDataR[y * nWidthFFT + x].real() * pSpaceDataR[y * nWidthFFT + x].real() +
				pSpaceDataR[y * nWidthFFT + x].imag() * pSpaceDataR[y * nWidthFFT + x].imag();
			dTemp = sqrt(dTemp);
			if (dTemp > 255)dTemp = 255;
			imageData[pixel + 2] = (unsigned char)dTemp;
		}
	}
	delete[]pSpaceDataB;
	delete[]pSpaceDataG;
	delete[]pSpaceDataR;
}

void ImageHisEqu(unsigned char * imageData, int width, int height)
{
	int gray[3][256] = { 0 };  //记录每个灰度级别下的像素个数
	double gray_prob[3][256] = { 0 };  //记录灰度分布密度
	double gray_distribution[3][256] = { 0 };  //记录累计密度
	int gray_equal[3][256] = { 0 };  //均衡化后的灰度值
	int gray_sum = width*height;  //像素总数
	GetHisInfo(imageData, width, height, gray, gray_prob, gray_distribution);
	//重新计算均衡化后的灰度值，四舍五入。参考公式：(N-1)*T+0.5
	for (int i = 0; i < 256; i++)
	{
		for (int k = 0; k < 3; k++) {
			gray_equal[k][i] = (unsigned char)(255 * gray_distribution[k][i] + 0.5);
		}
	}
	//直方图均衡化,更新原图每个点的像素值
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < 3; k++) {
				int index = (i * width + j)*4+k;
				imageData[index] = gray_equal[k][imageData[index]];
			}
		}
	}
}

void ImageHisSML(unsigned char * imageData, int width, int height, double zhist[][256])
{
	int gray[3][256] = { 0 };  //记录每个灰度级别下的像素个数
	double gray_prob[3][256] = { 0 };  //记录灰度分布密度
	double gray_distribution[3][256] = { 0 };  //记录累计密度
	int gray_equal[3][256] = { 0 };  //均衡化后的灰度值
	int gray_sum = width * height;  //像素总数
	GetHisInfo(imageData, width, height, gray, gray_prob, gray_distribution);
	int G[3][256] = { 0 };
	float zsumhist[3][256] = { 0.0 };
	zsumhist[0][0] = zhist[0][0];
	zsumhist[1][0] = zhist[1][0];
	zsumhist[2][0] = zhist[2][0];
	for (int i = 0; i < 256; i++) {
		for (int k = 0; k < 3; k++) {
			zsumhist[k][i] =  zhist[k][i];
		}
	}
	for (int k = 0; k < 3; k++) {
		for (int i = 0; i < 256; i++) {
			if (gray_distribution[k][i] < 0.00000001)continue;
			float minD = 2;
			for (int j = 0; j < 256; j++) {
				if (zsumhist[k][j] < 0.00000001)continue;
				float tmp = zsumhist[k][j] - gray_distribution[k][i];
				if (tmp > 0) {
					if (tmp < minD) {
						G[k][i] = j;
						break;
					}
				}
				else {
					if (fabs(tmp) < minD) {
						G[k][i] = j;
						minD = fabs(tmp);
					}
				}
			}
		}
	}
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < 3; k++) {
				int index = (i * width + j) * 4 + k;
				imageData[index] = G[k][imageData[index]];
			}
		}
	}

}

void ImageHisGML(unsigned char * imageData, int width, int height, double zhist[][256])
{
	int gray[3][256] = { 0 };  //记录每个灰度级别下的像素个数
	double gray_prob[3][256] = { 0 };  //记录灰度分布密度
	double gray_distribution[3][256] = { 0 };  //记录累计密度
	int gray_equal[3][256] = { 0 };  //均衡化后的灰度值
	int gray_sum = width * height;  //像素总数
	GetHisInfo(imageData, width, height, gray, gray_prob, gray_distribution);
	int G[3][256] = { 0 };
	float zsumhist[3][256] = { 0.0 };
	zsumhist[0][0] = zhist[0][0];
	zsumhist[1][0] = zhist[1][0];
	zsumhist[2][0] = zhist[2][0];
	for (int i = 0; i < 256; i++) {
		for (int k = 0; k < 3; k++) {
			zsumhist[k][i] = zhist[k][i];
		}
	}
	for (int k = 0; k < 3; k++) {
		int last = 0;
		for (int i = 0; i < 256; i++) {
			if (zsumhist[k][i] < 0.00000001)continue;
			float minD = 2;
			int id = 0;
			for (int j = 0; j < 256; j++) {
				if (gray_distribution[k][j] < 0.00000001)continue;
				float tmp = fabs(gray_distribution[k][j] - zsumhist[k][i]);
				if (tmp < minD) {
					minD = tmp;
					id = j;
				}
			}
			for (int m = last; m <= id; m++) {
				G[k][m] = i;
			}
			last = id+1;
		}
	}
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < 3; k++) {
				int index = (i * width + j) * 4 + k;
				imageData[index] = G[k][imageData[index]];
			}
		}
	}
}

void GetHisInfo(unsigned char * imageData, int width, int height, int gray[][256], double gray_prob[][256], double gray_distribution[][256])
{
	int gray_sum = width * height;  //像素总数
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			for (int k = 0; k < 3; k++) {
				gray[k][imageData[(i*width + j) * 4 + k]]++;
			}
		}
	}
	for (int i = 0; i < 256; i++) {
		for (int k = 0; k < 3; k++) {
			gray_prob[k][i] = (double)gray[k][i] / gray_sum;
		}
	}
	//计算累计密度
	gray_distribution[0][0] = gray_prob[0][0];
	gray_distribution[1][0] = gray_prob[1][0];
	gray_distribution[2][0] = gray_prob[2][0];
	for (int i = 1; i < 256; i++)
	{
		for (int k = 0; k < 3; k++) {
			gray_distribution[k][i] = gray_distribution[k][i - 1] + gray_prob[k][i];
		}
	}
}

void SharpLaplacian(unsigned char * imageData, int width, int height, float threash)
{
	unsigned char *newImage;
	newImage = new unsigned char[width * height * 4];
	memcpy(newImage, imageData, width * height * 4);
	int kernel[3][3] = {
	{-1,-1,-1},
	{-1,8,-1},
	{-1,-1,-1} };
	int sizeKernel = 3;
	int sumKernel = 3;
	for (int x = sizeKernel / 2; x < width - sizeKernel / 2; x++)
	{
		for (int y = sizeKernel / 2; y < height- sizeKernel / 2; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++)
			{
				for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++)
				{
					int index = ((y + i)*width + x + j) * 4;
					r += newImage[index] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					g += newImage[index+1] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					b += newImage[index+2] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
				}
			}
			int index = (y*width + x) * 4;
			r = r * threash + newImage[index];
			g = g * threash + newImage[index + 1];
			b = b * threash + newImage[index + 2];
			imageData[index]  = max(0, min(255, r));
			imageData[index + 1] = max(0, min(255, g));
			imageData[index + 2] = max(0, min(255, b));
		}
	}
	delete[] newImage;
	newImage = NULL;
}
void ImageGrad(unsigned char * imageData, int width, int height, float ratio,float threash, int flag) {
	unsigned char *newImage;
	newImage = new unsigned char[width * height * 4];
	memcpy(newImage, imageData, width * height * 4);
	for (int x = 0; x < width - 1; x++)
	{
		for (int y = 0; y < height - 1; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			int index0 = (y*width + x) * 4;
			int index1 = (y*width + x + 1) * 4;
			int index2 = ((y+1)*width + x) * 4;
			int rx = newImage[index0] - newImage[index1];
			int ry = newImage[index0] - newImage[index2];
			int gx = newImage[index0+1] - newImage[index1+1];
			int gy = newImage[index0+1] - newImage[index2+1];
			int bx = newImage[index0+2] - newImage[index1+2];
			int by = newImage[index0+2] - newImage[index2+2];
			if (flag == 0) {

				r = sqrt(rx * rx + ry * ry);
				g = sqrt(gx * gx + gy * gy);
				b = sqrt(bx * bx + by * by);
			}
			else if(flag==1){
				r = abs(newImage[index0] - newImage[index1])+ abs(newImage[index0] - newImage[index2]);
				g = abs((newImage[index0 + 1] - newImage[index1]) + abs(newImage[index0 + 1] - newImage[index2 + 1]));
				b = abs((newImage[index0 + 2] - newImage[index1]) + abs(newImage[index0 + 2] - newImage[index2 + 2]));
			}
			else if (flag == 2) {
				r = max(abs(newImage[index0] - newImage[index1]) , abs(newImage[index0] - newImage[index2]));
				g = max(abs(newImage[index0 + 1] - newImage[index1]) , abs(newImage[index0 + 1] - newImage[index2 + 1]));
				b = max(abs(newImage[index0 + 2] - newImage[index1]) , abs(newImage[index0 + 2] - newImage[index2 + 2]));
			}
			if (r < threash)r = 0;
			if (g < threash)g = 0;
			if (b < threash)b = 0;

			r = r * ratio + newImage[index0];
			g = g * ratio + newImage[index0 + 1];
			b = b * ratio + newImage[index0 + 2];


			imageData[index0] = max(0, min(255, r));
			imageData[index0 + 1] =  max(0, min(255, g));
			imageData[index0 + 2] =  max(0, min(255, b));
		}
	}
	delete[] newImage;
	newImage = NULL;
}
void ImagePrewitt(unsigned char * imageData, int width, int height, float ratio, float threash,int flag)
{
	unsigned char *newImage;
	newImage = new unsigned char[width * height * 4];
	memcpy(newImage, imageData, width * height * 4);
	int kernelX[3][3] = {
	{1,0,-1},
	{1,0,-1},
	{1,0,-1} };
	int kernelY[3][3] = {
	{1,1,1},
	{0,0,0},
	{-1,-1,-1} };
	int sizeKernel = 3;
	int sumKernel = 3;
	for (int x = sizeKernel / 2; x < width - sizeKernel / 2; x++)
	{
		for (int y = sizeKernel / 2; y < height - sizeKernel / 2; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			int rx = 0,ry = 0,gx= 0,gy = 0,bx=0,by=0;
			for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++)
			{
				for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++)
				{
					int index = ((y + i)*width + x + j) * 4;
					rx += newImage[index] * kernelX[sizeKernel / 2 + i][sizeKernel / 2 + j];
					gx += newImage[index + 1] * kernelX[sizeKernel / 2 + i][sizeKernel / 2 + j];
					bx += newImage[index + 2] * kernelX[sizeKernel / 2 + i][sizeKernel / 2 + j];

					ry += newImage[index] * kernelY[sizeKernel / 2 + i][sizeKernel / 2 + j];
					gy += newImage[index + 1] * kernelY[sizeKernel / 2 + i][sizeKernel / 2 + j];
					by += newImage[index + 2] * kernelY[sizeKernel / 2 + i][sizeKernel / 2 + j];
				}
			}
			if (flag == 0) {
				r = sqrt(rx * rx + ry * ry);
				g = sqrt(gx * gx + gy * gy);
				b = sqrt(bx * bx + by * by);
			}
			else if (flag == 1) {
				r = abs(rx) + abs(ry);
				g = abs(gx) + abs(gy);
				b = abs(bx) + abs(by);
			}
			else if (flag == 2) {
				r = max(abs(rx), abs(ry));
				g = max(abs(gx), abs(gy));
				b = max(abs(bx), abs(by));
			}
			if (r < threash)r = 0;
			if (g < threash)g = 0;
			if (b < threash)b = 0;

			int index = (y*width + x) * 4;
			r = r * ratio + newImage[index];
			g = g * ratio + newImage[index + 1];
			b = b * ratio + newImage[index + 2];

			imageData[index] = max(0, min(255, r));
			imageData[index + 1] = max(0, min(255, g));
			imageData[index + 2] = max(0, min(255, b));
		}
	}
	delete[] newImage;
	newImage = NULL;
}
void ImageSobel(unsigned char * imageData, int width, int height, float ratio, float threash, int flag)
{
	unsigned char *newImage;
	newImage = new unsigned char[width * height * 4];
	memcpy(newImage, imageData, width * height * 4);
	int kernelX[3][3] = {
	{1,0,-1},
	{2,0,-2},
	{1,0,-1} };
	int kernelY[3][3] = {
	{1,2,1},
	{0,0,0},
	{-1,-2,-1} };
	int sizeKernel = 3;
	int sumKernel = 3;
	for (int x = sizeKernel / 2; x < width - sizeKernel / 2; x++)
	{
		for (int y = sizeKernel / 2; y < height - sizeKernel / 2; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			int rx = 0, ry = 0, gx = 0, gy = 0, bx = 0, by = 0;
			for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++)
			{
				for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++)
				{
					int index = ((y + i)*width + x + j) * 4;
					rx += newImage[index] * kernelX[sizeKernel / 2 + i][sizeKernel / 2 + j];
					gx += newImage[index + 1] * kernelX[sizeKernel / 2 + i][sizeKernel / 2 + j];
					bx += newImage[index + 2] * kernelX[sizeKernel / 2 + i][sizeKernel / 2 + j];

					ry += newImage[index] * kernelY[sizeKernel / 2 + i][sizeKernel / 2 + j];
					gy += newImage[index + 1] * kernelY[sizeKernel / 2 + i][sizeKernel / 2 + j];
					by += newImage[index + 2] * kernelY[sizeKernel / 2 + i][sizeKernel / 2 + j];
				}
			}
			if (flag == 0) {
				r = sqrt(rx * rx + ry * ry);
				g = sqrt(gx * gx + gy * gy);
				b = sqrt(bx * bx + by * by);
			}
			else if (flag == 1) {
				r = abs(rx) + abs(ry);
				g = abs(gx) + abs(gy);
				b = abs(bx) + abs(by);
			}
			else if (flag == 2) {
				r = max(abs(rx), abs(ry));
				g = max(abs(gx), abs(gy));
				b = max(abs(bx), abs(by));
			}
			if (r < threash)r = 0;
			if (g < threash)g = 0;
			if (b < threash)b = 0;

			int index = (y*width + x) * 4;
			r = r * ratio + newImage[index];
			g = g * ratio + newImage[index + 1];
			b = b * ratio + newImage[index + 2];

			imageData[index] = max(0, min(255, r));
			imageData[index + 1] = max(0, min(255, g));
			imageData[index + 2] = max(0, min(255, b));
		}
	}
	delete[] newImage;
	newImage = NULL;
}
void ImageQualc(unsigned char * imageData, int width, int height, float threash)
{
	unsigned char *newImage;
	newImage = new unsigned char[width * height * 4];
	memcpy(newImage, imageData, width * height * 4);
	int kernel[3][3] = {
	{1,-2,1},
	{-2,5,-2},
	{1,-2,1} };
	int sizeKernel = 3;
	int sumKernel = 3;
	for (int x = sizeKernel / 2; x < width - sizeKernel / 2; x++)
	{
		for (int y = sizeKernel / 2; y < height - sizeKernel / 2; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++)
			{
				for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++)
				{
					int index = ((y + i)*width + x + j) * 4;
					r += newImage[index] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					g += newImage[index + 1] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					b += newImage[index + 2] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
				}
			}
			int index = (y*width + x) * 4;
			r = r * threash + newImage[index];
			g = g * threash + newImage[index + 1];
			b = b * threash + newImage[index + 2];
			imageData[index] = max(0, min(255, r));
			imageData[index + 1] = max(0, min(255, g));
			imageData[index + 2] = max(0, min(255, b));
		}
	}
	delete[] newImage;
	newImage = NULL;
}

void ImageLog(unsigned char * imageData, int width, int height, float threash)
{
	unsigned char *newImage;
	newImage = new unsigned char[width * height * 4];
	memcpy(newImage, imageData, width * height * 4);
	int kernel[5][5] = {
	{0,0,-1,0,0},
	{0,-1,-2,-1,0},
	{-1,-2,16,-2,-1},
	{0,-1,-2,-1,0}, 
	{0,0,-1,0,0}};
	int sizeKernel = 5;
	int sumKernel = 5;
	for (int x = sizeKernel / 2; x < width - sizeKernel / 2; x++)
	{
		for (int y = sizeKernel / 2; y < height - sizeKernel / 2; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++)
			{
				for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++)
				{
					int index = ((y + i)*width + x + j) * 4;
					r += newImage[index] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					g += newImage[index + 1] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
					b += newImage[index + 2] * kernel[sizeKernel / 2 + i][sizeKernel / 2 + j];
				}
			}
			int index = (y*width + x) * 4;
			r = r * threash + newImage[index];
			g = g * threash + newImage[index + 1];
			b = b * threash + newImage[index + 2];
			imageData[index] = max(0, min(255, r));
			imageData[index + 1] = max(0, min(255, g));
			imageData[index + 2] = max(0, min(255, b));
		}
	}
	delete[] newImage;
	newImage = NULL;
}

void ImageExpT(unsigned char * imageData,int width,int height, float contrastThresh)
{
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			int index = (y * width + x) * 4;
			imageData[index] = pow(imageData[index] / 255.0f, contrastThresh) * 255.0f;
			imageData[index + 1] = pow(imageData[index + 1] / 255.0f, contrastThresh) * 255.0f;
			imageData[index + 2] = pow(imageData[index + 2] / 255.0f, contrastThresh) * 255.0f;
		}
	}
}
void ImageLogT(unsigned char * imageData, int width, int height, float contrastThresh)
{
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			int index = (y * width + x) * 4;
			float c = 255.0f / log(1 + 255);
			imageData[index] = c * log(imageData[index] + 1.0f);
			imageData[index + 1] = c * log(imageData[index + 1] + 1.0f);
			imageData[index + 2] = c * log(imageData[index + 2] + 1.0f);
		}
	}
}
