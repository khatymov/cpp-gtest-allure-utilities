#pragma once

#include "JSONAdapterInterface/IJSONValue.h"

#include <rapidjson/document.h>


namespace systelab { namespace json { namespace rapidjson {

	class JSONDocument;

	class JSONMember
	{
	public:
		JSONMember(JSONDocument&, const std::string&, ::rapidjson::Value&, ::rapidjson::Document::AllocatorType&);
		virtual ~JSONMember();

		IJSONValue& getValue() const;

	private:
		std::string m_name;
		std::unique_ptr<IJSONValue> m_value;
	};

}}}
