/*
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#include <stdio.h>
#include "hilog/log.h"
#include "absl/flags/parse.h"
#include "api/scoped_refptr.h"
#include "client/conductor.h"
#include "client/flag_defs.h"
#include "client/peer_connection_client.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "system_wrappers/include/field_trial.h"
#include "test/field_trial.h"
#include "client/main_wnd.h"
#include "client/ohos/main_wnd.h"
#include "client/ohos/peer_sample.h"

class CustomSocketServer : public rtc::PhysicalSocketServer {
public:
    explicit CustomSocketServer(OhosMainWnd *wnd)
        : wnd_(wnd), conductor_(NULL), client_(NULL) {}
    virtual ~CustomSocketServer() {}

    void SetMessageQueue(rtc::Thread* queue) override { message_queue_ = queue; }

    void set_client(PeerConnectionClient* client) { client_ = client; }
    void set_conductor(Conductor* conductor) { conductor_ = conductor; }

    bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
        while (1) {
          int event = wnd_->GetEvent();
          if (event) {
              if (event == PEER_EVENT_CONNECT_SERVER) {
                  wnd_->ConnectToServer();
                    SetServerConnect(true);
              } else if (event == PEER_EVENT_CONNECT_PEER) {
                  int peerId = 1;   // need get with real  
                  wnd_->ConnectToPeer(peerId);
                    SetPeerConnect(true);
              } else if (event == PEER_EVENT_DISCONNECT_SERVER) {
                    wnd_->DisconnectFromServer();
                    SetServerConnect(false);
              } else if (event == PEER_EVENT_DISCONNECT_PEER) {
                    wnd_->DisConnectFromPeer();
                    SetPeerConnect(false);
              }
              wnd_->PopupEvent(event);
          } else {
              break;
          }
      }
      if (!wnd_->IsWindow() && !conductor_->connection_active() &&
          client_ != NULL && !client_->is_connected()) {
//RTC_LOG(LS_INFO) << __FUNCTION__;
        message_queue_->Quit();
      }

      return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(),
                                             process_io);
    }

protected:
    rtc::Thread* message_queue_;
    OhosMainWnd* wnd_;
    Conductor* conductor_;
    PeerConnectionClient* client_;
};
#define SERVER_PORT 8888
static int gClientRunning = 0;
static OhosMainWnd *g_wnd = nullptr;
static bool gPeerConnected = false;
static bool gServerConnected = false;

//int main(int argc, char* argv[]) {
void *ClientThreadRun(void *params)
{
    RTC_LOG(LS_INFO) << "peerconnection client start test" << " " << "测试奥术大师";
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "peerconnection client start test");
    OhosMainWnd *wnd = (OhosMainWnd *)params;

    CustomSocketServer socket_server(wnd);
    rtc::AutoSocketServerThread thread(&socket_server);

    rtc::InitializeSSL();

    // Must be constructed after we set the socketserver.
    PeerConnectionClient client;
    auto conductor = rtc::make_ref_counted<Conductor>(&client, wnd);
    socket_server.set_client(&client);
    socket_server.set_conductor(conductor.get());
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "before thread run ##############");
    thread.Run();
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "after thread run");
    rtc::CleanupSSL();
    gClientRunning = 0;
    return nullptr;
}

int PeerSampleClientStart(std::string server, int port)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerSampleClientStart");
    if (gClientRunning) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerSampleClient has been Starting");
        return 1;
    }
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerSampleClientStart");
    gClientRunning = 1;
    pthread_t clientThread;
    const std::string forced_field_trials = absl::GetFlag(FLAGS_force_fieldtrials);
    webrtc::field_trial::InitFieldTrialsFromString(forced_field_trials.c_str());

    if ((absl::GetFlag(FLAGS_port) < 1) || (absl::GetFlag(FLAGS_port) > 65535)) {
      return -1;
    }
//    const std::string server = absl::GetFlag(FLAGS_server);
    if (server == "") {
        server = "192.168.80.252";
    }
    g_wnd = new OhosMainWnd("192.168.80.252", 8888, absl::GetFlag(FLAGS_autoconnect),
                    absl::GetFlag(FLAGS_autocall));
    g_wnd->Create(server, port);
    if (pthread_create(&clientThread, nullptr, ClientThreadRun, g_wnd) < 0) {
        gClientRunning = 0;
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "pthread_create failed");
        return -1;
    }

    pthread_detach(clientThread);
    sleep(2);

    return 0;
}

void SetServerConnect(bool connected)
{
    gServerConnected = connected;
}
bool GetServerConnect()
{
    return  gServerConnected;   
}

void SetPeerConnect(bool connected)
{
    gPeerConnected = connected;
}
bool GetPeerConnect()
{
    return  gPeerConnected;   
}

int PeerSamplePostEvent(int event)
{
    if (g_wnd == nullptr) {
        return -1;
    }
    g_wnd->PushEvent(event);

    return 0;
}

int PeerSampleGetPeers(void)
{
    if (g_wnd == nullptr) {
        return -1;
    }
    return g_wnd->GetPeers();
}

std::string PeerSampleGetPeerName(int peerID)
{
    if (g_wnd == nullptr) {
        return "";
    }
    return g_wnd->GetPeerName(peerID);
}

void PeerSampleSetServer(const char *s, int p)
{
    if (g_wnd == nullptr) {
        return;
    }
    g_wnd->SetServerInfo(s, p);
}
