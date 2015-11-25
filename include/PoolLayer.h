/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#pragma once
#include "Layers.h"

class PoolLayer :
	public Layers
{
public:
	PoolLayer();
	~PoolLayer();
public:
	// use file pointer  to load datafor initializing layer' structure
	virtual bool initiStructureFromFile(char* &sin);
protected:
	int m_stride[2];
	int m_pad[4];
	std::string m_method;
	//pool height,width,expectively.
	int m_pool[2];
protected:
	// used before access outputData operation
	bool setOutputDataMem();
	bool applyPooling();
	bool checkParamBeforeApply();
	virtual bool setInput(float* inputData, int  dataDims[4]);
	virtual bool getOutput(float*& data, int(&dataDims)[4]);
public:
	// implemented from base class
	bool execute();
	bool setInput(Feature* &featCache, bool flag, int  dataDims[4]);
	void getOutPutDims(int* dims);
};

