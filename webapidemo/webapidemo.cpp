
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

task<utility::string_t> LoginAsync(wchar_t* url, wchar_t* userName, wchar_t* password)
{
	try
	{
		//http_client client(L"http://101.201.28.235:87");
		http_client_config config;
		config.set_timeout(utility::seconds(90)); //90s
		http_client client(url, config);
		http_request request(methods::POST);
		//request.headers().add(L"auth", L"eyJhbGciOiJIUzI1NiIsImV4cCI6MTM4NTY2OTY1NSwiaWF0IjoxMzg1NjY5MDU1fQ.eyJpZCI6MX0.XbOEFJkhjHJ5uRINh2JA1BPzXjSohK");
		request.headers().add(L"Content-Type", L"application/json");
		request.set_request_uri(L"api/login");
		JsonValue user;
		user[L"userName"] = JsonValue::string(userName);
		user[L"password"] = JsonValue::string(password);
		request.set_body(user);

		return client.request(request).then([](http_response response) -> task<JsonValue>
		{
			if (response.status_code() == status_codes::OK)
			{
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
				auto auth = jv.as_object().at(L"auth").as_string();
				return auth;
			}
			catch (const http_exception& e)
			{
				// Print error.  
				wostringstream ss;
				ss << e.what() << endl;
				wcout << ss.str();
			}
			catch (const std::exception& e)
			{
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
		return task_from_result(utility::string_t(L""));
	}
}

task<utility::string_t> ClearCalResultAsync(const wchar_t* url, const wchar_t* calName, const wchar_t* nowSerialNumber, const wchar_t*  auth)
{
	try
	{
		http_client_config config;
		config.set_timeout(utility::seconds(90)); //90s
		http_client client(url, config);
		http_request request(methods::POST);
		request.headers().add(L"AccessToken", auth);
		request.headers().add(L"Content-Type", L"application/json");
		request.set_request_uri(L"api/PressureClear");
		JsonValue user;
		user[L"CalName"] = JsonValue::string(calName);
		user[L"NowSerialNumber"] = JsonValue::string(nowSerialNumber);
		request.set_body(user);

		return client.request(request).then([](http_response response) -> task<utility::string_t>
		{
			if (response.status_code() == status_codes::OK)
			{
				return response.extract_string();
			}
			else
			{
				return task_from_result(utility::string_t(L""));
			}
		})
			.then([](task<utility::string_t> previousTask)
		{
			try
			{
				const auto result = previousTask.get();
				return result;
			}
			catch (const http_exception& e)
			{
				// Print error.  
				wostringstream ss;
				ss << e.what() << endl;
				wcout << ss.str();
			}
			catch (const std::exception& e)
			{
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
		return task_from_result(utility::string_t(L""));
	}
}


task<JsonValue> AddPressureDataAsync(const wchar_t* url, const wchar_t*  auth)
{
	try
	{
		http_client_config config;
		config.set_timeout(utility::seconds(90)); //90s
		http_client client(url, config);
		http_request request(methods::POST);
		request.headers().add(L"AccessToken", auth);
		request.headers().add(L"Content-Type", L"application/json");
		request.set_request_uri(L"api/PressureDataPost");

		//SPressureData
		JsonValue pdata;
		pdata[L"AnzSerNumber"] = JsonValue::number(4352343);
		pdata[L"BdSerialNumberID"] = JsonValue::string(L"ds0000");
		pdata[L"BdSerialNumberNo"] = JsonValue::number(232323);
		pdata[L"MfgEepromCrc"] = JsonValue::string(L"sfsdsd");
		pdata[L"MfgSWRev"] = JsonValue::string(L"15");
		pdata[L"PressureSensorType"] = JsonValue::string(L"sd");
		pdata[L"ComputerName"] = JsonValue::string(L"test");
		JsonValue jv;
		jv[L"SPressureData"] = JsonValue::string(pdata.serialize());

		JsonValue pbdata;
		//PresOff1
		JsonValue presOff1 = JsonValue::object();
		presOff1[L"PresOff1(mmHg)"] = JsonValue::string(L"");
		presOff1[L"PresOff1MinLimit(mmHg)"] = JsonValue::string(L"");
		presOff1[L"PresOff1MaxLimit(mmHg)"] = JsonValue::string(L"");
		presOff1[L"PresOff1Type"] = JsonValue::string(L"");
		presOff1[L"PresOff1EEPROMName"] = JsonValue::string(L"");
		presOff1[L"PresOff1EEPROMValue"] = JsonValue::string(L"");
		pbdata[L"PresOff1(mmHg)"] = presOff1;
		//PresOff2
		JsonValue presOff2 = JsonValue::object();
		presOff2[L"PresOff2(mmHg)"] = JsonValue::string(L"");
		presOff2[L"PresOff2MinLimit(mmHg)"] = JsonValue::string(L"");
		presOff2[L"PresOff2MaxLimit(mmHg)"] = JsonValue::string(L"");
		presOff2[L"PresOff2Type"] = JsonValue::string(L"");
		presOff2[L"PresOff2EEPROMName"] = JsonValue::string(L"");
		presOff2[L"PresOff2EEPROMValue"] = JsonValue::string(L"");
		pbdata[L"PresOff2(mmHg)"] = presOff2;
		//PresGain1
		JsonValue presGain1 = JsonValue::object();
		presGain1[L"PresGain1"] = JsonValue::string(L"");
		presGain1[L"PresGain1MinLimit"] = JsonValue::string(L"");
		presGain1[L"PresGain1MaxLimit"] = JsonValue::string(L"");
		presGain1[L"PresGain1Type"] = JsonValue::string(L"");
		presGain1[L"PresGain1EEPROMName"] = JsonValue::string(L"");
		presGain1[L"PresGain1EEPROMValue"] = JsonValue::string(L"");
		pbdata[L"PresGain1"] = presGain1;
		//PresGain2
		JsonValue presGain2 = JsonValue::object();
		presGain2[L"PresGain2"] = JsonValue::string(L"");
		presGain2[L"PresGain2MinLimit"] = JsonValue::string(L"");
		presGain2[L"PresGain2MaxLimit"] = JsonValue::string(L"");
		presGain2[L"PresGain2Type"] = JsonValue::string(L"");
		presGain2[L"PresGain2EEPROMName"] = JsonValue::string(L"");
		presGain2[L"PresGain2EEPROMValue"] = JsonValue::string(L"");
		pbdata[L"PresGain2"] = presGain2;
		//PresTransA1
		JsonValue presTransA1 = JsonValue::object();
		presTransA1[L"PresTransA1"] = JsonValue::string(L"");
		presTransA1[L"PresTransA1MinLimit"] = JsonValue::string(L"");
		presTransA1[L"PresTransA1MaxLimit"] = JsonValue::string(L"");
		presTransA1[L"PresTransA1Type"] = JsonValue::string(L"");
		presTransA1[L"PresTransA1EEPROMName"] = JsonValue::string(L"");
		presTransA1[L"PresTransA1EEPROMValue"] = JsonValue::string(L"");
		pbdata[L"PresTransA1"] = presTransA1;
		//PresTransA2
		JsonValue presTransA2 = JsonValue::object();
		presTransA2[L"PresTransA2"] = JsonValue::string(L"");
		presTransA2[L"PresTransA2MinLimit"] = JsonValue::string(L"");
		presTransA2[L"PresTransA2MaxLimit"] = JsonValue::string(L"");
		presTransA2[L"PresTransA1Type"] = JsonValue::string(L"");
		presTransA2[L"PresTransA1EEPROMName"] = JsonValue::string(L"");
		presTransA2[L"PresTransA1EEPROMValue"] = JsonValue::string(L"");
		pbdata[L"PresTransA2"] = presTransA2;
		//add some other fields
		jv[L"SPressureBoschData"] = JsonValue::string(pbdata.serialize());
		request.set_body(jv);

		return client.request(request).then([](http_response response) -> task<JsonValue>
		{
			if (response.status_code() == status_codes::OK)
			{
				return response.extract_json();
			}
			else
			{
				return task_from_result(JsonValue());
			}
		})
			.then([](task<JsonValue > previousTask)
		{
			try
			{
				const auto result = previousTask.get();
				//auto message = result.as_object().at(L"message").as_string();
				return result;
			}
			catch (const http_exception& e)
			{
				// Print error.  
				wostringstream ss;
				ss << e.what() << endl;
				wcout << ss.str();
			}
			catch (const std::exception& e)
			{
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
		return task_from_result(JsonValue());
	}
}



int wmain()
{

	try
	{
		/*wcout << L"Calling GetJsonValueAsync..." << endl;
		GetJsonValueAsync().wait();*/
		/*wcout << L"Calling PostJsonValueAsync..." << endl;
		PostJsonValueAsync().wait();*/

		wcout << L"Calling LoginAsync..." << endl;
		LoginAsync(L"http://192.168.0.103:5000", L"admin", L"Abbott101").then([](task<utility::string_t> auth)
		{
			//login first and get the auth code
			auto au = auth.get();
			wcout << au << endl;

			// call ClearCalResult method
			ClearCalResultAsync(L"http://192.168.0.103:5000", L"Pressure Calibration", L"SN1010", au.c_str()).then([](task<utility::string_t>  result)
			{
				auto rst = result.get();
				wcout << rst << endl;
			});

			// call add presure data method

			AddPressureDataAsync(L"http://192.168.0.103:5000", au.c_str()).then([](task<JsonValue> result)
			{
				auto rst = result.get();
				auto resultid = rst.as_object().at(L"resultId").as_integer();
				auto message = rst.as_object().at(L"message").as_string();
				wcout << L"resultId:" + std::to_wstring(resultid) + L",message:" + message << endl;
			});
		});


		getchar();
	}
	catch (const std::exception& e)
	{
		wostringstream ss;
		ss << e.what() << endl;
		wcout << ss.str();
	}
}

