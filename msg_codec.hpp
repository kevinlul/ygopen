#ifndef __IO_GMSG_STREAM_HPP__
#define __IO_GMSG_STREAM_HPP__
#include <memory> // std::unique_ptr
#include "generated/core_gmsg.pb.h"

namespace YGOpen
{

// Game Message Stream Base
class MsgCodecBase
{
protected:
	Core::GMsg lastGMsg;
};

// Input Game Message Encoder
// takes a raw buffer from the core and transform it into a GMsg protobuf message
class MsgEncoder : virtual public MsgCodecBase
{
	struct impl;
	std::unique_ptr<impl> pimpl;
	
	inline void SpecificMsg(Core::GMsg& gmsg, const int msgType);
	inline void InformationMsg(Core::GMsg& gmsg, const int msgType);
public:
	MsgEncoder();

	MsgEncoder(MsgEncoder&&) = delete;
	MsgEncoder(const MsgEncoder&) = delete;
	MsgEncoder& operator=(MsgEncoder&&) = delete;
	MsgEncoder& operator=(const MsgEncoder&) = delete;

	~MsgEncoder();

	Core::GMsg Encode(void* buffer, size_t length);
};

// Output Game Message Decoder
// takes a GMsg protobuf message and transform it into a core readable answer buffer
class MsgDecoder : virtual public MsgCodecBase
{
};

// Input/Output Game Message Stream
class MsgCodec : public MsgEncoder, public MsgDecoder
{

};

} // namespace YGOpen

#endif // __IO_GMSG_STREAM_HPP__
