/*
Copyright (C) 2014-15 Andrea Vedaldi.
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.

Some of this  file is part of the VLFeat library and is made available under
the terms of the BSD license (see the COPYING file).
*/
#include "NormalizationLayer.h"

#define VL_NNNORMALIZE_FAST
#define max(a,b) (((a)>=(b))?a:b)
#define xat(t) x[(t) * offset]
#define yat(t) y[(t) * offset]
#define zat(t) z[(t) * offset]

#ifndef VL_NNNORMALIZE_FAST
inline double fast_pow(double a, double b) { return pow(a,b) ; }
inline float fast_pow(float a, float b) { return powf(a, b); }
#else
//#define VERY_FAST
#ifndef VERY_FAST
inline double fast_pow(double x, double y)
{
	double z;
	double const plog3 = 0.164042561333445;
	double const plog2 = -0.606737602222409;
	double const plog1 = 1.442695040888963;
	double const pexp3 = 0.079441541679836;
	double const pexp2 = 0.227411277760219;
	double const pexp1 = 0.693147180559945;
	typedef int long long int_t;
	const int_t offset = 1023LL << 52;

	int_t ix = *(int_t*)&x - offset;
	int_t imx = (ix & ((1LL << 52) - 1LL)) + offset;
	double fx = (double)(ix >> 52);
	double mx = *((double*)&imx) - 1;
	double mx2 = mx*mx;
	double mx3 = mx2*mx;
	double t = y * (fx + mx*plog1 + mx2*plog2 + mx3*plog3);
	//  double t = y * (fx + mx) ;

	double fz = floor(t);
	double rz = t - fz;
	double rz2 = rz*rz;
	double rz3 = rz2*rz;
	double tz = fz + rz*pexp1 + rz2*pexp2 + rz3*pexp3;
	// double tz = fz + rz ;

	//  mexPrintf("%g %g -- ix %ld imx %ld fx %g mx %g t %g\n", x,y, ix,imx, fx, mx, t) ;
	*((int_t*)&z) = (int_t)(tz * (1LL << 52)) + offset;
	//z = exp(t * log(2.0)) ;
	return z;
}
#else
inline double fast_pow(double a, double b)
{
	double z ;
	typedef int int int int_t;
	const int_t offset = 1023L << 52 ;
	int_t ai = *((int_t*)&a) ;
	*((int_t*)&z) = (int_t)(b * (ai - offset)) + offset ;
	return z ;
}
#endif

#ifndef VERY_FAST
inline float fast_pow(float x, float y)
{
	float z;
	float const plog3 = 0.164042561333445F;
	float const plog2 = -0.606737602222409F;
	float const plog1 = 1.442695040888963F;
	float const pexp3 = 0.079441541679836F;
	float const pexp2 = 0.227411277760219F;
	float const pexp1 = 0.693147180559945F;
	typedef int int_t;
	const int_t offset = 127 << 23;

	int_t ix = *(int_t*)&x - offset;
	int_t imx = (ix & ((1 << 23) - 1)) + offset;
	float fx = (float)(ix >> 23);
	float mx = *((float*)&imx) - 1;
	float mx2 = mx*mx;
	float mx3 = mx2*mx;
	float t = y * (fx + mx*plog1 + mx2*plog2 + mx3*plog3);

	float fz = floor(t);
	float rz = t - fz;
	float rz2 = rz*rz;
	float rz3 = rz2*rz;
	float tz = fz + rz*pexp1 + rz2*pexp2 + rz3*pexp3;

	*((int_t*)&z) = (int_t)(tz * (1 << 23)) + offset;
	return z;
}
#else
inline float fast_pow(float a, float b)
{
	float z ;
	typedef int int_t;
	const int_t offset = 127 << 23 ;
	int_t ai = *((int_t*)&a) ;
	*((int_t*)&z) = (int_t)(b * (ai - offset)) + offset ;
	return z ;
}
#endif
#endif


NormalizationLayer::NormalizationLayer()
{
	m_param[0] = m_param[1] = m_param[2] = m_param[3]=-1.0;
}


NormalizationLayer::~NormalizationLayer()
{
}

bool NormalizationLayer::initiStructureFromFile(char* &sin){
	if (!sin)
		return false;
	//load type , name and param
	char str[256];
	read<char>(sin, str);
	m_type = str;
	read<char>(sin, str);
	m_name = str;
	read<double>(sin, m_param);
	return true;
}
bool NormalizationLayer::setInput(float* inputData, int  dataDims[4]){
	int size1, size2, size3, size4;
	size1 = dataDims[0];
	size2 = size1*dataDims[1];
	size3 = size2*dataDims[2];
	size4 = size3*dataDims[3];
	for (int i = 0; i < 4; ++i){
		m_inputDims[i] = dataDims[i];
	}
	memcpy(m_inputData,inputData,sizeof(float)*size4);
	return true;
}
bool NormalizationLayer::setInput(Feature* &featCache, bool flag, int  dataDims[4])
{
	if (!featCache || !featCache->buf[0] || !featCache->buf[1])
		return false;
	memcpy(m_inputDims,dataDims, sizeof(int) * 4);
	memcpy(m_outputDims, dataDims, sizeof(int) * 4);
	m_inputData = featCache->buf[flag];
	m_outputData = featCache->buf[!flag];
	return true;
}
void NormalizationLayer::getOutPutDims(int* dims){
	memcpy(dims, m_outputDims, sizeof(int) * 4);
}
bool NormalizationLayer::getOutput(float* &data, int(&dataDims)[4]){
	if (data != NULL){
		delete[]data;
	}
	int size = 1;
	for (int i = 0; i < 4; ++i){
		dataDims[i] = m_outputDims[i];
		size *= m_outputDims[i];
	}
	memcpy(data, m_outputData, sizeof(float)*size);
	return true;
}

/*
performs feature-wise sliding window
normalization of the image X. The normalized output is given by:

Y(i,j,k) = X(i,j,k) / L(i,j,k)^BETA

where the normalising factor is

L(i,j,k) = KAPPA + ALPHA * (sum_{q in Q(k)} X(i,j,k)^2,

PARAM = [N KAPPA ALPHA BETA], and N is the size of the window. The
window Q(k) itself is defined as:

Q(k) = [max(1, k-FLOOR((N-1)/2)), min(D, k+CEIL((N-1)/2))].

where D is the number of feature dimensions in X. Note in
particular that, by setting N >= 2D, the function can be used to
normalize the whole feature vector.

Krizhevsky A, Sutskever I, Hinton G E. Imagenet classification with deep convolutional neural networks[C]
//Advances in neural information processing systems. 2012: 1097-1105.

*/
bool NormalizationLayer::applyNormalization()
{
	if (!checkParamBeforeApply())
		return false;
	int normDepth = static_cast<int>(m_param[0]);

	int t;
	int m1 = ((signed)normDepth - 1) / 2;
	int m2 = (int)normDepth - m1 - 1;
	int offset = (int)m_inputDims[1]*(int)m_inputDims[0];
	float *result = m_outputData;
	float *src = m_inputData;
#ifndef VL_NNNORMALIZE_FAST
	for (int k = 0; k < m_inputDims[3]; ++k) {
		for (int h = 0; h < m_inputDims[0]; ++h) {
			for (int w = 0; w < m_inputDims[1]; ++w) {
				float const* x = src + w + h * m_inputDims[1];
				float* y = result + w + h * m_inputDims[1];
				float acc = 0;
				for (t = -m2; t < (signed)m_inputDims[2]; ++t) {
					float ap = 0;
					float am = 0;
					if (t - m1 - 1 >= 0) { am = xat(t - m1 - 1); }
					if (t + m2 < m_inputDims[2]) { ap = xat(t + m2); }
					acc += ap*ap - am*am;
					if (0 <= t && t < m_inputDims[2]) {
						yat(t) = xat(t) * fast_pow(kappa + alpha * acc, -beta);
					}
				}
			}
		}
		src += m_inputDims[1] * m_inputDims[0] * m_inputDims[2];
		result += m_inputDims[1] * m_inputDims[0] * m_inputDims[2];
	}
#else
	float * acc = (float*)calloc(sizeof(float), m_inputDims[1]*m_inputDims[0]);
	for (int k = 0; k < m_inputDims[3]; ++k) {
		memset(acc, 0, sizeof(float) * m_inputDims[1]*m_inputDims[0]) ;
		for (t = -m2 ; t < (signed)m_inputDims[2] ; ++t) {
			int tm = t - m1 - 1 ;
			int tp = t + m2 ;
			float const* xam = src + offset * (t - m1 - 1);
			float const* xap = src + offset * (t + m2);
			float *end = acc + m_inputDims[1]*m_inputDims[0] ;
			if (0 <= tm && tp < m_inputDims[2]) {
				for(float *xacc = acc ; xacc != end ; ++xacc, ++xam, ++xap) {
					float am = *xam ;
					float ap = *xap ;
					*xacc += ap*ap - am*am ;
				}
			} else if (0 > tm && tp < m_inputDims[2]) {
				for(float *xacc = acc ; xacc != end ; ++xacc, ++xap) {
					float ap = *xap ;
					*xacc += ap*ap ;
				}
			} else if (0 <= tm && tp >= m_inputDims[2]) {
				for(float *xacc = acc ; xacc != end ; ++xacc, ++xam) {
					float am = *xam ;
					*xacc -= am*am ;
				}
			}
			if (0 <= t && t < m_inputDims[2]) {
				float const* xx = src + offset * t;
				float* xy = result + offset * t;
				for(float *xacc = acc ; xacc != end ; ++xacc, ++xx, ++xy) {
					//suggest use float __cdecl powf(float _X,float _Y) in linux.
					(*xy) = (*xx) * (float)fast_pow(m_param[1] + m_param[2] * (*xacc), -m_param[3]);
				}
			}
		}
		src += m_inputDims[1] * m_inputDims[0] * m_inputDims[2];
		result += m_inputDims[1] * m_inputDims[0] * m_inputDims[2];
	}
	free(acc) ;
#endif
	return true;
}


bool NormalizationLayer::checkParamBeforeApply()
{
	if (!m_inputData)
		return false;
	bool flag = true;
	for (int i = 0; i < 4; ++i)
		flag = flag&&(m_param[i] >= 0.0);
	if (!flag)
		return false;
	return true;
}


// implemented from base class
bool NormalizationLayer::execute()
{
	if (!applyNormalization())
		return false;

	return true;
}
