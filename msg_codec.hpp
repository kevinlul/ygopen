#ifndef __MSG_CODEC_HPP__
#define __MSG_CODEC_HPP__
#include <memory> // std::unique_ptr
#include "core_msg.pb.h"

namespace YGOpen
{

// Game Message Stream Base
class MsgCodecBase
{
protected:
	Core::AnyMsg lastMsg;
};

// Input Game Message Encoder
// takes a memory buffer and "encodes" it onto a AnyMsg protobuf message
class MsgEncoder : virtual public MsgCodecBase
{
	struct impl;
	std::unique_ptr<impl> pimpl;
	
	inline void RequestMsg(int msgType, Core::AnyMsg& msg);
	inline void InformationMsg(int msgType, Core::AnyMsg& msg);
	inline void SpecInformationMsg(int msgType, Core::AnyMsg& msg);
public:
	MsgEncoder();

	MsgEncoder(const MsgEncoder&) = delete;
	MsgEncoder(MsgEncoder&&) = delete;
	MsgEncoder& operator=(const MsgEncoder&) = delete;
	MsgEncoder& operator=(MsgEncoder&&) = delete;

	~MsgEncoder();

	// Input: A buffer and buffer length
	// Output: A encoded message (if any)
	Core::AnyMsg Encode(void* buffer, size_t length);
};

// Output Game Message Decoder
// takes a protobuf message and "decodes" it onto a core compatible buffer
class MsgDecoder : virtual public MsgCodecBase
{
};

// Input/Output Game Message Stream
class MsgCodec : public MsgEncoder, public MsgDecoder
{
};

} // namespace YGOpen

#endif // __MSG_CODEC_HPP__
