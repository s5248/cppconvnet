/*
Copyright (C) 2014-15 Andrea Vedaldi and Karel Lenc.
Copyright (C) 2014-15 BiaoZhi Huang.
All rights reserved.

Some of this file is part of the VLFeat library and is made available under
the terms of the BSD license (see the COPYING file).
*/
#include "ReluLayer.h"
#include<algorithm>

ReluLayer::ReluLayer()
{

}


ReluLayer::~ReluLayer()
{

}

bool ReluLayer::initiStructureFromFile(char* &sin){
	if (!sin)
		return false;
	//load type and name
	char str[256];
	read<char>(sin, str);
	m_type = str;
	read<char>(sin, str);
	m_name = str;
	return true;
}
bool ReluLayer::setInput(float* inputData, int  dataDims[4]){
	int size1, size2, size3, size4;
	size1 = dataDims[0];
	size2 = size1*dataDims[1];
	size3 = size2*dataDims[2];
	size4 = size3*dataDims[3];
	memcpy(m_inputDims,dataDims,sizeof(int)*4);
	return true;
}
bool ReluLayer::setInput(Feature* &featCache, bool flag, int  dataDims[4])
{
	if (!featCache || !featCache->buf[0] || !featCache->buf[1])
		return false;
	memcpy(m_inputDims, dataDims, sizeof(int) * 4);
	memcpy(m_outputDims, dataDims, sizeof(int) * 4);
	int size = m_outputDims[0] * m_outputDims[1] * m_outputDims[2] * m_outputDims[3];
	m_inputData = featCache->buf[flag];
	m_outputData = featCache->buf[!flag];
	return true;
}

void ReluLayer::getOutPutDims(int* dims){
	memcpy(dims, m_outputDims, sizeof(int) * 4);
}

bool ReluLayer::getOutput(float* &data, int(&dataDims)[4]){
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
	Y = VL_NNRELU(X) applies the rectified linear unit to the data X.
	X can have arbitrary size.
	`Leak`:: 0(default)
	Set the leak factor, a non-negative number. Y is equal to X if
	X is not smaller than zero; otherwise, Y is equal to X
	multipied by the leak factor. By default, the leak factor is
	zero; for values greater than that one obtains the leaky ReLU
	unit.
*/
bool ReluLayer::applyRelu()
{
	if (!checkParamBeforeApply())
		return false;
	int size = m_outputDims[0] * m_outputDims[1] * m_outputDims[2] * m_outputDims[3];
	for (int i = 0; i < size; ++i){
		m_outputData[i] = std::max(m_inputData[i], 0.f);
	}
	return true;
}


bool ReluLayer::checkParamBeforeApply()
{
	if (!m_inputData)
		return false;
	return true;
}


// implemented from base class
bool ReluLayer::execute()
{
	if (!applyRelu())
		return false;
	return true;
}
