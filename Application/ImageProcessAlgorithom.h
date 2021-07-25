#pragma once
#include <complex> 
#include <iostream>
using namespace std;
#define PI 3.1425926
void FFT_1D(complex<double> * pCTData, complex<double> * pCFData, int nLevel);
void FFT_2D(complex<double> * pSpaceData, complex<double> * pFrequencyData, int nWidthFFT, int nHeightFFT);
void IFFT_1D(complex<double> * pCFData, complex<double> * pCTData, int nLevel);
void IFFT_2D(complex<double> *pFrequencyData, int nWidthFFT, int nHeightFFT, complex<double> *pSpaceData);
complex<double>* ImageFFT(unsigned char* imageData, int m_nPicWidth, int m_nPicHeight, int& nWidthFFT, int& nHeightFFT);
void ImageIFFT(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight);
void ImageILPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight,int thresh, bool useHigh = false);
void ImageBLPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, float sigma, int n, bool useHigh = false);
void ImageELPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, float sigma, int n, bool useHigh = false);
void ImageTLPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, int lowThresh, int highThresh, bool useHigh = false);
void ImageGLPF(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, int thresh, bool useHigh = false);
void ImageGLPFP(unsigned char *imageData, complex<double>* pFrequencyData, int nWidthFFT, int nHeightFFT, int m_nPicWidth, int m_nPicHeight, int thresh, float k, float c);