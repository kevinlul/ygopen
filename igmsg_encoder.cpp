#include "io_gmsg_stream.hpp"
#include "buffer.hpp"
#include "core_message.hpp"

namespace YGOpen
{

struct IGMsgEncoder::impl
{
	Buffer::ibuffer ib;
};

IGMsgEncoder::IGMsgEncoder() : pimpl(new impl())
{}

IGMsgEncoder::~IGMsgEncoder() = default;

inline void IGMsgEncoder::SpecificMsg(Core::GMsg& gmsg, const int msgType)
{
	Buffer::buffer_wrapper<Buffer::ibuffer> wrapper(&pimpl->ib);
}

inline void IGMsgEncoder::InformationMsg(Core::GMsg& gmsg, const int msgType)
{
	Buffer::buffer_wrapper<Buffer::ibuffer> wrapper(&pimpl->ib);
}

Core::GMsg IGMsgEncoder::Encode(void* buffer, size_t length)
{
	Core::GMsg gmsg{};

	pimpl->ib.open(Buffer::make_buffer(buffer, length));

	const auto msgType = pimpl->ib.read<uint8_t>("Encode: message type");
	switch(msgType)
	{
		// Specific messages
		case SelectBattleCmd:
		case SelectIdleCmd:
		{
			SpecificMsg(gmsg, msgType);
		}
		break;
		// Information messages
		case Win:
		case MatchKill:
		{
			InformationMsg(gmsg, msgType);
		}
		break;
		default:
		{
			// UNHANDLED
		}
		break;
	}

	return (lastGMsg = gmsg);
}


} // namespace YGOpen
