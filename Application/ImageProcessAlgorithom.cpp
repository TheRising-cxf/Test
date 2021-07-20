#include "ImageProcessAlgorithom.h"
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
