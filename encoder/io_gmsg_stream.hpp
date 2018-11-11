namespace YGOpen
{

// Game Message Stream Base
class GMsgStreamBase
{

}

// Input Game Message Encoder
// takes a raw buffer from the core and transform it into a GMsg protobuf message
class IGMsgEncoder : virtual public GMsgStreamBase
{

}

// Output Game Message Decoder
// takes a GMsg protobuf message and transform it into a core readable answer buffer
class OGMsgDecoder : virtual public GMsgStreamBase
{

}

// Input/Output Game Message Stream
class IOGMsgStream : public IGMsgEncoder, public OGMsgDecoder
{

}

} // namespace YGOpen
