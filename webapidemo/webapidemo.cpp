
#include "stdafx.h"
#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace pplx;
using namespace utility;
using namespace streams;

using namespace web;
using namespace http;
using namespace client;
using namespace json;
typedef value JsonValue;
typedef JsonValue::value_type JsonValueType;
typedef wstring String;

String JsonValueTypeToString(const JsonValueType& type)
{
	switch (type)
	{
	case JsonValueType::Array: return L"Array";
	case JsonValueType::Boolean: return L"Boolean";
	case JsonValueType::Null: return L"Null";
	case JsonValueType::Number: return L"Number";
	case JsonValueType::Object: return L"Object";
	case JsonValueType::String: return L"String";
	default:return L"String";
	}
}

void Externalize(const JsonValue& json)
{
	for (auto iter = json.as_object().cbegin(); iter != json.as_object().cend(); ++iter)
	{
		const auto &key = iter->first;
		const auto &value = iter->second;

		wcout << key << L" : " << value.serialize() << " (" << JsonValueTypeToString(value.type()) << ")" << endl;
	}
}


task<void> PostJsonValueAsync()
{
	try
	{
		http_client client(L"http://101.201.28.235:87");
		http_request request(methods::POST);
		//request.headers().add(L"auth", L"eyJhbGciOiJIUzI1NiIsImV4cCI6MTM4NTY2OTY1NSwiaWF0IjoxMzg1NjY5MDU1fQ.eyJpZCI6MX0.XbOEFJkhjHJ5uRINh2JA1BPzXjSohK");
		request.headers().add(L"Content-Type", L"application/json");
		request.set_request_uri(L"api/login");
		JsonValue user;
		user[L"userName"] = JsonValue::string(L"admin");
		user[L"password"] = JsonValue::string(L"Abbott101");
		request.set_body(user);

		/*uri_builder builder;
		builder.append_path(L"api/login");*/
		//return client.request(methods::POST, builder.to_string(),user.serialize(), L"application/json").then([](http_response response) -> task<JsonValue>
		return client.request(request).then([](http_response response) -> task<JsonValue>
		{
			wostringstream stream;
			stream.str(wstring());
			stream << L"Content type: " << response.headers().content_type() << endl;
			stream << L"Content length: " << response.headers().content_length() << L"bytes" << endl;
			wcout << stream.str();

			// If the status is OK extract the body of the response into a JSON value
			if (response.status_code() == status_codes::OK)
			{
				//Note: response.extract_json()/response.extract_string() only can call once time
				//wcout << response.extract_string().get().c_str() << endl;
				return response.extract_json();
			}
			else
			{
				// return an empty JSON value
				return task_from_result(JsonValue());
			}
		})
			.then([](task<JsonValue> previousTask)
		{
			try
			{
				const auto jv = previousTask.get();
				Externalize(jv);
			}
			catch (const http_exception& e)
			{
				// Print error.  
				wostringstream ss;
				ss << e.what() << endl;
				wcout << ss.str();
			}
		});
	}
	catch (const exception& e)
	{
		wostringstream ss;
		ss << e.what() << endl;
		wcout << ss.str();
		return task_from_result();
	}
}
// Retrieves a JSON value from an HTTP request.  
task<void> GetJsonValueAsync()
{
	http_client client(L"http://101.201.28.235:87/api/values");
	return client.request(methods::GET).then([](http_response response) -> task<JsonValue>
	{
		wostringstream stream;
		stream.str(wstring());
		stream << L"Content type: " << response.headers().content_type() << endl;
		stream << L"Content length: " << response.headers().content_length() << L"bytes" << endl;
		wcout << stream.str();

		// If the status is OK extract the body of the response into a JSON value
		if (response.status_code() == status_codes::OK)
		{
			//wcout << response.extract_string().get().c_str() << endl;
			return response.extract_json();
		}
		else
		{
			// return an empty JSON value
			return task_from_result(JsonValue());
		}
	})
		.then([](task<JsonValue> previousTask)
	{
		try
		{
			const auto jv = previousTask.get();
			// Perform actions here to process the JSON value...  
			//Externalize(jv);
		}
		catch (const http_exception& e)
		{
			// Print error.  
			wostringstream ss;
			ss << e.what() << endl;
			wcout << ss.str();
		}
	});
}

int wmain()
{
	/*wcout << L"Calling GetJsonValueAsync..." << endl;
	GetJsonValueAsync().wait();*/

	wcout << L"Calling PostJsonValueAsync..." << endl;
	PostJsonValueAsync().wait();

	getchar();
}

