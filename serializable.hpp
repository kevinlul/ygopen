#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP
#include <string>
#include <nlohmann/json.hpp>
#include "google/protobuf/util/json_util.h"

template<typename T, typename PbMsg>
class Serializable
{
public:
	// Deserializing
	static T FromJSON(const nlohmann::json& j)
	{
		using google::protobuf::util::JsonStringToMessage;
		T t;
		JsonStringToMessage(j.dump(), &t.pbMsg, {});
		return t;
	}
	
	static T FromMsg(const PbMsg& pb)
	{
		T t;
		t.pbMsg = pb;
		return t;
	}
	
	static T FromWire(const std::string& data)
	{
		T t;
		t.pbMsg.ParseFromString(data);
		return t;
	}
	
	// Serializing
	nlohmann::json ToJSON() const
	{
		using google::protobuf::util::MessageToJsonString;
		using google::protobuf::util::JsonPrintOptions;
		std::string str;
		JsonPrintOptions opts;
		opts.add_whitespace = false;
		opts.always_print_primitive_fields = true;
		MessageToJsonString(pbMsg, &str, opts);
		return nlohmann::json::parse(str);
	}
	
	std::string ToWire() const
	{
		std::string str;
		pbMsg.SerializeToString(&str);
		return str;
	}
	
	const PbMsg& GetMsg() const
	{
		return pbMsg;
	}
protected:
	PbMsg pbMsg;
};

#endif // SERIALIZABLE_HPP
