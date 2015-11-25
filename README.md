#cppconvnet
#CNNs in C++

cppconvnet is a light convolution neural netwrok system depending on the least but performing greate. It's written by C++ and it has high performance in image field,and the original system is the [matconvnet](https://github.com/vlfeat/matconvnet/). cppconvnet is designed for anyone who uses C++ language to study deep learning or something like it. So far,It has supported the basic features ,such as, convolution,relu activation function,max pooling,noramlization,and softmax classification function.  

Homepage: [cppconvnet](https://github.com/s5248/cppconvnet)  
Author: [@s5248]('')  
Contact me: huangzb007@gmail.com  

### Versions:
- 24/11/2015 - cppconvnet 1.0.0 Beta Release. *Important milestone for cppconvnet.*


### Dependence:
> principle:least dependencies but greate performace.  

[opencv2.4.9 or above](http://opencv.org/downloads.html): Provides a tool for loading and converting images to memory data.  
[OpenBlas](https://github.com/xianyi/OpenBLAS): Provides core functions like cblas_gemm used to speed up convolution.  

#### Thanks to:  

[matconvnet](https://github.com/vlfeat/matconvnet/): A MATLAB toolbox implementing Convolutional Neural Networks (CNNs) for computer vision applications.  

### Supplying:
convolution operation using OpenBLAS.  
Relu activation function.  
pooling operation.  
noramlization operation.  
softmax classification.  
feature extraction and feature matching.  
classification of many images once a time and their confidence level.
support for vgg-f model and so like and little modifications to support other model.

### In Planning:
add training process.  
add support for GPU
add various classification method such as SVM,ANN_MLP.
add documentation for details.  

### Documentation:
A guide book of cppconvnet:  
[Guide Book of matconvnet](http://www.vlfeat.org/matconvnet/matconvnet-manual.pdf)  

### Build:
waiting for Makefile.

### Feedback:
If you have any question, please submit comment here or mention me on [csdn](http://blog.csdn.net/CHIERYU/article/details/50020165).  
Technical issues are also welcomed to be submitted on [GitHub](https://github.com/s5248/cppconvnet/issues).

