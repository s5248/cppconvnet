/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#pragma once
#include"Layers.h"
#include"ConvLayer.h"
#include"ReluLayer.h"
#include"NormalizationLayer.h"
#include"PoolLayer.h"
#include"Classifier.h"
#include<vector>
#include<string>
#include<opencv2/opencv.hpp>

class CppConvNet
{
public:
	CppConvNet();
	CppConvNet(std::string file);
	~CppConvNet();
private:
	bool m_loadedNetData;
	//Net variables
	std::vector<Layers*> m_layerVec;
	ConvLayer m_convLayer[8];
	ReluLayer m_reluLayer[7];
	NormalizationLayer m_normLayer[2];
	PoolLayer m_poolLayer[3];
	int m_samples;
	int m_MaxSize;
	Feature* g_featCache;
	bool loadCNNnetData(std::string file);
	float *averageImg;
	int dataDims[4];
	Classifier m_classifier;
public:
	bool creatCNNnet(std::string file);
	void destroyCNNnet();
	//return the feature number,if failed,return -1.
	int driveNet(int* &featMap);
	bool driveNet2Classify(int *labelIdx);//used for classifier
	void initFeatureCache();
	bool setAverageImg(char* &sin);
	//input is only one image.
	bool setSrcImage(cv::Mat &mat);
	//input is image stacked,total number is num.
	bool setSrcImage(cv::Mat *mat,int num);
	int featNum;
};

