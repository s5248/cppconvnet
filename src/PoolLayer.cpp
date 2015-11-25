/*
Copyright (C) 2014-15 Andrea Vedaldi and Karel Lenc.
Copyright (C) 2014-15 BiaoZhi Huang.
All rights reserved.

Some of this  file is part of the VLFeat library and is made available under
the terms of the BSD license (see the COPYING file).
*/
#include "PoolLayer.h"
#include<algorithm>

template <typename type>
struct acc_max
{
	inline acc_max(int poolHeight, int poolWidth, type derOutput = 0)
		:
		value(-3.40282346638528860e+38),
		derOutput(derOutput),
		derDataActivePt(NULL)
	{ }

	inline void accumulate_forward(type x) {
		value = std::max(value, x);
	}

	inline void accumulate_backward(type const* data, type* derDataPt) {
		type x = *data;
		if (x > value) {
			value = x;
			derDataActivePt = derDataPt;
		}
	}

	inline type done_forward() const {
		return value;
	}

	inline void done_backward() const {
		if (derDataActivePt) { *derDataActivePt += derOutput; }
	}

	type value;
	type derOutput;
	type* derDataActivePt;
};
PoolLayer::PoolLayer()
{
	m_stride[0] = m_stride[1] = -1;
	m_pad[0] = m_pad[1] = m_pad[2] = m_pad[3] = -1;
	m_method.clear();
	m_pool[0] = m_pool[0] = -1;
}


PoolLayer::~PoolLayer()
{
}

bool PoolLayer::initiStructureFromFile(char* &sin){
	if (!sin)
		return false;
	char str[256];
	//load name

	read<char>(sin, str);
	m_name = str;
	//load stride
	int data[4];
	read<int>(sin, data);
	for (int i = 0; i < 2; ++i){
		m_stride[i] = (int)data[i];
	}
	//load pad
	read<int>(sin, data);
	for (int i = 0; i < 4; ++i){
		m_pad[i] = (int)data[i];
	}
	//load 
	read<char>(sin, str);
	m_type = str;
	//load method
	read<char>(sin, str);
	m_method = str;
	//load pool
	read<int>(sin, data);
	for (int i = 0; i < 2; ++i){
		m_pool[i] = (int)data[i];
	}
	return true; 
}
bool PoolLayer::setInput(float* inputData, int  dataDims[4]){
	int size1, size2, size3, size4;
	size1 = dataDims[0];
	size2 = size1*dataDims[1];
	size3 = size2*dataDims[2];
	size4 = size3*dataDims[3];
	for (int i = 0; i < 4; ++i){
		m_inputDims[i] = dataDims[i];
	}
	m_inputData = new float[size4];
	memcpy(m_inputData, inputData, sizeof(float)*size4);
	return true;
}
bool PoolLayer::setInput(Feature* &featCache, bool flag, int  dataDims[4])
{
	if (!featCache || !featCache->buf[0] || !featCache->buf[1])
		return false;
	memcpy(m_inputDims, dataDims, sizeof(int) * 4);
	m_inputData = featCache->buf[flag];
	setOutputDataMem();
	m_outputData = featCache->buf[!flag];
	return true;
}
void PoolLayer::getOutPutDims(int* dims){
	memcpy(dims, m_outputDims, sizeof(int) * 4);
}
bool PoolLayer::getOutput(float* &data, int(&dataDims)[4]){
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

bool PoolLayer::setOutputDataMem()
{
	//set outpuDataDims
	m_outputDims[0] = (m_inputDims[0] + (m_pad[0] + m_pad[1]) - m_pool[0]) / m_stride[0] + 1;
	m_outputDims[1] = (m_inputDims[1] + (m_pad[2] + m_pad[3]) - m_pool[1]) / m_stride[1] + 1;
	m_outputDims[2] = m_inputDims[2];
	m_outputDims[3] = m_inputDims[3];
	return true;
}

/*
Reverse accumulation style (better for writing).
- pick an input coordiante xi; goal is to compute dz/dxi
- look for all the pools Pj that cointain xi
-  compute dfj/dxi (Pj)
-  accumulate into dz/dxi += dz/dfj dfj/dxi (Pj)

The advantage of this method is that dz/dxi can be processed in parallel
without conflicts from other threads writing on different dz/dxi. The
disadvantage is that for eac xi we need to know dfj/dxi (Pj) for all
the pools Pj that contain xi. Barring special cases (e.g. linear) this
usually requires additional information to be available. For instance,
for max pooling knowing the output in addition to the input of the
pooling operator.

Direct accumulation style.
- pick an output coordiante fj and its pool Pj
- for all the input point xi in the pool Pj
- compute dfj/dxi (Pj)
- accumulate to dz/dxi += dz/dfj dfj/dxi (Pj)

The difference with the last method is that different output pools Pj
will share several input pixels xi; hence this will cause write conflicts if
Pj are processed in parallel.
*/
bool PoolLayer::applyPooling()
{
	if (!checkParamBeforeApply())
		return false;
	int pooledWidth = m_outputDims[1];
	int pooledHeight = m_outputDims[0];
	float *data = m_inputData;
	float *result = m_outputData;
	for (int z = 0; z < m_outputDims[2]; ++z) {
		for (int y = 0; y < pooledHeight; ++y) {
			for (int x = 0; x < pooledWidth; ++x) {
				int x1 = x * (signed)m_stride[0] - (signed)m_pad[2];
				int y1 = y * (signed)m_stride[1] - (signed)m_pad[0];
				int x2 = std::min(x1 + m_pool[1], m_inputDims[1]);
				int y2 = std::min(y1 + m_pool[0], m_inputDims[0]);
				x1 = std::max(x1, 0);
				y1 = std::max(y1, 0);
				acc_max<float> acc(y2 - y1, x2 - x1);
				for (int v = y1; v < y2; ++v) {
					for (int u = x1; u < x2; ++u) {
						acc.accumulate_forward(data[v *  m_inputDims[1] + u]);
					}
				}
				result[y * pooledWidth + x] = acc.done_forward();
			}
		}
		data += m_inputDims[1] * m_inputDims[0];
		result += pooledWidth*pooledHeight;
	}
	return true;
}


bool PoolLayer::checkParamBeforeApply()
{
	if (!m_inputData)
		return false;
	bool flag = true;
	flag = flag&&(m_stride[0] >= 0 && m_stride[1] >= 0);
	for (int i = 0; i < 4; ++i){
		flag = flag && (m_pad[i] >= 0);
	}
	flag = flag && (m_pool[0] >= 0 && m_pool[1] >= 0) && (m_method.size() > 0);
	if (!flag)
		return false;
	return true;
}


// implemented from base class
bool PoolLayer::execute()
{

	if (!applyPooling())
		return false;
	return true;
}
