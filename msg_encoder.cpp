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
	std::bind(&std::remove_pointer<decltype(o)>::type::f, o)

// Function prototype that reads data from a given wrapper and places the right content on the given card.
using InlineCardRead = std::function<void(Buffer::ibufferw&, Core::Data::CardInfo*)>;

// Reads a single card location info from the given wrapper
// NOTE: Use void to not read a specific value at compile time
template<typename Controller, typename Location, typename Sequence, typename Position>
inline void ReadCardLocInfo(Buffer::ibufferw& wrapper, Core::Data::CardInfo* card)
{
	// Controller
	if constexpr(!std::is_same<void, Controller>())
		card->set_controller(wrapper->read<Controller>("controller"));

	// Location
	if constexpr(!std::is_same<void, Location>())
		card->set_location(wrapper->read<Location>("location"));

	// Sequence
	if constexpr(!std::is_same<void, Sequence>())
		card->set_sequence(wrapper->read<Sequence>("sequence"));

	// Position
	if constexpr(!std::is_same<void, Position>())
	{
		const auto pos = wrapper->read<Position>("position");
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

// Reads a card vector from the given wrapper.
// a CardSpawner must be given, which should give a new card each time it is called
// aditionally, two functions before and after the default card read can be given, in case aditional info should be read
template<typename Count, typename Location, typename Sequence, typename Position = void>
void ReadCardVector(Buffer::ibufferw& wrapper, CardSpawner cs, InlineCardRead bcr = nullptr, InlineCardRead acr = nullptr)
{
	auto count = wrapper->read<Count>(".size()");
	for(Count i = 0; i < count; i++)
	{
		wrapper->log("read card ", (int)i, " from vector");

		Core::Data::CardInfo* card = cs();
		
		// Before Card Read
		if(bcr) bcr(wrapper, card);

		// Card Code & Dirty Bit
		ToCardCode(wrapper->read<code_t>("card code"), card);

		// Location Info
		ReadCardLocInfo<player_t, Location, Sequence, Position>(wrapper, card);

		// After Card Read
		if(acr) acr(wrapper, card);
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
	Buffer::ibufferw wrapper{&pimpl->ib};
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
	Buffer::ibufferw wrapper(&pimpl->ib);
	auto specific = msg.mutable_specific();
	specific->set_player(wrapper->read<player_t>("player"));
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
	Buffer::ibufferw wrapper(&pimpl->ib);
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
