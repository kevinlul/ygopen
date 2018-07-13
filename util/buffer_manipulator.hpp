#ifndef __BUFFER_MANIPULATOR_HPP__ 
#define __BUFFER_MANIPULATOR_HPP__
#include <cstdint>
#include <cassert>
#include <cstring>
#include <utility>

class BufferManipulator
{
	uint8_t* sp;
	uint8_t* cp;
	uint8_t* ep;
public:
	BufferManipulator(void* data, size_t len)
	{
		sp = cp = reinterpret_cast<uint8_t*>(data);
		ep = sp + len;
	}
	
	std::pair<void*, size_t> GetWrittenBuffer() const
	{
		return std::make_pair((void*)sp, size_t(cp - sp));
	}

	std::pair<void*, size_t> GetCurrentBuffer() const
	{
		return std::make_pair((void*)cp, size_t(ep - cp));
	}

	size_t GetCurrentLength() const
	{
		return size_t(cp - sp);
	}

	template<typename T>
	void Write(T data)
	{
		assert(!(cp + sizeof(T) > ep)); // NO write out of bounds
		std::memcpy(cp, &data, sizeof(T));
		cp += sizeof(T);
	}
	
	void Write(BufferManipulator& bm)
	{
		std::pair<void*, size_t> buffer = bm.GetWrittenBuffer();
		assert(!(cp + buffer.second > ep)); // NO write out of bounds
		std::memcpy(cp, buffer.first, buffer.second);
		cp += buffer.second;
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
	
	void Forward(size_t bytes)
	{
		assert(!(cp + bytes > ep)); // NO forward out of bounds
		cp += bytes;
	}

	void Backward(size_t bytes)
	{
		assert(!(cp - bytes < sp)); // NO backward out of bounds
		cp -= bytes;
	}

	void ToStart()
	{
		cp = sp;
	}

	bool CanAdvance()
	{
		return cp != ep;
	}
};

#endif // __BUFFER_MANIPULATOR_HPP__
