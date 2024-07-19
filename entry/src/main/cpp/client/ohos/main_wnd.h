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

#ifndef EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_

#include <stdint.h>
#include <pthread.h>
#include <memory>
#include <string>

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "client/main_wnd.h"
#include "client/peer_connection_client.h"


// Implements the main UI of the peer connection client.
// This is functionally equivalent to the MainWnd class in the Windows
// implementation.
class OhosMainWnd : public MainWindow {
public:
    OhosMainWnd(const char* server, int port, bool autoconnect, bool autocall);
    ~OhosMainWnd();

    virtual void RegisterObserver(MainWndCallback* callback);
    virtual bool IsWindow();
    virtual void SwitchToConnectUI();
    virtual void SwitchToPeerList(const Peers& peers);
    virtual void SwitchToStreamingUI();
    virtual void MessageBox(const char* caption, const char* text, bool is_error);
    virtual MainWindow::UI current_ui();
    virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
    virtual void StopLocalRenderer();
    virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
    virtual void StopRemoteRenderer();

    virtual void QueueUIThreadCallback(int msg_id, void* data);

    void Create(std::string s, int p);
    void Destroy();
    void ConnectToServer();
    void DisconnectFromServer();
    int GetPeers();
    std::string GetPeerName(int peer_id);
    int getCurrentPeer();
    void ConnectToPeer(int peer_id);
    void DisConnectFromPeer();
    void PushEvent(int event);
    int PopupEvent(int event);
    int GetEvent();
    void SetServerInfo(const char *server, int port);
protected:
    class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        VideoRenderer(OhosMainWnd* main_wnd,
                      webrtc::VideoTrackInterface* track_to_render);
        virtual ~VideoRenderer();

        // VideoSinkInterface implementation
        void OnFrame(const webrtc::VideoFrame& frame) override;

        const uint8_t* image() const { return image_.get(); }

        int width() const { return width_; }

        int height() const { return height_; }

    protected:
        void SetSize(int width, int height);
        std::unique_ptr<uint8_t[]> image_;
        int width_;
        int height_;
        OhosMainWnd* main_wnd_;
        rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
    };
protected:
    MainWndCallback* callback_;
    std::string server_;
    int port_;
    bool autoconnect_;
    bool autocall_;
    bool window_;
    std::unique_ptr<VideoRenderer> local_renderer_;
    std::unique_ptr<VideoRenderer> remote_renderer_;
    int width_;
    int height_;
    std::unique_ptr<uint8_t[]> draw_buffer_;
    int draw_buffer_size_;
    pthread_t ui_thread_;
    pthread_mutex_t mutex_;
    int events_;
    Peers peerList_;
};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_
