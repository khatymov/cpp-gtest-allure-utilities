#include "JSONValue.h"

#include "JSONDocument.h"
#include "JSONMember.h"

#include <stdexcept>


namespace systelab { namespace json { namespace rapidjson {

	JSONValue::JSONValue(JSONDocument& document,
						 ::rapidjson::Value& value,
						 ::rapidjson::Document::AllocatorType& allocator)
		:m_document(document)
		,m_value(value)
		,m_allocator(allocator)
		,m_objectMembersLoaded(false)
		,m_objectMembers()
		,m_arrayValuesLoaded(false)
		,m_arrayValues()
	{
	}

	JSONValue::~JSONValue() = default;

	Type JSONValue::getType() const
	{
		::rapidjson::Type type = m_value.GetType();
		switch (type)
		{
			case ::rapidjson::kFalseType:
			case ::rapidjson::kTrueType:
				return BOOLEAN_TYPE;
			case ::rapidjson::kObjectType:
				return OBJECT_TYPE;
			case ::rapidjson::kArrayType:
				return ARRAY_TYPE;
			case ::rapidjson::kStringType:
				return STRING_TYPE;
			case ::rapidjson::kNumberType:
				return NUMBER_TYPE;
			case ::rapidjson::kNullType:
			default:
				return NULL_TYPE;
		}
	}

	void JSONValue::setType(Type type)
	{
		switch (type)
		{
			case BOOLEAN_TYPE:
				m_value.SetBool(false);
				clearObjectMembers();
				clearArrayValues();
				break;

			case OBJECT_TYPE:
				m_value.SetObject();
				clearArrayValues();
				break;

			case ARRAY_TYPE:
				m_value.SetArray();
				clearObjectMembers();
				break;

			case STRING_TYPE:
				m_value.SetString("");
				clearObjectMembers();
				clearArrayValues();
				break;

			case NUMBER_TYPE:
				m_value.SetInt(0);
				clearObjectMembers();
				clearArrayValues();
				break;

			case NULL_TYPE:
			default:
				m_value.SetNull();
				clearObjectMembers();
				clearArrayValues();
				break;
		}
	}

	bool JSONValue::getBoolean() const
	{
		return m_value.GetBool();
	}

	void JSONValue::setBoolean(bool value)
	{
		m_value.SetBool(value);
	}

	std::string JSONValue::getString() const
	{
		return m_value.GetString();
	}

	void JSONValue::setString(const std::string& value)
	{
		m_value.SetString(value, m_allocator);
	}

	bool JSONValue::isInteger() const
	{
		return m_value.IsInt();
	}

	int JSONValue::getInteger() const
	{
		return m_value.GetInt();
	}

	double JSONValue::getDouble() const
	{
		return m_value.GetDouble();
	}

	void JSONValue::setInteger(int value)
	{
		m_value.SetInt(value);
	}

	void JSONValue::setLong(long value)
	{
		m_value.SetInt64(value);
	}

	void JSONValue::setLongLong(long long value)
	{
		m_value.SetInt64(value);
	}

	void JSONValue::setDouble(double value)
	{
		m_value.SetDouble(value);
	}

	bool JSONValue::isNull() const
	{
		return m_value.IsNull();
	}

	void JSONValue::setNull()
	{
		m_value.SetNull();
	}

	unsigned int JSONValue::getObjectMemberCount() const
	{
		return m_value.GetObject().MemberCount();
	}

	bool JSONValue::hasObjectMember(const std::string& name) const
	{
		return m_value.HasMember(name);
	}

	std::vector<std::string> JSONValue::getObjectMemberNames() const
	{
		std::vector<std::string> memberNames;
		for (auto itr = m_value.MemberBegin(); itr != m_value.MemberEnd(); ++itr)
		{
			memberNames.push_back(itr->name.GetString());

		}

		return memberNames;
	}

	IJSONValue& JSONValue::getObjectMemberValue(const std::string& name) const
	{
		loadObjectMembers();

		auto itr = m_objectMembers.find(name);
		if (itr != m_objectMembers.end())
		{
			return itr->second->getValue();
		}
		else
		{
			throw std::runtime_error(std::string("Member '" + name + "' not found").c_str());
		}
	}

	void JSONValue::addMember(const std::string& name, bool value)
	{
		std::unique_ptr<IJSONValue> newBooleanValue = buildValue(BOOLEAN_TYPE);
		newBooleanValue->setBoolean(value);
		addMember(name, std::move(newBooleanValue));
	}

	void JSONValue::addMember(const std::string& name, int value)
	{
		std::unique_ptr<IJSONValue> newNumberValue = buildValue(NUMBER_TYPE);
		newNumberValue->setInteger(value);
		addMember(name, std::move(newNumberValue));
	}

	void JSONValue::addMember(const std::string& name, long value)
	{
		std::unique_ptr<IJSONValue> newNumberValue = buildValue(NUMBER_TYPE);
		newNumberValue->setLong(value);
		addMember(name, std::move(newNumberValue));
	}

	void JSONValue::addMember(const std::string& name, long long value)
	{
		std::unique_ptr<IJSONValue> newNumberValue = buildValue(NUMBER_TYPE);
		newNumberValue->setLongLong(value);
		addMember(name, std::move(newNumberValue));
	}

	void JSONValue::addMember(const std::string& name, double value)
	{
		std::unique_ptr<IJSONValue> newNumberValue = buildValue(NUMBER_TYPE);
		newNumberValue->setDouble(value);
		addMember(name, std::move(newNumberValue));
	}

	void JSONValue::addMember(const std::string& name, const char* value)
	{
		addMember(name, std::string(value));
	}

	void JSONValue::addMember(const std::string& name, const std::string& value)
	{
		std::unique_ptr<IJSONValue> newStringValue = buildValue(STRING_TYPE);
		newStringValue->setString(value);
		addMember(name, std::move(newStringValue));
	}

	void JSONValue::addMember(const std::string& name, std::unique_ptr<IJSONValue> valueToAdd)
	{
		loadObjectMembers();

		JSONValue* adapterValueToAdd = dynamic_cast<JSONValue*>(valueToAdd.get());
		if (!adapterValueToAdd)
		{
			throw std::runtime_error("JSONValue::addMember() Provided value is not valid");
		}

		if (&adapterValueToAdd->m_document != &m_document)
		{
			throw std::runtime_error("JSONValue::addMember() Provided value does not belong to this document");
		}

		std::unique_ptr<::rapidjson::Value> freeValue = m_document.removeFreeValue(adapterValueToAdd->m_value);

		::rapidjson::Value memberName(name, m_allocator);
		m_value.AddMember(memberName, *freeValue, m_allocator);

		std::unique_ptr<JSONMember> member = std::make_unique<JSONMember>(m_document, name, m_value[name], m_allocator);
		m_objectMembers.insert(std::make_pair(name, std::move(member)));
	}

	void JSONValue::removeMember(const std::string& name)
	{
		if (m_value.MemberCount() == 0)
		{
			return;
		}

		loadObjectMembers();

		{
			/*RapidJSON's RemoveMember() swaps JSONValue element between the element to be removed and its last element. 
			* This modification swaps elements beforehand so after RemoveMember() is called the last element is not lost*/

			auto last = --(m_value.MemberEnd());
			auto lastName = last->name.GetString();
			auto toBeRemovedElement = m_objectMembers.find(name);
			if (toBeRemovedElement != m_objectMembers.end())
			{
				toBeRemovedElement->second.swap(m_objectMembers.find(lastName)->second);
			}
		}

		m_value.RemoveMember(name);
		m_objectMembers.erase(name);

	}

	unsigned int JSONValue::getArrayValueCount() const
	{
		return m_value.GetArray().Size();
	}

	IJSONValue& JSONValue::getArrayValue(unsigned int index) const
	{
		loadArrayValues();

		return *m_arrayValues[index];
	}

	void JSONValue::addArrayValue(std::unique_ptr<IJSONValue> valueToAdd)
	{
		JSONValue* adapterValueToAdd = dynamic_cast<JSONValue*>(valueToAdd.get());
		if (!adapterValueToAdd)
		{
			throw std::runtime_error("JSONValue::addArrayValue() Provided value is not valid");
		}

		if (&adapterValueToAdd->m_document != &m_document)
		{
			throw std::runtime_error("JSONValue::addArrayValue() Provided value does not belong to this document");
		}

		std::unique_ptr<::rapidjson::Value> freeValue = m_document.removeFreeValue(adapterValueToAdd->m_value);
		m_value.PushBack(freeValue->Move(), m_allocator);

		unsigned int addedValueIndex = m_value.Size() - 1;
		::rapidjson::Value& addedValue = m_value[addedValueIndex];
		auto adapterValueAdded = std::make_unique<JSONValue>(m_document, addedValue, m_allocator);
		m_arrayValues.push_back(std::move(adapterValueAdded));
	}

	void JSONValue::clearArray()
	{
		m_value.GetArray().Clear();
		m_arrayValuesLoaded = true;
		m_arrayValues.clear();
	}

	std::unique_ptr<IJSONValue> JSONValue::buildValue(Type type) const
	{
		auto freeValue = std::make_unique<::rapidjson::Value>();
		auto newValue = std::make_unique<JSONValue>(m_document, *freeValue, m_allocator);
		newValue->setType(type);
		m_document.addFreeValue(std::move(freeValue));

		return newValue;
	}

	std::unique_ptr<IJSONDocument> JSONValue::buildDocument() const
	{
		auto newDocument = std::make_unique<::rapidjson::Document>();
		newDocument->CopyFrom(m_value, newDocument->GetAllocator());
		return std::make_unique<JSONDocument>(std::move(newDocument));
	}

	void JSONValue::loadObjectMembers() const
	{
		if (m_objectMembersLoaded)
		{
			return;
		}

		for (auto itr = m_value.MemberBegin(); itr != m_value.MemberEnd(); ++itr)
		{
			std::string memberName = itr->name.GetString();
			std::unique_ptr<JSONMember> member = std::make_unique<JSONMember>(m_document, memberName, itr->value, m_allocator);
			m_objectMembers.insert(std::make_pair(memberName, std::move(member)));
		}

		m_objectMembersLoaded = true;
	}

	void JSONValue::clearObjectMembers()
	{
		m_objectMembersLoaded = false;
		m_objectMembers.clear();
	}

	void JSONValue::loadArrayValues() const
	{
		if (m_arrayValuesLoaded)
		{
			return;
		}

		for (auto itr = m_value.Begin(); itr != m_value.End(); ++itr)
		{
			std::unique_ptr<IJSONValue> value = std::make_unique<JSONValue>(m_document, *itr, m_allocator);
			m_arrayValues.push_back(std::move(value));
		}

		m_arrayValuesLoaded = true;
	}

	void JSONValue::clearArrayValues()
	{
		m_arrayValuesLoaded = false;
		m_arrayValues.clear();
	}

	IJSONValue* JSONValue::getJSONPointerValue(const std::string& jsonPointer)
	{
		if (jsonPointer.empty())
		{
			return this;
		}

		const auto slashItr = jsonPointer.find("/");
		std::string firstFragment = jsonPointer.substr(0, slashItr);
		std::string nextFragments = (slashItr != std::string::npos) ? jsonPointer.substr(slashItr + 1) : "";

		systelab::json::Type type = getType();
		if (type == OBJECT_TYPE)
		{
			if (hasObjectMember(firstFragment))
			{
				return getObjectMemberValue(firstFragment).getJSONPointerValue(nextFragments);
			}
		}
		else if (type == ARRAY_TYPE)
		{
			if (firstFragment.find_first_not_of("0123456789") == std::string::npos)
			{
				unsigned int itemIndex = std::atoi(firstFragment.c_str());
				if (itemIndex < getArrayValueCount())
				{
					return getArrayValue(itemIndex).getJSONPointerValue(nextFragments);
				}
			}
		}

		return nullptr;
	}

	const IJSONValue* JSONValue::getJSONPointerValue(const std::string& jsonPointer) const
	{
		auto currentJSONValue = const_cast<IJSONValue*>(static_cast<const IJSONValue*>(this));
		return currentJSONValue->getJSONPointerValue(jsonPointer);
	}

}}}
