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

#include "client/ohos/main_wnd.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstdint>
#include <map>
#include <utility>

#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"
#include "api/video/video_source_interface.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "third_party/libyuv/include/libyuv/convert_from.h"
#include "hilog/log.h"
namespace {
//
// Simple static functions that simply forward the callback to the
// OhosMainWnd instance.
//
struct UIThreadCallbackData {
    explicit UIThreadCallbackData(MainWndCallback *cb, int id, void *d) : callback(cb), msg_id(id), data(d) {}
    MainWndCallback *callback;
    int msg_id;
    void *data;
};

bool HandleUIThreadCallback(void *data) {
    UIThreadCallbackData *cb_data = reinterpret_cast<UIThreadCallbackData *>(data);
    cb_data->callback->UIThreadCallback(cb_data->msg_id, cb_data->data);
    delete cb_data;
    return false;
}
} // namespace

OhosMainWnd::OhosMainWnd(const char *server, int port, bool autoconnect, bool autocall)
    : server_(server), port_(port), autoconnect_(autoconnect), autocall_(autocall) {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    char buffer[10];
    RTC_LOG(LS_INFO) << server << ":" << port_;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OhosMainWnd");
    window_ = false;
    ui_thread_ = 0;
    events_ = 0;
    callback_ = nullptr;
    mutex_ = PTHREAD_MUTEX_INITIALIZER;
}

OhosMainWnd::~OhosMainWnd() { RTC_DCHECK(!IsWindow()); }

void OhosMainWnd::RegisterObserver(MainWndCallback *callback) { callback_ = callback; }

bool OhosMainWnd::IsWindow() {
    //    RTC_LOG(LS_INFO) << __FUNCTION__;
    return window_;
}

void OhosMainWnd::Create(std::string s, int p) {
    RTC_DCHECK(window_ == false);
    RTC_LOG(LS_INFO) << __FUNCTION__;
    window_ = true;
    server_ = s;
    port_ = p;
}

void OhosMainWnd::Destroy() { window_ = false; }

void OhosMainWnd::PushEvent(int e) {
    pthread_mutex_lock(&mutex_);
    events_ = e;
    pthread_mutex_unlock(&mutex_);
}

int OhosMainWnd::PopupEvent(int e) {
    int event = 0;
    pthread_mutex_lock(&mutex_);
    event = events_;
    events_ = 0;
    pthread_mutex_unlock(&mutex_);
    return event;
}

int OhosMainWnd::GetEvent() {
    int event = 0;
    pthread_mutex_lock(&mutex_);
    event = events_;
    pthread_mutex_unlock(&mutex_);

    return event;
}
void OhosMainWnd::MessageBox(const char *caption, const char *text, bool is_error) {}
void OhosMainWnd::SetServerInfo(const char *server, int port) {
    server_ = server;
    port_ = port;
}

void OhosMainWnd::SwitchToPeerList(const Peers &peers) {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "SwitchToPeerList");
    //    peerList_ = peers;
    int j = 0;
    for (Peers::const_iterator i = peers.begin(); i != peers.end(); ++i, j++) {
        // peerList_.insert(std::make_pair(i->first, i->second));
        peerList_[j] = i->second;
    }
}

void OhosMainWnd::SwitchToConnectUI() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "SwitchToConnectUI");
}

void OhosMainWnd::SwitchToStreamingUI() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "SwitchToStreamingUI");
}

MainWindow::UI OhosMainWnd::current_ui() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "current_ui");
    return STREAMING;
}

void OhosMainWnd::StartLocalRenderer(webrtc::VideoTrackInterface *local_video) {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartLocalRenderer");
    local_renderer_.reset(new VideoRenderer(this, local_video));
}

void OhosMainWnd::StopLocalRenderer() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StopLocalRenderer");
    local_renderer_.reset();
}

void OhosMainWnd::StartRemoteRenderer(webrtc::VideoTrackInterface *remote_video) {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartRemoteRenderer");
    remote_renderer_.reset(new VideoRenderer(this, remote_video));
}

void OhosMainWnd::StopRemoteRenderer() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StopRemoteRenderer");
    remote_renderer_.reset();
}

void OhosMainWnd::QueueUIThreadCallback(int msg_id, void *data) {
    //  g_idle_add(HandleUIThreadCallback,
    //             new UIThreadCallbackData(callback_, msg_id, data));
    HandleUIThreadCallback(new UIThreadCallbackData(callback_, msg_id, data));
}

void OhosMainWnd::ConnectToServer() {
    if (callback_ != nullptr) {
        callback_->StartLogin(server_, port_);
    }
}

void OhosMainWnd::DisconnectFromServer() {
    if (callback_ != nullptr) {
        callback_->DisconnectFromServer();
    }
}

int OhosMainWnd::getCurrentPeer() {
    if (peerList_.begin() != peerList_.end())
        return 0;

    return -1;
}

int OhosMainWnd::GetPeers() { return peerList_.size(); }

std::string OhosMainWnd::GetPeerName(int peer_id) {

    Peers::const_iterator p = peerList_.find(peer_id);
    return p->second;
}

void OhosMainWnd::ConnectToPeer(int peer_id) {
    if (callback_ != nullptr && peer_id >= 0) {
        callback_->ConnectToPeer(peer_id);
    }
}

void OhosMainWnd::DisConnectFromPeer() {
    if (callback_ != nullptr) {
        callback_->DisconnectFromCurrentPeer();
    }
}

OhosMainWnd::VideoRenderer::VideoRenderer(OhosMainWnd *main_wnd, webrtc::VideoTrackInterface *track_to_render)
    : width_(0), height_(0), main_wnd_(main_wnd), rendered_track_(track_to_render) {
    rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
}

OhosMainWnd::VideoRenderer::~VideoRenderer() { rendered_track_->RemoveSink(this); }

void OhosMainWnd::VideoRenderer::SetSize(int width, int height) {
    if (width_ == width && height_ == height) {
        return;
    }
    width_ = width;
    height_ = height;
    image_.reset(new uint8_t[width * height * 4]);
}
#include "../samples/sample_bitmap.h"
void OhosMainWnd::VideoRenderer::OnFrame(const webrtc::VideoFrame &video_frame) {

    rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(video_frame.video_frame_buffer()->ToI420());
    if (video_frame.rotation() != webrtc::kVideoRotation_0) {
        buffer = webrtc::I420Buffer::Rotate(*buffer, video_frame.rotation());
    }
    SetSize(buffer->width(), buffer->height());
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "VideoRenderer::OnFrame width:%{public}d height:%{public}d",
                 buffer->width(), buffer->height());
    libyuv::I420ToABGR(buffer->DataY(), buffer->StrideY(), buffer->DataU(), buffer->StrideU(), buffer->DataV(),
                       buffer->StrideV(), image_.get(), width_ * 4, buffer->width(), buffer->height());

    SampleBitMap::NapiDrawPatternNative(nullptr, image_.get());
}
