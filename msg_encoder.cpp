#include <functional>
#include <bitset>
#include <array>


#include "msg_codec.hpp"
#include "buffer.hpp"
#include "core_message.hpp"

namespace YGOpen
{
// Types that represent the right size type for what they hold
using player_t = uint8_t; // used for controller too
using cardcount_t = uint32_t;
using cardcode_t = uint32_t;
using location_t = uint32_t;
using sequence_t = uint32_t;
using position_t = uint32_t;
using effectdesc_t = uint64_t;

using field_cardcount_t = uint8_t;

// Types that should be changed (increased) in far future
using small_location_t = uint8_t;
using small_sequence_t = uint8_t;
using small_position_t = uint8_t;

// Used to identify which messages are part of the different hierarchies
using msg_type_map = std::array<bool, 255>;

// NOTE: write info location sizes:
// controller = 1 byte (player_t)
// location = 1 byte (small_location_t)
// sequence = 4 bytes (sequence_t)
// position = 4 bytes (position_t)

// parses a effectdesc_t and puts information on to the given EffectDesc
void ToEffectDesc(const effectdesc_t ed, Core::Data::EffectDesc* msg)
{
	msg->set_code(ed >> 4);
	msg->set_string_id(ed & 0xF);
}

// reads a card code and sets the right values of a Data.CardInfo message
void ToCardCode(const cardcode_t code, Core::Data::CardInfo* card)
{
	card->set_code(code & 0x7FFFFFFF); // Do not include last bit
	card->set_bit(code & 0x80000000);
}

// Function prototype that returns a CardInfo (newly created most likely).
// The returned card should be inside of a array,
// managed by a GMsg; normally the [gmsg]->add_* functions
using CardSpawner = std::function<Core::Data::CardInfo*()>;

// Binds a function from a object pointer, without needing to write the full object path.
// NOTE: Can be improved but this works right now.
#define BindFromPointer(object, member) std::bind(&std::remove_pointer<decltype(object)>::type::member, object)

// Function prototype that reads data from a given wrapper and places the right content on the given card.
using InlineCardRead = std::function<void(Buffer::ibufferw&, const int, Core::Data::CardInfo*)>;

// Reads a single card location info from the given wrapper
// NOTE: Use void to not read a specific value at compile time
template<typename Controller, typename Location, typename Sequence, typename Position>
inline void ReadCardLocInfo(Buffer::ibufferw& wrapper, const int count, Core::Data::CardInfo* card)
{
	// Controller
	if constexpr(!std::is_same<void, Controller>())
		card->set_controller(wrapper->read<Controller>("controller ", count));

	// Location
	if constexpr(!std::is_same<void, Location>())
		card->set_location(wrapper->read<Location>("location ", count));

	// Sequence
	if constexpr(!std::is_same<void, Sequence>())
		card->set_sequence(wrapper->read<Sequence>("sequence ", count));

	// Position
	if constexpr(!std::is_same<void, Position>())
	{
		const auto pos = wrapper->read<Position>("position ", count);
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
	const Count count = wrapper->read<Count>(".size()");
	for(Count i = 0; i < count; i++)
	{
		Core::Data::CardInfo* card = cs();
		
		// Before Card Read
		if(bcr) bcr(wrapper, i, card);

		// Card Code & Dirty Bit
		ToCardCode(wrapper->read<cardcode_t>("card code ", (int)i), card);

		// Location Info
		ReadCardLocInfo<player_t, Location, Sequence, Position>(wrapper, i, card);

		// After Card Read
		if(acr) acr(wrapper, i, card);
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
