#ifndef __IO_GMSG_STREAM_HPP__
#define __IO_GMSG_STREAM_HPP__
#include <memory> // std::unique_ptr
#include "gmsg/core_gmsg.pb.h"

namespace YGOpen
{

// Game Message Stream Base
class GMsgStreamBase
{
	Core::GMsg lastMessage;
};

// Input Game Message Encoder
// takes a raw buffer from the core and transform it into a GMsg protobuf message
class IGMsgEncoder : virtual public GMsgStreamBase
{
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	IGMsgEncoder();
	~IGMsgEncoder();
	Core::GMsg Encode(void* buffer, size_t length);
};

// Output Game Message Decoder
// takes a GMsg protobuf message and transform it into a core readable answer buffer
class OGMsgDecoder : virtual public GMsgStreamBase
{
};

// Input/Output Game Message Stream
class IOGMsgStream : public IGMsgEncoder, public OGMsgDecoder
{

};

} // namespace YGOpen

#endif // __IO_GMSG_STREAM_HPP__
