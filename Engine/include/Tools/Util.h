#pragma once 
template<typename T>
struct DataDirty
{
	bool isDirty = false;
	T data = T();
};
