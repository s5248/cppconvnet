/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#pragma once
#include<iostream>
#include<functional>

using namespace std;
#define MY_read(dst,src,type,num) { memcpy(dst, src, num*sizeof(type));\
	src+=num*sizeof(type);}
template<class T>
void read(char* &sin, T*buffer){
	int num;
	int flag = 0;
	MY_read(&num, sin, int, 1);
	flag = (sizeof(T) == sizeof(char));//used for '\0' in string.
	if (buffer == NULL){
		buffer = new T[(num + flag)*sizeof(T)];
	}
	buffer[num - 1 + flag] = 0;
	MY_read(buffer, sin, T, num);
}