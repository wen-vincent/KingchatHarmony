
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

#ifndef __PEER_CLIENT_H__
#define __PEER_CLIENT_H__

typedef enum {
    PEER_EVENT_NULL,
    PEER_EVENT_CONNECT_SERVER,
    PEER_EVENT_CONNECT_PEER,
    PEER_EVENT_DISCONNECT_SERVER,
    PEER_EVENT_DISCONNECT_PEER,
    PEER_EVENT_TYPE
} PEER_CLIENT_EVENT;

int PeerSampleClientStart(std::string server, int port);
void SetServerConnect(bool connected);
bool GetServerConnect();
void SetPeerConnect(bool connected);
bool GetPeerConnect();

int PeerSamplePostEvent(int event);

int PeerSampleGetPeers(void);

std::string PeerSampleGetPeerName(int peerID);

#endif  /* __PEER_CLIENT_H__ */
