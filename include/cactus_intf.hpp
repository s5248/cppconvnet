/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#ifndef	CactusIntf_HPP_
#define CactusIntf_HPP_

#include <string>
#include<opencv2/opencv.hpp>
#include"CppConvNet.h"
#include<fstream>
#include"defines.h"
using namespace std;

class CactusIntf {

public:
	explicit CactusIntf(){};
	virtual ~CactusIntf();
	int init(const string& param);

	/**
	* extract features
	* @return feat_buf size
	*/
	int extract(const char* img_buf, long img_size, int* feat_buf, long max_size);

	/**
	* less value, higher rank
	* @return match value,range is [0,1e6]not [0,1], considering precision.
	*/
	float match(const int* feat_buf1, const int* feat_buf2, long buf_size);

	int release();
	/*
	* classify data into N classes. 
	* return the label index,from 1 to N.
	*/
	bool classify(cv::Mat *mat, int num,int *labelIdx);
protected:
	float *feat1;
	float *feat2;
private:
	CppConvNet net;
	void normalization(float* dstFeat, const int *srcFeat, int size);
	int featSize;
public:

	int extract(cv::Mat &srcImg, long img_size, int*  feat_buf);
};


#endif
