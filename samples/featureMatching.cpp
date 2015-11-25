/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#include"cactus_intf.hpp"
#include<iostream>
#include<opencv2/opencv.hpp>

int main(int argc, char** argv){
	CactusIntf cactus;
	std::string path = "D:/cppconvnet/data/modelvgg-f";
	cactus.init(path);
	std::string filename = "D:/cppconvnet/data/dog.jpg";
	cv::Mat img = cv::imread(filename, 1);
	if (img.empty()){
		return -1;
	}
	int *feat_buf = new int[4096];
	cactus.extract(img, 0, feat_buf);
	//please replace the second parameter with another image's feature.
	cout << "feature distance is :" << cactus.match(feat_buf, feat_buf, 4096)<<endl;
	delete[]feat_buf;
	return 0;
}
