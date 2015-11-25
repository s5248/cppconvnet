/*
Copyright (C) 2014 Andrea Vedaldi and Max Jaderberg
Copyright (C) 2015 Andrea Vedaldi.
Copyright (C) 2015 BiaoZhi Huang.

All rights reserved.

Some of this  file is part of the VLFeat library and is made available under
the terms of the BSD license (see the COPYING file).
*/
#include "ConvLayer.h"
#include<algorithm>


static inline int floor_divide(int a, int b);
static inline int ceil_divide(int a, int b);

ConvLayer::ConvLayer()
{
	m_biasDims = 0;
	m_filters = NULL;
	m_bias = NULL;
	allones = new float[54 * 54];
	for (int i = 0; i < 54 * 54; ++i)
		allones[i] = 1.0f;
}


ConvLayer::~ConvLayer()
{
	if (m_filters) {
		delete[]m_filters;
		m_filters = NULL;
	}
	if (m_bias) {
		delete[]m_bias;
		m_bias = NULL;
	}
	if (allones)
	{
		delete[]allones;
		allones = NULL;
	}
}

// used before getOutputData operation
bool ConvLayer::setOutputDataMem()
{
	//set outpuDataDims
	m_outputDims[0] = (m_inputDims[0] + (m_pad[0] + m_pad[1]) - m_filterDims[0]) / m_stride[0] + 1;
	m_outputDims[1] = (m_inputDims[1] + (m_pad[2] + m_pad[3]) - m_filterDims[1]) / m_stride[1] + 1;
	m_outputDims[2] = m_filterDims[3];
	m_outputDims[3] = m_inputDims[3];
	return true;
}

//alpha*A*B = C.A is :size2*size4,B is size4*size5,C is size2*size5(eg:54*54*64)
bool ConvLayer::applyConvolution(float dataMult=1.0, float outputMult=0.0)
{
	if (!checkParamBeforeApply())
		return false;
	int numGroups = m_inputDims[2] / m_filterDims[2];//1
	bool fullyConnectedMode = (m_outputDims[0] == 1 &&
		m_outputDims[1] == 1 &&
		m_stride[0] == 1 &&
		m_stride[1] == 1 &&
		m_pad[0] == 0 &&
		m_pad[1] == 0 &&
		m_pad[2] == 0 &&
		m_pad[3] == 0 &&
		numGroups == 1);
	if (fullyConnectedMode){
		nnfullyconnected_forward();
		return true;
	}
	int numFiltersPerGroup = m_filterDims[3] / numGroups;
	int numOutputPixels = m_outputDims[0] * m_outputDims[1];
	int filtersVolume = m_filterDims[0] * m_filterDims[1] * m_filterDims[2];
	int tempVolume = numOutputPixels * filtersVolume * numGroups;
	float* tempMemory = new float[tempVolume];
	float* tempOnes = new float[numOutputPixels];
	memcpy(tempOnes, allones, sizeof(float)*numOutputPixels);
	for (int image = 0; image < m_inputDims[3]; ++image) {
		int dataOffset = (m_inputDims[0] * m_inputDims[1] * m_inputDims[2]) * image;//0
		int outputOffset = (m_outputDims[0] * m_outputDims[1] * m_outputDims[2]) * image;//0	
		//under sample
		im2row(tempMemory, m_inputData + dataOffset, m_outputDims[1], m_outputDims[0],filtersVolume);
		for (int g = 0; g < numGroups; ++g) {
			int filterGrpOffset = filtersVolume * numFiltersPerGroup * g;//0
			int tempGrpOffset = numOutputPixels * filtersVolume * g;//0
			int outputGrpOffset = numOutputPixels * numFiltersPerGroup * g;//0
			float alpha = dataMult;
			float beta = outputMult;
			//convolution
			cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, numOutputPixels, numFiltersPerGroup, filtersVolume, alpha,
				tempMemory + tempGrpOffset, numOutputPixels, m_filters + filterGrpOffset, filtersVolume, beta,
				m_outputData + outputOffset + outputGrpOffset, numOutputPixels);

		}
		if (m_bias!=NULL) {
			float alpha = 1;
			float beta = 1;
			int k = 1;
			//add bias
			cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, numOutputPixels, m_biasDims, k, alpha, tempOnes, numOutputPixels,
				m_bias, k, beta, m_outputData + outputOffset, numOutputPixels);
		}
	}
	delete[] tempMemory;
	delete[]tempOnes;
	return true;
}


bool ConvLayer::initiStructureFromFile(char* &sin){
	if (!sin)
		return false;
	//load filters
	int dims[4],size=1;
	read<int>(sin, (int*)dims);
	for (int i = 0; i < 4; ++i){
		m_filterDims[i] = dims[i];
		size *= dims[i];
	}
	m_filters = new float[size];
	read<float>(sin, m_filters);
	
	//load bias length and  data
	read<int>(sin, (int*)&m_biasDims);
	m_bias = new float[m_biasDims];
	read<float>(sin, m_bias);
	//load pad values
	read<int>(sin, m_pad);
	//load type and name.
	read<char>(sin, (char*)m_type.c_str());
	read<char>(sin, (char*)m_name.c_str());
	//load stride
	int stride[2];
	read<int>(sin,stride);
	for (int i = 0; i < 2; ++i){
		m_stride[i] = (int)stride[i];
	}
	return true;
}

bool ConvLayer::setInput(float* inputData, int  dataDims[4]){
	int size1, size2, size3, size4;
	size1 = dataDims[0];
	size2 = size1*dataDims[1];
	size3 = size2*dataDims[2];
	size4 = size3*dataDims[3];
	memcpy(m_inputDims,dataDims,sizeof(int)*4);
	return true;
}
bool ConvLayer::getOutput(float* &data, int(&dataDims)[4]){
	if (data != NULL){
		delete[]data;
		data = NULL;
	}
	int size = 1;
	for (int i = 0; i < 4; ++i){
		dataDims[i] = m_outputDims[i];
		size *= m_outputDims[i];
	}
	data = m_outputData;
	return true;
}
bool ConvLayer::setInput(Feature* &featCache, bool flag, int  dataDims[4])
{
	if (!featCache || !featCache->buf[flag])
		return false;
	memcpy(m_inputDims, dataDims, sizeof(int) * 4);
	m_inputData = featCache->buf[flag];
	setOutputDataMem();
	m_outputData = featCache->buf[!flag];
	return true;
}
void ConvLayer::getOutPutDims(int* dims){
	memcpy(dims, m_outputDims, sizeof(int)*4);
}
static inline int floor_divide(int a, int b) {
	if (a >= 0) return a / b;
	else return (a - b + 1) / b;
}

static inline int ceil_divide(int a, int b) {
	if (a >= 0) return (a + b - 1) / b;
	else return a / b;
}

bool ConvLayer::checkParamBeforeApply()
{
	if (!m_inputData || !m_filters||!m_outputData)
		return false;
	return true;
}


// implemented from base class
bool ConvLayer::execute()
{
	if (!applyConvolution())
		return false;

	return true;
}


void ConvLayer::im2row(float* stacked, float* data, int numPatchesX, int numPatchesY, int numRows)
{

	/*
	Fill a row of the stacked image at a time. Since patches are stored
	aint the columns, scanning a row menas visiting all patche once.
	Each row corresponds to a particular offset within each patch.

	In this manner, as we fill a row
	we tend to access spatially adiacent elements
	in the input image, particulary for small strides.
	*/
	float* tempMemory = stacked;
	for (int row = 0; row < numRows; ++row) {
		/*
		Get the patch offset corresponding to this row of the stacked
		image.
		*/
		int u = row;
		int v = u / m_filterDims[1];
		int z = v / m_filterDims[0];
		u %= m_filterDims[1];
		v %= m_filterDims[0];

		/*
		Filling this row amounts to visiting all the pixels in the input
		image that appear at a given offset in the outut patches. Accounting
		for the subsampling of the output patches and input padding,
		these pixels are given by

		x_data(x) = x * m_stride[1] + u - m_pad[2],  0 <= x < numPatchesX
		y_data(y) = y * m_stride[0] + v - m_pad[0],   0 <= y < numPatchesY
		z_data(z) = z.

		Here (x,y) are the spatial indexes of the output patches. Depending
		on the padding, some of these values will read pixels outised
		the input image, which should default to 0. In particular, x lands
		on a x_data(x) within the image if x0 <= x < x1 where:

		x_data(x) >= 0 <=> x >= (m_pad[2] - u) / stride
		<=> x >= ceil((m_pad[2] - u) / stride) = x0
		x_data(x) <= m_inputDims[1]-1 <=> x <= (m_inputDims[1]-1 + m_pad[2] - u) / stride
		<=> x <= floor((m_inputDims[1]-1 + m_pad[2] - u) / stride)
		<=> x <  floor((m_inputDims[1]-1 + m_pad[2] - u) / stride) + 1 = x1

		and the same for y. Note that, while usually x0 <= x1, there are
		special cases for which x1 < x0. This is accounted for in the loops
		below.
		*/

		int x0 = min(numPatchesX, (int)ceil_divide(m_pad[2] - u, m_stride[1]));
		int y0 = min(numPatchesY, (int)ceil_divide(m_pad[0] - v, m_stride[0]));
		int x1 = min(numPatchesX, (int)floor_divide(m_inputDims[1] - 1 + m_pad[2] - u, m_stride[1]) + 1);
		int y1 = min(numPatchesY, (int)floor_divide(m_inputDims[0] - 1 + m_pad[0] - v, m_stride[0]) + 1);
		int x;
		int y;

		for (y = 0; y < y0; ++y) {
			for (x = 0; x < numPatchesX; ++x) {
				*stacked++ = 0;
			}
		}
		for (; y < y1; ++y) {
			for (x = 0; x < x0; ++x) {
				*stacked++ = 0;
			}
			int y_data = y * m_stride[0] + v - m_pad[0];
			int x_data = x * m_stride[1] + u - m_pad[2];
			float const * b = data + (z * m_inputDims[0] + y_data) * m_inputDims[1] + x_data;
			for (; x < x1; ++x) {
				*stacked++ = *b;
				b += m_stride[1];
			}
			for (; x < numPatchesX; ++x) {
				*stacked++ = 0;
			}
		}
		for (; y < numPatchesY; ++y) {
			for (x = 0; x < numPatchesX; ++x) {
				*stacked++ = 0;
			}
		}
	}
}


bool ConvLayer::nnfullyconnected_forward()
{
	float alpha = 1.f;
	float beta = 0.f;
	int k = 1;
	ptrdiff_t filtersVolume = m_filterDims[0] * m_filterDims[1] * m_filterDims[2];
	if (m_filters) {
		if (m_inputDims[3] == 1) {
			/* one image in the stack */
			cblas_sgemv(CblasColMajor, CblasTrans, filtersVolume, m_filterDims[3], alpha, m_filters, filtersVolume,
				m_inputData,1, beta, m_outputData, 1);
		}
	}
	else{
		/* multiple images in the stack */
		cblas_sgemm(CblasColMajor, CblasTrans, CblasNoTrans,m_filterDims[3], m_inputDims[3], filtersVolume,
			alpha, m_filters, filtersVolume, m_inputData, filtersVolume, beta, m_outputData, m_filterDims[3]);
	//cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, numOutputPixels, numFiltersPerGroup, filtersVolume, alpha,
	//	tempMemory + tempGrpOffset, numOutputPixels, m_filters + filterGrpOffset, filtersVolume, beta,
	//	m_outputData + outputOffset + outputGrpOffset, numOutputPixels);
	}
	if (m_bias) {
		float beta = 1;
		float* tempOnes = new float[m_inputDims[3]];
		memcpy(tempOnes, allones, sizeof(float)*m_inputDims[3]);
		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m_biasDims, m_inputDims[3], k, 1.0,
			m_bias, m_biasDims, tempOnes, k, beta, m_outputData, m_biasDims);
		delete[]tempOnes;
	}
	return true;
}
