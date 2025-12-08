#pragma once

#include "JSONAdapterInterface/IJSONSchemaValidator.h"

#include <map>
#include <memory>
#include <rapidjson/schema.h>


namespace systelab { namespace json {
	class IJSONRemoteSchemaProvider;
}}

namespace systelab { namespace json { namespace rapidjson {

	class JSONSchemaValidator : public IJSONSchemaValidator
	{
	private:
		class RapidjsonSchemaRemoteDocumentProvider : public ::rapidjson::IRemoteSchemaDocumentProvider
		{
		public:
			RapidjsonSchemaRemoteDocumentProvider(const IJSONRemoteSchemaProvider&);

			virtual const ::rapidjson::SchemaDocument* GetRemoteDocument(const char* uri, ::rapidjson::SizeType length);

		private:
			const IJSONRemoteSchemaProvider& m_remoteSchemaProvider;
			std::map<std::string, std::unique_ptr<::rapidjson::SchemaDocument> > m_remoteSchemaMap;

			std::unique_ptr<::rapidjson::SchemaDocument> buildSchemaDocument(const std::string& uri);
		};

	public:
		JSONSchemaValidator(const IJSONDocument&);
		JSONSchemaValidator(const IJSONDocument&, const IJSONRemoteSchemaProvider&);
		virtual ~JSONSchemaValidator();

		bool validate(const IJSONDocument&, std::string& reason) const;

	private:
		std::unique_ptr<RapidjsonSchemaRemoteDocumentProvider> m_rapidjsonRemoteSchemaProvider;
		std::unique_ptr<::rapidjson::SchemaDocument> m_schemaDocument;
	};

}}}
