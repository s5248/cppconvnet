#pragma once
#include<iostream>
#include"defines.h"

class Classifier
{
public:
	Classifier();
	~Classifier();
	std::string m_name;
	std::string m_type;
	virtual int execute();
	virtual bool Classifier::execute(int *labelIdx);
	virtual bool setInput(Feature* &featCache, bool flag, int  *dataDims);

protected:
	float* m_inputData;
	int m_inputDims[4];
	float* m_outputData;
	int m_outputDims[4];
};

