/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#pragma once
#include<iostream>

typedef struct feat{
	float* buf[2];
	bool flag;
	feat(){
		buf[0] = NULL;
		buf[1] = NULL;
		flag = false;//default use the zero buffer to cache the input data.
	}
	~feat(){
		buf[0] = NULL;
		buf[1] = NULL;
		flag = false;
	}
}Feature;
