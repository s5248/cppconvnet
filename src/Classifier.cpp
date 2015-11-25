/*
Copyright (C) 2015 BiaoZhi Huang.

All rights reserved.

Some of this  file is part of the VLFeat library and is made available under
the terms of the BSD license (see the COPYING file).
*/
#include "Classifier.h"


Classifier::Classifier()
{
	m_name = "softmaxClassifier";
	m_type = "softmax";
}


Classifier::~Classifier()
{
}

/*
assume input is 1*1*c*n;
c is the number of classes.
n is the number of frames in one video file.
*/
int Classifier::execute(){
	float *max = new float[m_outputDims[3]];
	memset(max, 0, sizeof(float)*m_outputDims[3]);
	for (int n = 0; n < m_inputDims[3]; ++n){
		for (int c = 0; c < m_inputDims[2]; ++c){
			if (max[n] < m_inputData[n*m_inputDims[2] + c])
			{
				max[n] = m_inputData[n*m_inputDims[2] +c];
			}
		}
	}
	
	float *sum = new float[m_outputDims[3]];
	memset(sum, 0, sizeof(float)*m_outputDims[3]);
	for (int n = 0; n < m_inputDims[3]; ++n){
		for (int c = 0; c < m_outputDims[2]; ++c){
			m_outputData[n*m_outputDims[2] + c] = expf(m_inputData[n*m_inputDims[2] + c] - max[n]);
			sum[n] += m_outputData[n*m_outputDims[2] + c];
		}
	}
	//find the classLabel with the maxProbability.
	float *prob = new float[m_outputDims[2]];
	memset(prob, 0, sizeof(float)*m_outputDims[2]);
	for (int c = 0; c < m_outputDims[2]; ++c){
		for (int n = 0; n < m_outputDims[3]; ++n){
			prob[c] += (m_outputData[n*m_outputDims[2] + c]/sum[n]);
		}
	}
	float maxProb = prob[0];
	int maxClassLabel = 1;
	for (int c = 1; c < m_outputDims[2]; ++c){
		if (maxProb < prob[c]){
			maxClassLabel = c + 1;
			maxProb = prob[c];
		}
	}
	delete[]prob;
	prob = NULL;
	delete[]sum;
	sum = NULL;
	delete[]max;
	max = NULL;
	return maxClassLabel;
}

/*return the class label index for each instance.
the matrix of m_inputData is C*N,element m_inputData(i,j)
stands for(but not equal) the probability of instance j 
belongs to class i.
*/
bool Classifier::execute(int *labelIdx){
	int nsize = sizeof(labelIdx) / sizeof(labelIdx[0]);
	if (nsize < m_outputDims[3]){
		std::cout << "class label index is not enough!\n";
		return false;
	}
	//find the classLabel with the maxProbability.
	float *max = new float[m_outputDims[3]];
	memset(max, 0, sizeof(float)*m_outputDims[3]);
	for (int n = 0; n < m_inputDims[3]; ++n){
		for (int c = 0; c < m_inputDims[2]; ++c){
			if (max[n] < m_inputData[n*m_inputDims[2] + c])
			{
				max[n] = m_inputData[n*m_inputDims[2] + c];
				labelIdx[n] = c+1;
			}
		}
	}
	delete[]max;
	max = NULL;
	return true;
}

bool Classifier::setInput(Feature* &featCache, bool flag, int  dataDims[4])
{
	if (!featCache || !featCache->buf[flag])
		return false;
	memcpy(m_inputDims, dataDims, sizeof(int) * 4);
	m_inputData = featCache->buf[flag];
	m_outputDims[0] = m_outputDims[1] = 1;
	m_outputDims[2] = m_inputDims[2];
	m_outputDims[3] = m_inputDims[3];
	m_outputData = featCache->buf[!flag];
	return true;
}