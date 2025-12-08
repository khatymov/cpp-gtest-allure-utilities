#pragma once

#include "JSONAdapterInterface/IJSONAdapter.h"

#include <rapidjson/document.h>


namespace systelab { namespace json { namespace rapidjson {

	class JSONAdapter : public IJSONAdapter
	{
	public:
		JSONAdapter();
		virtual ~JSONAdapter();

		virtual std::unique_ptr<IJSONDocument> buildEmptyDocument() const;
		virtual std::unique_ptr<IJSONDocument> buildDocumentFromString(const std::string&) const;

		virtual std::unique_ptr<IJSONSchemaValidator> buildSchemaValidator(const IJSONDocument&) const;
		virtual std::unique_ptr<IJSONSchemaValidator> buildSchemaValidator(const IJSONDocument&, const IJSONRemoteSchemaProvider&) const;
	};

}}}
