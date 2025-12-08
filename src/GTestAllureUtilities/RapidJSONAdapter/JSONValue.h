#pragma once

#include "JSONAdapterInterface/IJSONValue.h"

#include <string>
#include <map>
#include <memory>
#include <vector>

#include <rapidjson/document.h>


namespace systelab { namespace json { namespace rapidjson {

	class JSONDocument;
	class JSONMember;

	class JSONValue : public IJSONValue
	{
	public:
		JSONValue(JSONDocument&, ::rapidjson::Value&, ::rapidjson::Document::AllocatorType&);
		virtual ~JSONValue();

		Type getType() const override;
		void setType(Type) override;

		// Only for boolean values
		bool getBoolean() const override;
		void setBoolean(bool) override;

		// Only for string values
		std::string getString() const override;
		void setString(const std::string&) override;

		// Only for number values
		bool isInteger() const override;
		int getInteger() const override;
		double getDouble() const override;

		void setInteger(int) override;
		void setLong(long) override;
		void setLongLong(long long) override;
		void setDouble(double) override;

		// Only for Null values
		bool isNull() const override;
		void setNull() override;
		
		// Only for object values
		unsigned int getObjectMemberCount() const override;
		bool hasObjectMember(const std::string&) const override;
		std::vector<std::string> getObjectMemberNames() const override;
		IJSONValue& getObjectMemberValue(const std::string&) const override;

		void addMember(const std::string& name, bool value) override;
		void addMember(const std::string& name, int value) override;
		void addMember(const std::string& name, long value) override;
		void addMember(const std::string& name, long long value) override;
		void addMember(const std::string& name, double value) override;
		void addMember(const std::string& name, const char* value) override;
		void addMember(const std::string& name, const std::string& value) override;
		void addMember(const std::string& name, std::unique_ptr<IJSONValue>) override;
		void removeMember(const std::string& name) override;

		// Only for array values
		unsigned int getArrayValueCount() const override;
		IJSONValue& getArrayValue(unsigned int) const override;

		void addArrayValue(std::unique_ptr<IJSONValue>) override;
		void clearArray() override;

		// JSON pointer
		IJSONValue* getJSONPointerValue(const std::string& jsonPointer) override;
		const IJSONValue* getJSONPointerValue(const std::string& jsonPointer) const override;

		// Factory of values
		std::unique_ptr<IJSONValue> buildValue(Type) const override;
		std::unique_ptr<IJSONDocument> buildDocument() const override;

	private:
		void loadObjectMembers() const;
		void clearObjectMembers();

		void loadArrayValues() const;
		void clearArrayValues();

	private:
		JSONDocument& m_document;
		::rapidjson::Value& m_value;
		::rapidjson::Value::AllocatorType& m_allocator;

		mutable bool m_objectMembersLoaded;
		mutable std::map< std::string, std::unique_ptr<JSONMember> > m_objectMembers;

		mutable bool m_arrayValuesLoaded;
		mutable std::vector< std::unique_ptr<IJSONValue> > m_arrayValues;
	};

}}}
