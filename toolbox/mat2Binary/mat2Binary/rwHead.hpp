/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#include<fstream>
#include<iostream>
#include<mex.h>
#include<algorithm>
#include<mat.h>
using namespace std;
#define _read(dst,src,type,num) {memcpy(dst, src, num*sizeof(type));\
	src+=num*sizeof(type);}

typedef struct{
	std::string type;
	float* weights1; //4D 	filters
	int weightDims[4]; //filters dimensions:height,width,depth,Size
	float *weights2; //1*64single , biases
	int weightlen;
	int pad[4];
	std::string name;
	int stride[2];
	double param[4];
	int pool[2];
	std::string method;
}CNNLayer;

typedef struct normalization{
	double *averageImage; //image_bufSize
	bool keepAspect;
	int border[2];
	int imageSize[3];
	std::string interpolation;
}CNNNormalization;

typedef struct net{
	CNNLayer layers[108];
	CNNNormalization normalization;
	net(){
		for (int i = 0; i < 21; ++i){
			this->layers[i].weights1 = NULL;
			this->layers[i].weights2 = NULL;
		}
		if (this->normalization.averageImage != NULL){
			this->normalization.averageImage = NULL;
		}
	}
	~net(){
		for (int i = 0; i < 21; ++i){
			if (this->layers[i].weights1 != NULL){
				delete[]this->layers[i].weights1;
			}
			if (this->layers[i].weights2 != NULL){
				delete[]this->layers[i].weights2;
			}
		}
		if (this->normalization.averageImage != NULL){
			delete[]this->normalization.averageImage;
		}
	}
}NET;

void readLayers(MATFile*pmatFile, NET &net, ofstream& fout);
void readAndWriteAverageImg(MATFile*pmatFile, NET &net, ofstream& fout);
void RaWMyMeanImage(std::string filename, ofstream &fout);
//template<class T>
//inline void read(ifstream&fin,T*buffer){
//	int num;
//	int flag=0;
//	fin.read((char*)&num, sizeof(int));
//	flag = (sizeof(T) == sizeof(char));//used for '\0' in string.
//	if (buffer == NULL){
//		buffer = new T[(num+flag)*sizeof(T)];
//	}
//	buffer[num - 1 + flag] = 0;
//	fin.read((char*)buffer, num*sizeof(T));
//}
template<class T>
void write(ofstream& fout, const T*buffer, int num){
	fout.write((char*)&num, sizeof(int));
	fout.write((char*)buffer, num*sizeof(T));
}
template<class T>
inline void read(char* &sin, T*buffer){
	int num;
	int flag = 0;
	_read(&num, sin, int, 1);
	flag = (sizeof(T) == sizeof(char));//used for '\0' in string.
	if (buffer == NULL){
		buffer = new T[(num + flag)*sizeof(T)];
	}
	buffer[num - 1 + flag] = 0;
	_read(buffer, sin, T, num);
}

void readAndWriteMat(std::string file, std::string output){
	MATFile *pmatFile = NULL;
	mxArray *pMxArray = NULL;
	ofstream fout;
	fout.open(output.c_str(), ios::binary);
	pmatFile = matOpen(file.c_str(), "r");
	NET net;
	readAndWriteAverageImg(pmatFile, net, fout);
	//RaWMyMeanImage(string("D:/Features/program/meanImg.txt"), fout);
	readLayers(pmatFile, net, fout);
	fout.close();
	matClose(pmatFile);
}
void RaWMyMeanImage(std::string filename, ofstream &fout){
	float meanImg[224 * 224 * 3];
	ifstream fin(filename);
	int n = 224 * 224 * 3;
	for (int i = 0; i < n;++i){
		fin >> meanImg[i];
	}
	fin.close();
	write<int>(fout, &n, 1);
	write<float>(fout, meanImg, n);
}
void readAndWriteAverageImg(MATFile*pmatFile, NET &net, ofstream& fout){
		mxArray *normalization = matGetVariable(pmatFile, "normalization");
		if (normalization){
			long ncell = mxGetNumberOfFields(normalization);
			long j = 0;
			std::string feildname(mxGetFieldNameByNumber(normalization, j));
			long t = 0;
			mxArray* feild = mxGetFieldByNumber(normalization, t, j);
			if (strcmp(feildname.c_str(), "averageImage") == 0){
				double *data = (double*)mxGetData(feild); //for vgg-f.
				//float *data = (float*)mxGetData(feild);
				int n = (int)mxGetNumberOfElements(feild);
				size_t* dims = (size_t*)mxGetDimensions(feild);
				for (int i = 0; i < 3; ++i)
					std::cout << dims[i] << " ";
				std::cout << endl;
				std::cout << "elements is :" << n << " pixels.\n";
				float *meanImg = new float[n];
				for (int i = 0; i < n; ++i){
					meanImg[i] = (float)data[i];
				}
				write<int>(fout, &n, 1);
				write<float>(fout, meanImg, n);
				for (int i = 0; i < 50; ++i){
					std::cout << meanImg[i] << " ";
				}
				std::cout << endl;
				delete[]meanImg;
			}
		}
}
void MeanImgRd(char*&sin){
	int size;
	read<int>(sin, &size);
	std::cout << endl << "AverageImage:" << endl;
	float *meanImg = new float[size];
	read<float>(sin, meanImg);
	for (int i = 0; i < 50; ++i){
		std::cout << meanImg[i] << " ";
	}
	std::cout << endl;
}
void readLayers(MATFile*pmatFile, NET &net, ofstream& fout){
	mxArray *layers = matGetVariable(pmatFile, "layers");
	int layerSize = (int)mxGetNumberOfElements(layers);
	cout <<"layerSize="<< layerSize << endl; 
	if (layers){
		for (int i = 0; i < layerSize; ++i){
			std::cout << "\n\n---------------- " << i << " --------------------\n\n";
			mxArray* cell = mxGetCell(layers, i);
			int ncell = mxGetNumberOfFields(cell);
			for (int j = 0; j < ncell; ++j){
				std::string feildname(mxGetFieldNameByNumber(cell, j));
				int t = 0;
				mxArray* feild = mxGetFieldByNumber(cell, t, j);
				if (strcmp(feildname.c_str(), "weights") == 0){
					mxArray *weights = mxGetCell(feild, 0);
					size_t *dims = (size_t *)mxGetDimensions(weights);
					int ndims = (int)mxGetNumberOfDimensions(weights);//should be 4
					std::cout << "dims:";
					for (int k = 0; k < ndims; ++k)
					{
						net.layers[i].weightDims[k] = (int)dims[k];
						std::cout << dims[k] << " ";
					}
					std::cout << endl;
					int num = 4;
					write<int>(fout, net.layers[i].weightDims, num);
					num = (int)mxGetNumberOfElements(weights);
					net.layers[i].weights1 = new float[num];
					float* data = (float*)mxGetData(weights);
					for (int ni = 0; ni < num; ++ni)
					{
						net.layers[i].weights1[ni] = data[ni];
						if (ni < 10)
							std::cout << data[ni] << " ";
					}
					write<float>(fout, data, num);
					weights = mxGetCell(feild, 1);
					ndims = (int)mxGetNumberOfDimensions(weights);
					net.layers[i].weightlen = (int)dims[1];
					num = (int)mxGetNumberOfElements(weights);
					write<int>(fout, &num, 1);
					std::cout << num << endl;
					net.layers[i].weights2 = new float[num];
					data = (float*)mxGetData(weights);
					for (int ni = 0; ni < num; ++ni){
						net.layers[i].weights2[i] = data[i];
						if (ni<10)
							std::cout << data[ni] << " ";
					}
					write<float>(fout, data, num);
				}
				if (strcmp(feildname.c_str(), "pad") == 0){
					int num = (int)mxGetNumberOfElements(feild);
					double *data = (double*)mxGetData(feild);
					std::cout << "\npad:";
					for (int k = 0; k < num; ++k)
					{
						net.layers[i].pad[k] = (int)data[k];
						std::cout << net.layers[i].pad[k] << "   ";
					}
					write<int>(fout, net.layers[i].pad, num);
				}
				if (strcmp(feildname.c_str(), "type") == 0){
					char str[256];
					mxGetString(feild, str, sizeof(str));
					net.layers[i].type = str;
					std::cout << "type:" << str << " ";
					write<char>(fout, str, (int)strlen(str));
				}
				if (strcmp(feildname.c_str(), "name") == 0){
					char str[256];
					mxGetString(feild, str, sizeof(str));
					net.layers[i].name = str;
					std::cout << "name:" << str << endl;
					write<char>(fout, str, (int)strlen(str));
				}
				if (strcmp(feildname.c_str(), "stride") == 0){
					int num = (int)mxGetNumberOfElements(feild);
					double *data = (double*)mxGetData(feild);
					std::cout << "\nstride:";
					for (int k = 0; k < num; ++k)
					{
						net.layers[i].stride[k] = (int)data[k];
						std::cout << net.layers[i].stride[k] << " ";
					}
					std::cout << endl;
					write<int>(fout, net.layers[i].stride, num);
				}
				if (strcmp(feildname.c_str(), "param") == 0){
					int num = (int)mxGetNumberOfElements(feild);
					double *data = (double*)mxGetData(feild);
					std::cout << "param:";
					for (int k = 0; k < num; ++k)
					{
						net.layers[i].param[k] = data[k];
						std::cout << data[k] << " ";
					}
					std::cout << endl;
					write<double>(fout, data, num);
				}
				if (strcmp(feildname.c_str(), "method") == 0){
					char str[256];
					mxGetString(feild, str, sizeof(str));
					net.layers[i].method = str;
					write<char>(fout, str, (int)strlen(str));
				}
				if (strcmp(feildname.c_str(), "pool") == 0){
					int num = (int)mxGetNumberOfElements(feild);
					double *data = (double*)mxGetData(feild);
					std::cout << "pool:";
					for (int k = 0; k < num; ++k)
					{
						net.layers[i].pool[k] = (int)data[k];
						std::cout << net.layers[i].pool[k] << "  ";
					}
					std::cout << endl;
					write<int>(fout, net.layers[i].pool, num);
				}
			}
		}
	}
}
void convRd(char*&sin, int idx){
	std::cout << "\n\n----------------- " << idx << " -------------\n\n";
	int size;
	NET net;
	read<int>(sin, net.layers[0].weightDims);
	size = 1;
	std::cout << "filter dims:";
	for (int i = 0; i < 4; ++i){
		std::cout << net.layers[0].weightDims[i] << " ";
		size *= net.layers[0].weightDims[i];
	}
	std::cout << endl << "filters:" << endl;
	net.layers[0].weights1 = new float[size];
	read<float>(sin, net.layers[0].weights1);
	if (net.layers[0].weights1){
		for (int i = 0; i < std::min(size,50); ++i){
			std::cout << net.layers[0].weights1[i] << " ";
		}
	}
	std::cout << endl;
	std::cout << "bias dim:";
	read<int>(sin, &net.layers[0].weightlen);
	size = net.layers[0].weightlen;
	std::cout << net.layers[0].weightlen << endl;
	net.layers[0].weights2 = new float[size];
	read<float>(sin, net.layers[0].weights2);
	std::cout << "bias:";
	for (int i = 0; i < std::min(size, 50); ++i){
		std::cout << net.layers[0].weights2[i] << " ";
	}
	std::cout << endl;
	read<int>(sin, net.layers[0].pad);
	for (int i = 0; i < 4; ++i){
		std::cout << net.layers[0].pad[i] << " ";
	}
	read<char>(sin, (char*)net.layers[0].type.c_str());
	std::cout << "type:" << net.layers[0].type.c_str() << "  ";
	read<char>(sin, (char*)net.layers[0].name.c_str());
	std::cout << "name:" << net.layers[0].name.c_str() << "   stride:";
	read<int>(sin, (int*)net.layers[0].stride);
	for (int i = 0; i < 2; ++i){
		std::cout << net.layers[0].stride[i] << " ";
	}
}
void reluRd(char*&sin, int idx){
	std::cout << "\n\n----------------- " << idx << " -------------\n\n";
	NET net;
	read<char>(sin, (char*)net.layers[1].type.c_str());
	std::cout << endl << net.layers[1].type.c_str() << "  ";
	read<char>(sin, (char*)net.layers[1].name.c_str());
	std::cout << net.layers[1].type.c_str() << endl;
}
void normRd(char*&sin, int idx){
	std::cout << "\n\n----------------- " << idx << " -------------\n\n";
	NET net;
	read<char>(sin, (char*)net.layers[2].type.c_str());
	std::cout << endl << net.layers[2].type.c_str() << "  ";
	read<char>(sin, (char*)net.layers[2].name.c_str());
	std::cout << endl << net.layers[2].name.c_str() << "  ";
	read<double>(sin, net.layers[2].param);
	for (int i = 0; i < 4; ++i){
		std::cout << net.layers[2].param[i] << " ";
	}
	std::cout << endl;
}
void poolRd(char*&sin, int idx){
	std::cout << "\n\n----------------- " << idx << " -------------\n\n";
	NET net;
	read<char>(sin, (char*)net.layers[3].name.c_str());
	std::cout << net.layers[3].name.c_str() << "  ";
	read<int>(sin, (int*)net.layers[3].stride);
	for (int i = 0; i < 2; ++i){
		std::cout << net.layers[3].stride[i] << " ";
	}
	read<int>(sin, net.layers[3].pad);
	for (int i = 0; i < 4; ++i){
		std::cout << net.layers[3].pad[i] << " ";
	}
	read<char>(sin, (char*)net.layers[3].type.c_str());
	std::cout << net.layers[3].type.c_str() << "  ";
	read<char>(sin, (char*)net.layers[3].method.c_str());
	std::cout << net.layers[3].method.c_str() << "  ";
	read<int>(sin, net.layers[3].pool);
	for (int i = 0; i < 2; i++){
		std::cout << net.layers[3].pool[i] << " ";
	}
	std::cout << endl;
}

void readModel(std::string file){
	std::cout << sizeof(int) << " " << sizeof(int) << endl;
	ifstream fin;
	fin.open(file.c_str(), ios::binary);
	if (fin.fail()){
		std::cout << "open file failed!\n";
		return;
	}
	fin.seekg(0, ios::end);
	int size = (int)fin.tellg();
	char *src = new char[size];
	char* sin = src;
	fin.seekg(0, ios::beg);
	fin.read(src, size);
	fin.close();
	std::cout << "\n=============================================\n";
	MeanImgRd(sin);
	convRd(sin, 0);
	reluRd(sin, 1);
	normRd(sin, 2);
	poolRd(sin, 3);
	convRd(sin, 4);
	reluRd(sin, 5);
	normRd(sin, 6);
	poolRd(sin, 7);
	convRd(sin, 8);
	reluRd(sin, 9);
	convRd(sin, 10);
	reluRd(sin, 11);
	convRd(sin, 12);
	reluRd(sin, 13);
	poolRd(sin, 14);
	convRd(sin, 15);
	reluRd(sin, 16);
	convRd(sin, 17);
	reluRd(sin, 18);
	convRd(sin, 19);
	if (src){
		delete[]src;
	}
}
void readVggVd19Model(std::string file){
	std::cout << sizeof(int) << " " << sizeof(int) << endl;
	ifstream fin;
	fin.open(file.c_str(), ios::binary);
	if (fin.fail()){
		std::cout << "open file failed!\n";
		return;
	}
	fin.seekg(0, ios::end);
	int size = (int)fin.tellg();
	char *src = new char[size];
	char* sin = src;
	fin.seekg(0, ios::beg);
	fin.read(src, size);
	fin.close();
	std::cout << "\n=============================================\n";
	MeanImgRd(sin);
	convRd(sin,0);
	reluRd(sin, 1);
	convRd(sin, 2);
	reluRd(sin, 3);
	poolRd(sin, 4);
	convRd(sin, 5);
	reluRd(sin, 6);
	convRd(sin, 7);
	reluRd(sin, 8);
	poolRd(sin, 9);
	convRd(sin, 10);
	reluRd(sin, 11);
	convRd(sin, 12);
	reluRd(sin, 13);
	convRd(sin, 14);
	reluRd(sin, 15);
	convRd(sin, 16);
	reluRd(sin, 17);
	poolRd(sin, 18);
	convRd(sin, 19);
	reluRd(sin, 20);
	convRd(sin, 21);
	reluRd(sin, 22);
	convRd(sin, 23);
	reluRd(sin, 24);
	convRd(sin, 25);
	reluRd(sin, 26);
	poolRd(sin, 27);
	convRd(sin, 28);
	reluRd(sin, 29);
	convRd(sin, 30);
	reluRd(sin, 31);
	convRd(sin, 32);
	reluRd(sin, 33);
	convRd(sin, 34);
	reluRd(sin, 35);
	poolRd(sin, 36);
	convRd(sin, 37);
	reluRd(sin, 38);
	convRd(sin, 39);
	reluRd(sin, 40);
	convRd(sin, 41);
}
