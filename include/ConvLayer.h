/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#pragma once
#include"Layers.h"
#include<iostream>
#include<cblas.h>
using namespace std;

class ConvLayer:public Layers
{

public:
	ConvLayer();
	~ConvLayer();
private:
	float *allones;
	void im2row(float* tempMemory, float* data, int numPatchesX, int numPatchesY, int numRows);
protected:
	float* m_filters;
	float* m_bias;
	//filter's height,width,depth,size,expectively.eg:11*11*3*64
	int m_filterDims[4];
	//bias's size,eg:1*64
	int m_biasDims;
	//strideY,strideX ,expectively.
	int m_stride[2];
	//pad for top,bottom,left,right,expectively.
	int m_pad[4];
	float *m_inputData;
	//inputData's height,width,depth,size,expectively.eg:224*224*3*1
	int m_inputDims[4];
	//outputData's height,width,depth,size,expectively.eg:54*54*64*1
	float* m_outputData;
	int m_outputDims[4];
	// used before access outputData operation
	bool setOutputDataMem();
	virtual bool setInput(float* inputData, int  dataDims[4]);
	virtual bool getOutput(float*& data, int  (&dataDims)[4]);
	bool checkParamBeforeApply();
public:
	// need data stored row-major.
	bool applyConvolution(float dataMult, float outputMult);
	/*used when don't have filters and use backmod.
	  need data stored row-major.
	*/
	// implemented from base class
	bool execute();
	// use file pointer  to load datafor initializing layer' structure
	virtual bool initiStructureFromFile(char* &sin);
	bool setInput(Feature* &featCache, bool flag, int  dataDims[4]);
	void getOutPutDims(int* dims);
protected:
	bool nnfullyconnected_forward();
};

