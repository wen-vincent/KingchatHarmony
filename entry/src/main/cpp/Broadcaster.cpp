#include "Broadcaster.h"
// #include "MediaStreamTrackFactory.h"
#include "mediasoupclient.hpp"
#include "json.hpp"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include "MediaStreamTrackFactory.h"
#include "hilog/log.h"
#include "utils/utilCallJs.h"

using json = nlohmann::json;

Broadcaster::~Broadcaster()
{
	this->Stop();
}

void Broadcaster::OnTransportClose(mediasoupclient::Producer* /*producer*/)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnTransportClose()\n");
}

void Broadcaster::OnTransportClose(mediasoupclient::DataProducer* /*dataProducer*/)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnTransportClose()\n");
}

/* Transport::Listener::OnConnect
 *
 * Fired for the first Transport::Consume() or Transport::Produce().
 * Update the already created remote transport with the local DTLS parameters.
 */
std::future<void> Broadcaster::OnConnect(mediasoupclient::Transport* transport, const json& dtlsParameters)
{
	// std::cout << "[INFO] dtlsParameters: " << dtlsParameters.dump(4) << std::endl;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnConnect()\n");

    if (transport->GetId() == this->sendTransport->GetId())
	{
		return this->OnConnectSendTransport(dtlsParameters);
	}
	else if (transport->GetId() == this->recvTransport->GetId())
	{
		return this->OnConnectRecvTransport(dtlsParameters);
	}
	else
	{
		std::promise<void> promise;

		promise.set_exception(std::make_exception_ptr("Unknown transport requested to connect"));

		return promise.get_future();
	}
}

std::future<void> Broadcaster::OnConnectSendTransport(const json& dtlsParameters)
{
	std::promise<void> promise;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnConnectSendTransport()\n");
    /* clang-format off */
// 	json body =
// 	{
// 		{ "dtlsParameters", dtlsParameters }
// 	};
// 	/* clang-format on */
//
// 	auto r = cpr::PostAsync(
// 	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
// 	                     this->sendTransport->GetId() + "/connect" },
// 	           cpr::Body{ body.dump() },
// 	           cpr::Header{ { "Content-Type", "application/json" } },
// 	           cpr::VerifySsl{ verifySsl })
// 	           .get();
//
// 	if (r.status_code == 200)
// 	{
// 		promise.set_value();
// 	}
// 	else
// 	{
// 		std::cerr << "[ERROR] unable to connect transport"
// 		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
//
// 		promise.set_exception(std::make_exception_ptr(r.text));
// 	}
    promise.set_value();
    return promise.get_future();
}

std::future<void> Broadcaster::OnConnectRecvTransport(const json& dtlsParameters)
{
	std::promise<void> promise;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnConnectRecvTransport()\n");
	/* clang-format off */
// 	json body =
// 	{
// 		{ "dtlsParameters", dtlsParameters }
// 	};
// 	/* clang-format on */
//
// 	auto r = cpr::PostAsync(
// 	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
// 	                     this->recvTransport->GetId() + "/connect" },
// 	           cpr::Body{ body.dump() },
// 	           cpr::Header{ { "Content-Type", "application/json" } },
// 	           cpr::VerifySsl{ verifySsl })
// 	           .get();
//
// 	if (r.status_code == 200)
// 	{
// 		promise.set_value();
// 	}
// 	else
// 	{
// 		std::cerr << "[ERROR] unable to connect transport"
// 		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
//
// 		promise.set_exception(std::make_exception_ptr(r.text));
// 	}
promise.set_value();
	return promise.get_future();
}

/*
 * Transport::Listener::OnConnectionStateChange.
 */
void Broadcaster::OnConnectionStateChange(
  mediasoupclient::Transport* /*transport*/, const std::string& connectionState)
{
	OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnConnectionStateChange()\n");

	if (connectionState == "failed")
	{
		Stop();
		std::exit(0);
	}
}

/* Producer::Listener::OnProduce
 *
 * Fired when a producer needs to be created in mediasoup.
 * Retrieve the remote producer ID and feed the caller with it.
 */
std::future<std::string> Broadcaster::OnProduce(
  mediasoupclient::SendTransport* /*transport*/,
  const std::string& kind,
  json rtpParameters,
  const json& /*appData*/)
{

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnProduce() %{public}u\n",std::this_thread::get_id());
	std::promise<std::string> promise;

	// call js 
    napi_env env;
    std::string parm = rtpParameters.dump();
    std::future<std::string> fu = getProduceId->executeJs( env, true, parm);
//     std::string id = fu.get();
//     promise.set_value(id);
//    
    promise.set_value("1234567890video");
	return promise.get_future();
    
//     return getProduceId->executeJs( env, true);
}

/* Producer::Listener::OnProduceData
 *
 * Fired when a data producer needs to be created in mediasoup.
 * Retrieve the remote producer ID and feed the caller with it.
 */
std::future<std::string> Broadcaster::OnProduceData(
  mediasoupclient::SendTransport* /*transport*/,
  const json& sctpStreamParameters,
  const std::string& label,
  const std::string& protocol,
  const json& /*appData*/)
{
	OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnProduceData()\n");

	std::promise<std::string> promise;

	/* clang-format off */
// 	json body =
//     {
//         { "label"                , label },
//         { "protocol"             , protocol },
//         { "sctpStreamParameters" , sctpStreamParameters }
// 		// { "appData"				 , "someAppData" }
// 	};
// 	/* clang-format on */
//
// 	auto r = cpr::PostAsync(
// 	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
// 	                     this->sendTransport->GetId() + "/produce/data" },
// 	           cpr::Body{ body.dump() },
// 	           cpr::Header{ { "Content-Type", "application/json" } },
// 	           cpr::VerifySsl{ verifySsl })
// 	           .get();
//
// 	if (r.status_code == 200)
// 	{
// 		auto response = json::parse(r.text);
//
// 		auto it = response.find("id");
// 		if (it == response.end() || !it->is_string())
// 		{
// 			promise.set_exception(std::make_exception_ptr("'id' missing in response"));
// 		}
// 		else
// 		{
// 			auto dataProducerId = (*it).get<std::string>();
// 			promise.set_value(dataProducerId);
// 		}
// 	}
// 	else
// 	{
// 		std::cerr << "[ERROR] unable to create data producer"
// 		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
//
// 		promise.set_exception(std::make_exception_ptr(r.text));
// 	}
    promise.set_value("testOnProduceData");
	return promise.get_future();
}

const nlohmann::json& Broadcaster::Start(
  bool enableAudio,
  bool useSimulcast,
  const json& routerRtpCapabilities,
  bool verifySsl)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::Start() %{public}s\n",routerRtpCapabilities.dump().c_str());
	this->verifySsl = verifySsl;

	// Load the device.
	this->device.Load(routerRtpCapabilities);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "device.Load\n");

	/* clang-format off */
// 	json body =
// 	{
// 		{ "id",          this->id          },
// 		{ "displayName", "broadcaster"     },
// 		{ "device",
// 			{
// 				{ "name",    "libmediasoupclient"       },
// 				{ "version", mediasoupclient::Version() }
// 			}
// 		},
// 		{ "rtpCapabilities", this->device.GetRtpCapabilities() }
// 	};
// 	/* clang-format on */
//
// 	auto r = cpr::PostAsync(
// 	           cpr::Url{ this->baseUrl + "/broadcasters" },
// 	           cpr::Body{ body.dump() },
// 	           cpr::Header{ { "Content-Type", "application/json" } },
// 	           cpr::VerifySsl{ verifySsl })
// 	           .get();
//
// 	if (r.status_code != 200)
// 	{
// 		std::cerr << "[ERROR] unable to create Broadcaster"
// 		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
//
// 		return;
// 	}
    
    return this->device.GetRtpCapabilities();
}

int Broadcaster::CreateTransport(const nlohmann::json &transportInfo) {
    this->CreateSendTransport(true, false,transportInfo);
//     this->CreateRecvTransport(transportInfo);    
    return 0;
}

void Broadcaster::CreateDataConsumer()
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::CreateDataConsumer()\n");
	const std::string& dataProducerId = this->dataProducer->GetId();

	/* clang-format off */
	json body =
	{
		{ "dataProducerId", dataProducerId }
	};
	/* clang-format on */
	// create server data consumer
// 	auto r = cpr::PostAsync(
// 	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports/" +
// 	                     this->recvTransport->GetId() + "/consume/data" },
// 	           cpr::Body{ body.dump() },
// 	           cpr::Header{ { "Content-Type", "application/json" } },
// 	           cpr::VerifySsl{ verifySsl })
// 	           .get();
// 	if (r.status_code != 200)
// 	{
// 		std::cerr << "[ERROR] server unable to consume mediasoup recv WebRtcTransport"
// 		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
// 		return;
// 	}
//
// 	auto response = json::parse(r.text);
// 	if (response.find("id") == response.end())
// 	{
// 		std::cerr << "[ERROR] 'id' missing in response" << std::endl;
// 		return;
// 	}
// 	auto dataConsumerId = response["id"].get<std::string>();
//
// 	if (response.find("streamId") == response.end())
// 	{
// 		std::cerr << "[ERROR] 'streamId' missing in response" << std::endl;
// 		return;
// 	}
// 	auto streamId = response["streamId"].get<uint16_t>();
//
// 	// Create client consumer.
// 	this->dataConsumer = this->recvTransport->ConsumeData(
// 	  this, dataConsumerId, dataProducerId, streamId, "chat", "", nlohmann::json());
}

void Broadcaster::CreateSendTransport(bool enableAudio, bool useSimulcast,const nlohmann::json& transportInfo)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] CreateSendTransport %{public}s\n",
                 transportInfo.dump().c_str());

//     json sctpCapabilities = this->device.GetSctpCapabilities();
	OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] 11111111111\n");
	auto response = transportInfo;
    
    if (response.find("id") == response.end())
	{
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "mytest", "ERROR] 'id' missing in response\n");
        return;
	}
	else if (response.find("iceParameters") == response.end())
	{
		std::cerr << "[ERROR] 'iceParametersd' missing in response" << std::endl;
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "mytest", "[ERROR] 'iceParametersd' missing in response\n");
        return;
	}
	else if (response.find("iceCandidates") == response.end())
	{
		std::cerr << "[ERROR] 'iceCandidates' missing in response" << std::endl;
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "mytest", "[ERROR] 'iceCandidates' missing in response\n");
        return;
	}
	else if (response.find("dtlsParameters") == response.end())
	{
		std::cerr << "[ERROR] 'dtlsParameters' missing in response" << std::endl;
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "mytest", "[ERROR] 'dtlsParameters' missing in response\n");
        return;
	}
	else if (response.find("sctpParameters") == response.end())
	{
		std::cerr << "[ERROR] 'sctpParameters' missing in response" << std::endl;
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "mytest", "[ERROR] 'sctpParameters' missing in response\n");
        return;
	}

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] 22222222\n");

    auto sendTransportId = response["id"].get<std::string>();
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] 433333333333333333\n");
    this->sendTransport = this->device.CreateSendTransport(
	  this,
	  sendTransportId,
	  response["iceParameters"],
	  response["iceCandidates"],
	  response["dtlsParameters"],
	  response["sctpParameters"]);

	///////////////////////// Create Audio Producer //////////////////////////
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] 4444444444444444444444\n");
    if (enableAudio && this->device.CanProduce("audio"))
	{
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] 开始推流 audio\n");
        auto audioTrack = createAudioTrack(std::to_string(rtc::CreateRandomId()));

        /* clang-format off */
        json codecOptions = {
            { "opusStereo", true },
            { "opusDtx",		true }
        };
        /* clang-format on */

        this->sendTransport->Produce(this, audioTrack.get(), nullptr, &codecOptions, nullptr);
	}
	else
	{
		std::cerr << "[WARN] cannot produce audio" << std::endl;
	}

	///////////////////////// Create Video Producer //////////////////////////

	if (this->device.CanProduce("video"))
	{
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] 开始推流 video\n");
        		auto videoTrack = createSquaresVideoTrack(std::to_string(rtc::CreateRandomId()));

        		if (useSimulcast)
        		{
        			std::vector<webrtc::RtpEncodingParameters> encodings;
        			encodings.emplace_back(webrtc::RtpEncodingParameters());
        			encodings.emplace_back(webrtc::RtpEncodingParameters());
        			encodings.emplace_back(webrtc::RtpEncodingParameters());

        			this->sendTransport->Produce(this, videoTrack.get() ,&encodings, nullptr, nullptr);
        		}
        		else
        		{
        			this->sendTransport->Produce(this, videoTrack.get(), nullptr, nullptr, nullptr);
        		}
	}
	else
	{
		std::cerr << "[WARN] cannot produce video" << std::endl;

		return;
	}

	///////////////////////// Create Data Producer //////////////////////////

// 	this->dataProducer = sendTransport->ProduceData(this);
//
// 	uint32_t intervalSeconds = 10;
// 	std::thread([this, intervalSeconds]() {
// 		bool run = true;
// 		while (run)
// 		{
// 			std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
// 			std::time_t t                           = std::chrono::system_clock::to_time_t(p);
// 			std::string s                           = std::ctime(&t);
// 			auto dataBuffer                         = webrtc::DataBuffer(s);
// 			std::cout << "[INFO] sending chat data: " << s << std::endl;
// 			this->dataProducer->Send(dataBuffer);
// 			run = timerKiller.WaitFor(std::chrono::seconds(intervalSeconds));
// 		}
// 	})
// 	  .detach();

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::CreateSendTransport over--!!!------\n");
}

void Broadcaster::CreateRecvTransport(const nlohmann::json& transportInfo)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::CreateRecvTransport -----------\n");
//     json sctpCapabilities = this->device.GetSctpCapabilities();
	/* clang-format off */
// 	json body =
// 	{
// 		{ "type",    "webrtc" },
// 		{ "rtcpMux", true     },
// 		{ "sctpCapabilities", sctpCapabilities }
// 	};
// 	/* clang-format on */
//
// 	// create server transport
// 	auto r = cpr::PostAsync(
// 	           cpr::Url{ this->baseUrl + "/broadcasters/" + this->id + "/transports" },
// 	           cpr::Body{ body.dump() },
// 	           cpr::Header{ { "Content-Type", "application/json" } },
// 	           cpr::VerifySsl{ verifySsl })
// 	           .get();
//
// 	if (r.status_code != 200)
// 	{
// 		std::cerr << "[ERROR] unable to create mediasoup recv WebRtcTransport"
// 		          << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
//
// 		return;
// 	}
//
// 	auto response = json::parse(r.text);
//
// 	if (response.find("id") == response.end())
// 	{
// 		std::cerr << "[ERROR] 'id' missing in response" << std::endl;
//
// 		return;
// 	}
// 	else if (response.find("iceParameters") == response.end())
// 	{
// 		std::cerr << "[ERROR] 'iceParameters' missing in response" << std::endl;
//
// 		return;
// 	}
// 	else if (response.find("iceCandidates") == response.end())
// 	{
// 		std::cerr << "[ERROR] 'iceCandidates' missing in response" << std::endl;
//
// 		return;
// 	}
// 	else if (response.find("dtlsParameters") == response.end())
// 	{
// 		std::cerr << "[ERROR] 'dtlsParameters' missing in response" << std::endl;
//
// 		return;
// 	}
// 	else if (response.find("sctpParameters") == response.end())
// 	{
// 		std::cerr << "[ERROR] 'sctpParameters' missing in response" << std::endl;
//
// 		return;
// 	}
//
// 	auto recvTransportId = response["id"].get<std::string>();
//
// 	std::cout << "[INFO] creating RecvTransport..." << std::endl;
//
// 	auto sctpParameters = response["sctpParameters"];
//
// 	this->recvTransport = this->device.CreateRecvTransport(
// 	  this,
// 	  recvTransportId,
// 	  response["iceParameters"],
// 	  response["iceCandidates"],
// 	  response["dtlsParameters"],
// 	  sctpParameters);
//
// 	this->CreateDataConsumer();
}

void Broadcaster::OnMessage(mediasoupclient::DataConsumer* dataConsumer, const webrtc::DataBuffer& buffer)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnMessage()\n");
	if (dataConsumer->GetLabel() == "chat")
	{
		std::string s = std::string(buffer.data.data<char>(), buffer.data.size());
		std::cout << "[INFO] received chat data: " + s << std::endl;
	}
}

void Broadcaster::Stop()
{
	OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::Stop()\n");

	this->timerKiller.Kill();

	if (this->recvTransport)
	{
		recvTransport->Close();
	}

	if (this->sendTransport)
	{
		sendTransport->Close();
	}

// 	cpr::DeleteAsync(
// 	  cpr::Url{ this->baseUrl + "/broadcasters/" + this->id }, cpr::VerifySsl{ verifySsl })
// 	  .get();
}

void Broadcaster::OnOpen(mediasoupclient::DataProducer* /*dataProducer*/)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnOpen()\n");
}
void Broadcaster::OnClose(mediasoupclient::DataProducer* /*dataProducer*/)
{
	OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnClose()\n");
}
void Broadcaster::OnBufferedAmountChange(mediasoupclient::DataProducer* /*dataProducer*/, uint64_t /*size*/)
{
	OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "[INFO] Broadcaster::OnBufferedAmountChange()\n");
}
