#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__
#include <cstdint> // uint8_t
#include <cstring> // std::memcpy
#include <utility> // std::forward, std::pair

#if defined(_DEBUG) // Change this with the right debug flag
#ifndef BUFFER_DEBUG
#define BUFFER_DEBUG
#endif // BUFFER_DEBUG

#include <ctime>
#include <sstream>
#include <string>
#include <fstream>
#include <iomanip> // std::setw

#include <type_traits> // std::is_base_of

constexpr unsigned n_width = 3;
const char* reason_unspecified = "Unspecified";
#endif // defined(_DEBUG)

// TODO: add a note explaining that this file does not follow the main project code style
// but is more near to that style used on the standard libraries

namespace YGOpen
{
namespace Buffer
{

// Type used to pass buffers around, keep in mind that
// this object does not have ownership of the underlying pointer.
using basic_buffer = std::pair<void*, std::size_t>;

// Syntatic sugar.
basic_buffer make_buffer(void* buffer, std::size_t size)
{
	return std::pair<void*, std::size_t>(buffer, size);
}

// Defines the type used for new positions.
using pos_type = int;

// Types of seeks
// beg: from the beggining
// end: from the end
// cur: relatively
typedef enum
{
	beg,
	end,
	cur
}seek_dir;

class buffer_base
{
public:
#ifdef BUFFER_DEBUG
	std::fstream log_file;

	template<typename ...Params>
	void log(Params&& ...args)
	{
		(log_file << ... << args);
		log_file.flush();
	}

#endif // BUFFER_DEBUG
	buffer_base()
	{
#ifdef BUFFER_DEBUG
		// Initialize logging feature.
		time_t rn = std::time(nullptr);
		char rn_str[100];
		std::strftime(rn_str, sizeof(rn_str), "%c", std::localtime(&rn));
		std::stringstream ss;
		ss << std::to_string(rn)
		   << "-bufferio-"
		   << std::hex << "0x" << reinterpret_cast<std::uintptr_t>(this)
		   << "-debug.log";
		std::string log_filename = ss.str();
		log_file.open(log_filename, std::ios::out);
		ss.str(std::string());
		ss.clear();
		log("Logging started at ", rn_str, " this buffer_base was created\n");
#endif // BUFFER_DEBUG
	}

	// Sets the working buffer and its boundaries to a new buffer.
	void open(const basic_buffer& buffer)
	{
#ifdef BUFFER_DEBUG
		log("Setting buffer at ", std::hex, "0x", reinterpret_cast<std::uintptr_t>(buffer.first));
		log(" with size ", std::dec, buffer.second, ".\n");
#endif // BUFFER_DEBUG
		sp = cp = reinterpret_cast<uint8_t*>(buffer.first);
		ep = sp + buffer.second;
	}
	
	void open(void* buffer, size_t length)
	{
		open(make_buffer(buffer, length));
	}

	buffer_base(const basic_buffer& buffer) : buffer_base()
	{
		open(buffer);
	}

	buffer_base(void* buffer, size_t length) : buffer_base()
	{
		open(buffer, length);
	}

	// Returns the current position of the buffer.
	// seek_dir::beg == returned value is relative to the beggining
	// otherwise it is relative to the end
	pos_type tell(const seek_dir dir = seek_dir::beg) const
	{
		if(dir == seek_dir::beg)
			return pos_type(cp - sp);
		return pos_type(ep - cp);
	}

	// Checks if the buffer has any operation available.
	bool good() const
	{
		return cp >= sp && cp <= ep;
	}

	// Checks if we consumed the entire buffer already.
	bool eof() const
	{
		return cp == ep;
	}

	// Move current position.
	// seek_dir::beg == restarts to the beggining plus off
	// seek_dir::end == position is set at the end plus off
	// seek_dir::cur == position is added relatively to the current position
	template<typename ...Reasons>
	void seek(const pos_type off, const seek_dir dir = seek_dir::beg,
	          [[maybe_unused]] Reasons&& ...args)
	{
#ifdef false
		log("Moving :", std::setw(n_width), off);
		if(dir == seek_dir::beg)
			log(" byte(s) from the beggining");
		else if(dir == seek_dir::end)
			log(" byte(s) from the end");
		else if(dir == seek_dir::cur)
			log(" byte(s) relatively");
		if(sizeof...(args) == 0)
			log(", Reason(s): ", reason_unspecified);
		else
			log(", Reason(s): ", std::forward<Reasons>(args)...);
		log(".\n");
#endif // BUFFER_DEBUG
		if(dir == seek_dir::beg)
			cp = sp + off;
		else if(dir == seek_dir::end)
			cp = ep + off;
		else if(dir == seek_dir::cur)
			cp = cp + off;
	}
protected:
	uint8_t* sp{nullptr}; // starting pos
	uint8_t* cp{nullptr}; // current pos
	uint8_t* ep{nullptr}; // ending pos
};

class ibuffer : virtual public buffer_base
{
public:
	ibuffer() : buffer_base() {};
	ibuffer(const basic_buffer& buffer) : buffer_base(buffer) {};

	// Read any object from the current buffer.
	// Aditionally, moves the current position forward
	// the same number of bytes read.
	template<typename T, typename ...Reasons>
	T read([[maybe_unused]] Reasons&& ...args)
	{
#ifdef BUFFER_DEBUG
		log("Reading:", std::setw(n_width), sizeof(T), " byte(s)");
		if(sizeof...(args) == 0)
			log(", Reason(s): ", reason_unspecified);
		else
			log(", Reason(s): ", std::forward<Reasons>(args)...);
		log(".\n");
#endif // BUFFER_DEBUG
		T ret = read_base<T>();
		seek(sizeof(T), seek_dir::cur, "called from ibuffer::read()");
		return ret;
	}
private:
	template<typename T>
	const T read_base() const
	{
		T ret;
		std::memcpy(&ret, cp, sizeof(T));
		return ret;
	}
};

class obuffer : virtual public buffer_base
{
public:
	obuffer() : buffer_base() {};
	obuffer(const basic_buffer& buffer) : buffer_base(buffer) {};

	// Writes any object to the current buffer.
	// Aditionally, moves the current position forward
	// the same number of bytes written.
	template<typename T, typename ...Reasons>
	void write(T&& val, [[maybe_unused]] Reasons&& ...args)
	{
#ifdef BUFFER_DEBUG
		log("Writing:", std::setw(n_width), sizeof(T), " byte(s)");
		if(sizeof...(args) == 0)
			log(", Reason(s): ", reason_unspecified);
		else
			log(", Reason(s): ", std::forward<Reasons>(args)...);
		log(".\n");
#endif // BUFFER_DEBUG
		write_base(std::forward<T>(val));
		seek(sizeof(T), seek_dir::cur, "called from obuffer::write()");
	}
private:
	template<typename T>
	void write_base(T val)
	{
		std::memcpy(cp, &val, sizeof(T));
	}
};

class iobuffer : public ibuffer, public obuffer
{
public:
	iobuffer() : buffer_base() {};
	iobuffer(const basic_buffer& buffer) : buffer_base(buffer) {};
};

template<typename BufferType>
class buffer_wrapper
{
	static_assert(std::is_base_of<buffer_base, BufferType>::value,
	              "BufferType must inherit from buffer_base");
public:
	buffer_wrapper(BufferType* buf) : buff_ptr(buf)
	{
#ifdef BUFFER_DEBUG
		buff_ptr->log("Wrapper created.\n");
#endif // BUFFER_DEBUG
	}

#ifdef BUFFER_DEBUG
	~buffer_wrapper()
	{
		buff_ptr->log("Wrapper destroyed. ");
		buff_ptr->log("byte(s) read/written: ", buff_ptr->tell(), ". ");
		buff_ptr->log("byte(s) leftover: ", buff_ptr->tell(seek_dir::end), ".\n");
	}
#endif // BUFFER_DEBUG

	BufferType* operator->() const
	{
		return buff_ptr;
	}
private:
	BufferType* buff_ptr;
};

using ibufferw = buffer_wrapper<Buffer::ibuffer>;
using obufferw = buffer_wrapper<Buffer::obuffer>;
using iobufferw = buffer_wrapper<iobuffer>;

} // namespace Buffer
} // namespace YGOpen

#endif // __BUFFER_HPP__
