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

import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';
import image from '@ohos.multimedia.image';
import abilityAccessCtrl from '@ohos.abilityAccessCtrl';
import camera from '@ohos.multimedia.camera';
import  Logger from '../utils/Logger';
const logger =new Logger('aaa');

@Entry
@Component
struct Index {

   private dataSize: image.Size = {
                  height: 640,
                  width: 480
                };
  private receiver = image.createImageReceiver(this.dataSize, 2000, 8);
  private mXComponentController: XComponentController = new XComponentController;
  private XComponentSurfaceId: string = '';

  @State button0Txt: string = 'connect peer'
  @State button1Txt: string = 'connect server'
  @State button2Txt: string = 'show peer'

  build() {
    Column() {
      Row() {
        Column() {
          Button(this.button0Txt)
          .width(100)
          .height(100)
          .onClick(() => {
            if (testNapi.pcClientPeerIsConnect()) {
              testNapi.pcClientDisConnectPeer();
              this.button0Txt = "connect peer";
            } else if (testNapi.pcClientGetPeers() > 0) {
              abilityAccessCtrl.createAtManager()
                .requestPermissionsFromUser(getContext(), ['ohos.permission.CAMERA'])
                .then(async () => {
                  let receiverSurfaceId: string = await this.receiver.getReceivingSurfaceId();
                  this.XComponentSurfaceId = this.mXComponentController.getXComponentSurfaceId();
                  console.log("button click in");
                  testNapi.pcClientConnectPeer(receiverSurfaceId, this.XComponentSurfaceId);
                  console.log("add fun end");
                  this.button0Txt = "disconnect peer";
                  this.receiver.on('imageArrival', () => {
                    console.log("imageArrival start");
                    let img: image.Image = testNapi.createFromReceiver(this.receiver);
                    if (img) {
                      img.release();
                    }
                  });
                })
            }
          })
        }
        .layoutWeight(1)
        Column() {
          Button(this.button1Txt)
          .width(100)
          .height(100)
          .onClick(() => {
            hilog.warn(0x0000,'mytest','tetets');
            if (testNapi.pcClientServerIsConnect()) {
              testNapi.pcClientDisConnectServer();
              this.button1Txt = "connect server";
              logger.warn('tetets2');
              hilog.debug(0x0000,'mytest','tetets');
            } else {
              if (testNapi.pcClientInit("192.168.80.252", 8888) == 0) {
                testNapi.pcClientConnectServer();
                this.button1Txt = "disconnect server";
              }
            }
          })
        }
        .layoutWeight(1)
        Column() {
          Button(this.button2Txt)
          .width(100)
          .height(100)
          .onClick(() => {
            if (testNapi.pcClientGetPeers() > 0) {
              this.button2Txt = testNapi.pcClientGetPeerName(0);
              console.log("mytest peerClientGetPeers" + this.button2Txt);
            } else {
              this.button2Txt = "show peer";
              console.log("mytest peerClientGetPeers 0 ");
            }


          })
        }
        .layoutWeight(1)
      }
      .layoutWeight(100)
      Column() {
        XComponent({
          id: this.XComponentSurfaceId,
          type: 'surface',
          libraryname: '',
          controller: this.mXComponentController
        })
          .width(640)
          .height(480)
          .onLoad(() => {
            // 设置Surface宽高（1920*1080），预览尺寸设置参考前面 previewProfilesArray 获取的当前设备所支持的预览分辨率大小去设置
            // 预览流与录像输出流的分辨率的宽高比要保持一致
            this.mXComponentController.setXComponentSurfaceSize({ surfaceWidth: 640, surfaceHeight: 480 });
          })
      }
      .layoutWeight(640)
    }
    .height('100%')
  }
}