/*
Copyright (C) 2015 BiaoZhi Huang.
All rights reserved.
*/
#include "Layers.h"

Layers::Layers()
{
	m_inputData = NULL;
	m_outputData = NULL;
	m_name.clear();
	m_type.clear();
}
Layers::Layers(std::string type, std::string name)
{
	m_inputData = NULL;
	m_outputData = NULL;
	m_name = name;
	m_type = type;
}

Layers::~Layers()
{
	if (m_inputData) {
		m_inputData = NULL;
	}
	if (m_outputData) {
		m_outputData = NULL;
	}
}


bool Layers::setTypeName(std::string type, std::string name)
{
	if (!type.size() && !name.size())
		return false;
	m_type = type;
	m_name = name;
	return true;
}
void Layers::releaseData(){
	if (m_inputData != NULL)
	{
		delete[]m_inputData;
		m_inputData = NULL;
	}
}
