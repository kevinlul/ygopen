#include <functional>
#include <bitset>
#include <array>


#include "msg_codec.hpp"
#include "buffer.hpp"
#include "core_message.hpp"

namespace YGOpen
{
// Types used to read the right values size from core messages
using player_t = uint8_t;  // used for controller too
using count_t  = uint32_t; // count type, used for arrays read
using code_t   = uint32_t; // card code type
using loc_t    = uint32_t; // location type
using seq_t    = uint32_t; // sequence type
using pos_t    = uint32_t; // position type
using ed_t     = uint64_t; // effect descriptor type

// Smaller (used) versions of the above types
using s_count_t = uint8_t;
using s_loc_t   = uint8_t;
using s_seq_t   = uint8_t;
using s_pos_t   = uint8_t;

// NOTE: core write_info_location type and sizes:
// controller = 1 byte (player_t)
// location = 1 byte (s_loc_t)
// sequence = 4 bytes (seq_t)
// position = 4 bytes (pos_t)

// Used to identify which messages are part of the different hierarchies
using msg_type_map = std::array<bool, 255>;

// Parses a ed_t and puts information on to the given EffectDesc
void ToEffectDesc(const ed_t ed, Core::Data::EffectDesc* msg)
{
	msg->set_code(ed >> 4);
	msg->set_string_id(ed & 0xF);
}

// Reads a card code and puts information on to the given CardInfo
void ToCardCode(const code_t code, Core::Data::CardInfo* card)
{
	card->set_code(code & 0x7FFFFFFF); // Do not include last bit
	card->set_bit(code & 0x80000000);
}

// Function prototype that returns a different CardInfo on consecutive calls,
// normally the protobuf msg->add_* functions
using CardSpawner = std::function<Core::Data::CardInfo*()>;

// Shorthand to bind a object function address to a object pointer
#define BIND_FUNC_TO_OBJ_PTR(o, f) \
	std::bind(&std::remove_pointer_t<decltype(o)>::f, o)

// Reads a single card location info from the given wrapper
// NOTE: Use void to not read a specific value at compile time
template<typename Player, typename Loc, typename Seq, typename Pos>
inline void ReadInfoLoc(Buffer::ibufferw& w, Core::Data::CardInfo* card)
{
	if constexpr(!std::is_same<void, Player>())
		card->set_controller(w->read<Player>("controller"));

	if constexpr(!std::is_same<void, Loc>())
		card->set_location(w->read<Loc>("location"));

	if constexpr(!std::is_same<void, Seq>())
		card->set_sequence(w->read<Seq>("sequence"));

	if constexpr(!std::is_same<void, Pos>())
	{
		const auto pos = w->read<Pos>("position");
		if(card->location() & 0x80) // if the card is overlay
		{
			card->set_overlay_sequence(pos);
			card->set_position(0x2); // face-up attack
		}
		else
		{
			card->set_position(pos);
		}
	}
}

// Shorthand for the function used in the core to write location info
#define READ_INFO_LOC_CORE ReadInfoLoc<player_t, s_loc_t, seq_t, pos_t>

// Function prototype that reads data from a given wrapper,
// and puts information on the given CardInfo.
using CardRead = std::function<void(Buffer::ibufferw&, Core::Data::CardInfo*)>;

// Reads a card vector from the given wrapper.
// a optional CardRead function can be given to read aditional info.
template<typename Count, typename Loc, typename Seq, typename Pos = void>
void ReadCardVector(Buffer::ibufferw& w, CardSpawner cs, CardRead pos = nullptr)
{
	auto count = w->read<Count>(".size()");
	for(Count i = 0; i < count; i++)
	{
		w->log("read card ", (int)i, " from vector\n");

		Core::Data::CardInfo* card = cs();

		// Card Code & Dirty Bit
		ToCardCode(w->read<code_t>("card code"), card);

		// Location Info
		ReadInfoLoc<player_t, Loc, Seq, Pos>(w, card);

		// Postfix read
		if(pos) pos(w, card);
	}
}

struct MsgEncoder::impl
{
	Buffer::ibuffer ib{};
};

MsgEncoder::MsgEncoder() : pimpl(new impl())
{
}

MsgEncoder::~MsgEncoder() = default;

// INFORMATION MESSAGES
inline void MsgEncoder::InformationMsg(int msgType, Core::AnyMsg& msg)
{
	Buffer::ibufferw w{&pimpl->ib};
	auto information = msg.mutable_information();
	switch(msgType)
	{
#include "encoder_information_impl.hpp"
	}
}

static const msg_type_map MSG_INFORMATION_MAP = []
{
	const auto CheckOne = [](int i)
	{
		switch(i)
		{
#define FILTERING
#include "encoder_information_impl.hpp"
		}
	};
	msg_type_map a;
	for(int i = 0; i < a.max_size(); i++)
		a[i] = CheckOne(i);
	return a;
}();

// SPECIFIC REQUEST MESSAGES
inline void MsgEncoder::RequestMsg(int msgType, Core::AnyMsg& msg)
{
	Buffer::ibufferw w(&pimpl->ib);
	auto specific = msg.mutable_specific();
	specific->set_player(w->read<player_t>("player"));
	switch(msgType)
	{
#include "encoder_request_impl.hpp"
	}
}

static const msg_type_map MSG_REQUEST_MAP = []
{
	const auto CheckOne = [](int i)
	{
		switch(i)
		{
#define FILTERING
#include "encoder_request_impl.hpp"
		}
	};
	msg_type_map a;
	for(int i = 0; i < a.max_size(); i++)
		a[i] = CheckOne(i);
	return a;
}();

// SPECIFIC INFORMATION MESSAGES
inline void MsgEncoder::SpecInformationMsg(int msgType, Core::AnyMsg& msg)
{
	Buffer::ibufferw w(&pimpl->ib);
	auto specific = msg.mutable_specific();
	switch(msgType)
	{
#include "encoder_spec_information_impl.hpp"
	}
}

static const msg_type_map MSG_SPEC_INFORMATION_MAP = []
{
	const auto CheckOne = [](int i)
	{
		switch(i)
		{
#define FILTERING
#include "encoder_spec_information_impl.hpp"
		}
	};
	msg_type_map a;
	for(int i = 0; i < a.max_size(); i++)
		a[i] = CheckOne(i);
	return a;
}();

Core::AnyMsg MsgEncoder::Encode(void* buffer, size_t length)
{
	Core::AnyMsg msg{};
	pimpl->ib.open(buffer, length);
	pimpl->ib.log("Message Type: ", msgType, " (", MSG_NAMES.at(msgType), ')', '\n');
	const int msgType = (int)pimpl->ib.read<uint8_t>("msg number");
	
	if(MSG_INFORMATION_MAP[msgType])
		InformationMsg(msgType, msg);
	else if(MSG_REQUEST_MAP[msgType])
		RequestMsg(msgType, msg);
	else if(MSG_SPEC_INFORMATION_MAP[msgType])
		SpecInformationMsg(msgType, msg);
	else
		pimpl->ib.log("Warning: Unhandled Message\n");
	
	return (lastMsg = msg);
}

} // namespace YGOpen
