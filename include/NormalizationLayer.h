/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#pragma once
#include "Layers.h"

class NormalizationLayer :
	public Layers
{
public:
	NormalizationLayer();
	~NormalizationLayer();
	// use file pointer  to load datafor initializing layer' structure
	virtual bool initiStructureFromFile(char* &sin);
protected:
	//normDepth,normKappa,normAlpha,normBeta,expectively.
	double m_param[4];
	bool applyNormalization();
	bool checkParamBeforeApply();
	virtual bool setInput(float* inputData, int  dataDims[4]);
	virtual bool getOutput(float* &data, int  (&dataDims)[4]);
public:
	// implemented from base class
	bool execute();
	bool setInput(Feature* &featCache, bool flag, int  dataDims[4]);
	void getOutPutDims(int* dims);
};

