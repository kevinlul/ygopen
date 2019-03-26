#ifndef __MSG_CODEC_HPP__
#define __MSG_CODEC_HPP__
#include <memory> // std::unique_ptr
#include "generated/core_msg.pb.h"

namespace YGOpen
{

// Game Message Stream Base
class MsgCodecBase
{
protected:
	Core::AnyMsg lastMsg;
};

// Input Game Message Encoder
// takes a raw buffer from the core and transform it into a GMsg protobuf message
class MsgEncoder : virtual public MsgCodecBase
{
	struct impl;
	std::unique_ptr<impl> pimpl;
	
	inline bool SpecificRequestMsg(Core::AnyMsg& msg, const int msgType);
	inline bool SpecificInformationMsg(Core::AnyMsg& msg, const int msgType);
	inline bool InformationMsg(Core::AnyMsg& msg, const int msgType);
public:
	MsgEncoder();

	MsgEncoder(const MsgEncoder&) = delete;
	MsgEncoder(MsgEncoder&&) = delete;
	MsgEncoder& operator=(const MsgEncoder&) = delete;
	MsgEncoder& operator=(MsgEncoder&&) = delete;

	~MsgEncoder();

	// Input: A buffer and buffer length
	// Output: A encoded message (if any) and a boolean telling if we got a message
	Core::AnyMsg Encode(void* buffer, size_t length, bool& encoded);
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

#endif // __MSG_CODEC_HPP__
