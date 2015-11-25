#include"cactus_intf.hpp"

int CactusIntf::init(const string& param){
	std::string modelfile = param;
	net.initFeatureCache();
	net.creatCNNnet(modelfile);
	featSize = 4096;
	feat1 = new float[featSize];
	feat2 = new float[featSize];
	return 0;
}
int CactusIntf::extract(cv::Mat &srcImg, long img_size, int*  feat_buf)
{
	net.setSrcImage(srcImg);
	int featNum = net.driveNet(feat_buf);
	return featNum;
}
int CactusIntf::extract(const char* img_buf, long img_size, int* feat_buf, long max_size){
	std::vector<uchar> imgchar(img_buf, img_buf + img_size);
	cv::Mat img = cv::imdecode(cv::Mat(imgchar), CV_LOAD_IMAGE_COLOR);
	net.setSrcImage(img);
	int featNum = net.driveNet(feat_buf);
	return featNum;
}
void CactusIntf::normalization(float* dstFeat, const int *srcFeat, int size){
	float sum = 0.f;
	for (int i = 0; i < size; ++i){
		//accumulate
		sum += float((srcFeat[i] / 1e3)*(srcFeat[i] / 1e3));
	}	
	if (fabs(sum) < 1e-5){
		cout << "error in normalization.\n";
	}
	else{
			for (int j = 0; j < size; ++j){
				dstFeat[j] = (srcFeat[j]) / (sqrt(sum));
				if (dstFeat[j] < 0){
					std::cout << "second float value overflow!\n";
				}
			}
		}
}

float CactusIntf::match(const int* feat_buf1, const int* feat_buf2, long buf_size){
	normalization(feat1, feat_buf1, featSize);
	normalization(feat2, feat_buf2, featSize);
	float distance = 0.f;
	for (int i = 0; i < featSize; ++i){
		distance += feat1[i] * feat2[i];
	}
	cout << "similarity = " << distance << endl;
	distance = std::max(1e6f - distance, 0.f);
	return distance;
}

int CactusIntf::release(){
	if (feat1){
		delete[] feat1;
		feat1 = NULL;
	}
	if (feat2){
		delete[] feat2;
		feat2 = NULL;
	}
	return 0;
}
CactusIntf::~CactusIntf(){

}
bool CactusIntf::classify(cv::Mat *mat, int num,int *labelIdx){
	net.setSrcImage(mat, num);
	bool success = net.driveNet2Classify(labelIdx);
	return true;
}