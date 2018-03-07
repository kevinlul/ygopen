#ifndef __BUFFER_MANIPULATOR_HPP__ 
#define __BUFFER_MANIPULATOR_HPP__
#include <cstdint>
#include <cassert>
#include <cstring>

class BufferManipulator
{
	uint8_t* sp;
	uint8_t* cp;
	uint8_t* ep;
public:
	BufferManipulator(void* data, int len)
	{
		sp = cp = reinterpret_cast<uint8_t*>(data);
		ep = sp + len;
	}

	template<typename T>
	void Write(T data)
	{
		assert(!(cp + sizeof(T) > ep)); // NO write out of bounds
		std::memcpy(cp, &data, sizeof(T));
		cp += sizeof(T);
	}

	template<typename T>
	T Read()
	{
		assert(!(cp + sizeof(T) > ep)); // NO read out of bounds
		T ret;
		std::memcpy(&ret, cp, sizeof(T));
		cp += sizeof(T);
		return ret;
	}
	
	void Forward(uint32_t bytes)
	{
		assert(!(cp + bytes > ep)); // NO forward out of bounds
		cp += bytes;
	}

	void Backward(uint32_t bytes)
	{
		assert(!(cp - bytes < sp)); // NO backward out of bounds
		cp -= bytes;
	}

	bool CanAdvance()
	{
		return cp != ep;
	}
};

#endif // __BUFFER_MANIPULATOR_HPP__
