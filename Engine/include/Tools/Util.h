#pragma once 
#include <stdexcept>
#include <vector>

template<typename T>
struct DataDirty
{
	bool isDirty = false;
	T data = T();
};




template<typename T, size_t targetSize>
struct PadData;



template<typename T, size_t targetSize> requires(targetSize > sizeof(T))
struct PadData<T, targetSize>
{

	PadData() : val(), tabPad() {}
	PadData(const T& copy) : val(copy), tabPad() {}
	PadData(T&& move) noexcept : val(std::move(move)), tabPad() {}
	PadData& operator=(const T& data)
	{
		val = data;
		return *this;
	}
	PadData& operator=(T&& data) noexcept
	{
		val = std::move(data);
		return *this;
	}

	
	// T = PadData;
	operator T& ()
	{
		return val;
	}
	operator const T& () const
	{
		return val;
	}
	T* operator->()
	{
		return &val;
	}
	const T* operator->() const
	{
		return &val;
	}
	T val;
private:
	char tabPad[targetSize - sizeof(T)];
};

template<typename T, size_t targetSize> requires(targetSize == sizeof(T))
struct PadData<T, targetSize>
{
	PadData() : val() {}
	PadData(const T& copy) : val(copy) {}
	PadData(T&& move) noexcept : val(std::move(move)) {}
	PadData& operator=(const T& data)
	{
		val = data;
		return *this;
	}
	PadData& operator=(T&& data) noexcept
	{
		val = std::move(data);
		return *this;
	}
	operator T& ()
	{
		return val;
	}
	operator const T& () const
	{
		return val;
	}
	T* operator->()
	{
		return &val;
	}
	const T* operator->() const
	{
		return &val;
	}
	T val;
};

template<typename type, size_t maxCount>
struct StorageContainer
{
	static StorageContainer FromVec(const std::vector<type>& data)
	{
		if (data.size() > maxCount)
			throw std::out_of_range("to much");
		StorageContainer result;
		result.size = data.size();
		int i = 0;
		for (auto& it : data)
		{
			result.tab[i++] = it;
		}
		return result;
	}
	static uint32_t Capacity()
	{
		return sizeof(type) * maxCount;
	}
	type* Data()
	{
		return tab;
	}
	const type* Data() const
	{
		return tab;
	}
	size_t UploadSize() const
	{
		return size * sizeof(type);
	}
	size_t GetSize() const
	{
		return size;
	}
	uint32_t* GetSizeData()
	{
		return &size;
	}
	const uint32_t* GetSizeData() const
	{
		return &size;
	}
private:
	uint32_t size = 0;
	type tab[maxCount];
};