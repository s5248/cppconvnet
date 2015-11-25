#if 1
#include<fstream>
#include<iostream>
#include"rwHead.hpp"
using namespace std;
std::string mat = "D:/cppconvnet/data/imagenet-vgg-f.mat";
std::string out = "D:/cppconvnet/data/modelvgg-f";
int main(int argc,char**argv){
	if (argc >= 3){
		mat = argv[1];
		out = argv[2];
	}
	readAndWriteMat(mat, out);
	readModel(out);
	system("pause");
	return 0;
}
#endif