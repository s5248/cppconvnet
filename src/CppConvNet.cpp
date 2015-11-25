/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#include "CppConvNet.h"
#include<fstream>
#include <sstream>
string num2str(int i)
{
	stringstream ss;
	ss << i;
	return ss.str();
}
CppConvNet::CppConvNet()
{
	/*set the default maximum number of samples CNN can process once a time.
	 can't be too big,total size of featureMap_cache is m_samples*m_MaxSize*2+224*224*3
	*/
	m_samples = 256;
	dataDims[3] = m_samples;
	dataDims[2] = 3;
	dataDims[1] = 224;
	dataDims[0] = 224;
	featNum = -1;
	averageImg = new float[224 * 224 * 3];
	memset(averageImg, 0, sizeof(float) * 224 * 224 * 3);
}
CppConvNet::CppConvNet(std::string file){
	if(creatCNNnet(file))
		m_loadedNetData = true;
}

CppConvNet::~CppConvNet()
{
	if (m_loadedNetData)
		destroyCNNnet();
}
bool CppConvNet::creatCNNnet(std::string file){
	for (int i = 1; i <= 8; ++i){
		std::string type = "conv";
		std::string name = type + num2str(i);
		m_convLayer[i - 1].setTypeName(type, name);
	}
	for (int i = 1; i <= 7; ++i){
		std::string type = "relu";
		std::string name = type + num2str(i);
		m_reluLayer[i - 1].setTypeName(type, name);
	}
	for (int i = 1; i <= 2; ++i){
		std::string type = "normalization";
		std::string name = type + num2str(i);
		m_normLayer[i - 1].setTypeName(type, name);
	}
	for (int i = 1; i <= 3; ++i){
		std::string type = "pool";
		std::string name = type + num2str(i);
		m_poolLayer[i - 1].setTypeName(type, name);
	}
	m_layerVec.push_back(&m_convLayer[0]);		//conv1
	m_layerVec.push_back(&m_reluLayer[0]);      //relu1
	m_layerVec.push_back(&m_normLayer[0]);		//norm1
	m_layerVec.push_back(&m_poolLayer[0]);		//pool1
	m_layerVec.push_back(&m_convLayer[1]);		//conv2
	m_layerVec.push_back(&m_reluLayer[1]);		//relu2
	m_layerVec.push_back(&m_normLayer[1]);		//norm2
	m_layerVec.push_back(&m_poolLayer[1]);		//pool2
	m_layerVec.push_back(&m_convLayer[2]);		//conv3
	m_layerVec.push_back(&m_reluLayer[2]);		//relu3
	m_layerVec.push_back(&m_convLayer[3]);		//conv4
	m_layerVec.push_back(&m_reluLayer[3]);		//relu4
	m_layerVec.push_back(&m_convLayer[4]);		//conv5
	m_layerVec.push_back(&m_reluLayer[4]);		//relu5
	m_layerVec.push_back(&m_poolLayer[2]);		//pool3
	m_layerVec.push_back(&m_convLayer[5]);		//conv6
	m_layerVec.push_back(&m_reluLayer[5]);		//relu6
	m_layerVec.push_back(&m_convLayer[6]);		//conv7
	m_layerVec.push_back(&m_reluLayer[6]);		//relu7
	m_layerVec.push_back(&m_convLayer[7]);		//conv8

	//load data from file.
	if (!loadCNNnetData(file)){
		return false;
	}
	m_loadedNetData = true;
	return true;
}
void CppConvNet::destroyCNNnet(){
	size_t size = m_layerVec.size();
	if (m_layerVec.size()>0)
		m_layerVec.clear();
	if (g_featCache->buf[0]){
		delete[]g_featCache->buf[0];
		g_featCache->buf[0] = NULL;
	}
	if (g_featCache->buf[1]){
		delete[]g_featCache->buf[1];
		g_featCache->buf[1] = NULL;
	}
	if (this->g_featCache){
		delete this->g_featCache;
		this->g_featCache = NULL;
	}
	if (averageImg != NULL){
		delete[]averageImg;
		averageImg = NULL;
	}
	m_loadedNetData = false;
}

bool CppConvNet::loadCNNnetData(std::string file){
	ifstream fin;
	fin.open(file.c_str(), ios::binary);
	if (fin.fail()){
		return false;
	}
	fin.seekg(0, ios::end);
	size_t size = (size_t)fin.tellg();
	char *src = new char[size];
	char* sin = src;
	fin.seekg(0, ios::beg);
	fin.read(src, size);
	fin.close();
	bool success = false;
	int vecSize = (int)m_layerVec.size();
	//load averageImage data.
	setAverageImg(sin);
	for (int i = 0; i < vecSize; ++i){
		if (sin > src + size){
			std::cout << "pointer error"<<endl;
		}
		//cout << m_layerVec[i]->m_name<<endl;
		success = m_layerVec[i]->initiStructureFromFile(sin);
		if (!success){
			break;
		}
	}
	if (src){
		delete[]src;
		src = NULL;
		sin = NULL;
	}
	return true;
}
void CppConvNet::initFeatureCache()
{
	m_MaxSize = 54 * 54 * 64 * m_samples;
	this->g_featCache = new Feature();
	this->g_featCache->buf[0] = new float[m_MaxSize];
	this->g_featCache->buf[1] = new float[m_MaxSize];
	this->g_featCache->flag = false;
}
int CppConvNet::driveNet(int* &featMap){
	if (!m_loadedNetData){//check has initialized the net structure and data.
		return -1;
	}
	int dims[4] = { dataDims[0], dataDims[1], dataDims[2], dataDims[3] };
	size_t size = m_layerVec.size(),i;
	for (i = 0; i < 19; ++i){
		m_layerVec[i]->setInput(g_featCache, g_featCache->flag, dims);
		g_featCache->flag = !g_featCache->flag;
		if (!m_layerVec[i]->execute())
		{
			return -1;
		}
		m_layerVec[i]->getOutPutDims(dims);

	}
	int featNum = dims[2];
	for (int i = 0; i < featNum; ++i){
		featMap[i] = int(g_featCache->buf[g_featCache->flag][i] * 1e7);
	}

	if (this->featNum>0)
		this->featNum = featNum;
	return featNum;
}

//output the class label.
bool CppConvNet::driveNet2Classify(int *labelIdx){
	if (!m_loadedNetData){//check has initialized the net structure and data.
		return false;
	}
	int dims[4] = { dataDims[0], dataDims[1], dataDims[2], dataDims[3] };
	size_t size = m_layerVec.size(), i;
	for (i = 0; i < size; ++i){
		m_layerVec[i]->setInput(g_featCache, g_featCache->flag, dims);
		g_featCache->flag = !g_featCache->flag;
		if (!m_layerVec[i]->execute())
		{
			return false;
		}
		m_layerVec[i]->getOutPutDims(dims);
	}
	//give data to classifier,and get the class label(from 1 to N).
	m_classifier.setInput(g_featCache, g_featCache->flag, dims);
	g_featCache->flag = !g_featCache->flag;
	return m_classifier.execute(labelIdx);
}

bool CppConvNet::setAverageImg(char* &sin)
{
	int size;
	read<int>(sin, &size);
	read<float>(sin, averageImg);
	return true;
}
//set the input is 224*224*3*1
bool CppConvNet::setSrcImage(cv::Mat &mat)
{
	cv::Mat img;
	cv::cvtColor(mat, mat, CV_BGR2RGB);
	dataDims[3] = 1;
	//preProcessImage(mat, img);
	//crop image.
	cv::Rect rtROI(int(mat.cols*0.15), int(mat.rows*0.15), int(mat.cols*0.7),int(mat.rows*0.7));
	img = mat(rtROI);
	cv::resize(img, img, cv::Size(224, 224), 0.0, 0.0, CV_INTER_CUBIC);//don't do any proprocess.
	int channels = img.channels();
	int step = img.step;
	int sub = 0;
	g_featCache->flag = false;
	for (int k = 0; k < channels; ++k){
		for (int j = 0; j < img.cols; ++j){
			for (int i = 0; i < img.rows; ++i){
				this->g_featCache->buf[0][sub++] = float(*(img.data + j*step + i*channels + k)) - averageImg[i];
			}

		}
	}
	return true;
}

//set the input is 224*224*3*num.
bool CppConvNet::setSrcImage(cv::Mat *mat,int num)
{
	int channels = mat[0].channels();
	int step = mat[0].step;
	int sub = 0;
	dataDims[3] = num;
	g_featCache->flag = false;
	for (int n = 0; n < num; ++n){
		cv::cvtColor(mat[n], mat[n], CV_BGR2RGB);
		for (int k = 0; k < channels; ++k){
			for (int i = 0; i < mat[n].rows; ++i){
				for (int j = 0; j < mat[n].cols; ++j){
					this->g_featCache->buf[0][sub] = float(*(mat[n].data + j*step + i*channels + k)) - averageImg[sub % (224 * 224 * 3)];
					sub++;
				}
			}
		}
	}
	return true;
}
