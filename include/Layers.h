/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#pragma once
#include<iostream>
#include"readModel.hpp"
#include<cassert>
#include"defines.h"

class Layers
{
public:
	Layers();
	Layers(std::string type,std::string name);
	virtual ~Layers();
protected:
	float* m_inputData;
	int m_inputDims[4];
	float* m_outputData;
	int m_outputDims[4];
public:
	std::string m_name;
	std::string m_type;
	//dims for:height,width,depth,size(number),expectively.
	virtual bool setInput(float* inputData, int  dataDims[4])=0;
	virtual bool getOutput(float*& data, int  (&dataDims)[4])=0;
	bool setTypeName(std::string type, std::string name);
	// use file pointer  to load datafor initializing layer' structure
	virtual bool initiStructureFromFile(char* &sin) = 0;
	virtual bool setInput(Feature* &featCache, bool flag, int  *dataDims) = 0;
protected:
	virtual bool checkParamBeforeApply() = 0;
public:
	// each layer need to implemented this operation,this is the same interface for others.
	virtual bool execute() = 0;
	void releaseData();
	virtual void getOutPutDims(int* dims) = 0;
};
