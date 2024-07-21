import testNapi from 'libentry.so';
import image from '@ohos.multimedia.image';
import abilityAccessCtrl from '@ohos.abilityAccessCtrl';
import camera from '@ohos.multimedia.camera';
import Logger from '../utils/Logger'
import {RoomClient} from '../kingchat/roomClient';

//wss://inward.szkingdom.vip/kingchat/?roomId=0099&peerId=ulfp3xury07br9xz&videoMode=2

// const chatClient = new RoomClient({
//   roomId:'0099',
//   localStream:'',
//   displayName:'',
//   mode:'2',
//   protooUrl:'wss://kingchat.szkingdom.vip/kingchat/',
//   videoContentHint:'',
//   forceAudioCodec:'',
//   audioContentHint:'',
//   canvas:'',
//   ctx:'',
//   canvasArea:''
// });
// chatClient.joinRoom();

const logger = new Logger('index.ets');
@Entry
@Component
struct Index {

   private dataSize: image.Size = {
                  height: 480,
                  width: 640
                };
  private receiver = image.createImageReceiver(this.dataSize, 2000, 8);
  private mXComponentController: XComponentController = new XComponentController;
  private XComponentSurfaceId: string = '';

  @State button0Txt: string = 'connect peer'
  @State button1Txt: string = 'connect server'
  @State button2Txt: string = 'show peer'
  @State button4Txt: string = 'test'

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
          Button(this.button4Txt)
            .width(100)
            .height(100)
            .onClick(() => {
                abilityAccessCtrl.createAtManager()
                  .requestPermissionsFromUser(getContext(), ['ohos.permission.CAMERA'])
                  .then(async () => {

                    let receiverSurfaceId: string = await this.receiver.getReceivingSurfaceId();
                    this.XComponentSurfaceId = this.mXComponentController.getXComponentSurfaceId();
                    // testNapi.initCameraAndCreatTrack(receiverSurfaceId, this.XComponentSurfaceId);
                    // testNapi.openCamera(receiverSurfaceId, this.XComponentSurfaceId);
                    // testNapi.changeCamera();
                    this.receiver.on('imageArrival', () => {
                      console.log("imageArrival start");
                      let img: image.Image = testNapi.createFromReceiver(this.receiver);
                      if (img) {
                        img.release();
                      }
                    });

                    const chatClient = new RoomClient({
                      roomId:'0099',
                      localStream:'',
                      displayName:'',
                      mode:'2',
                      protooUrl:'wss://kingchat.szkingdom.vip/kingchat/',
                      videoContentHint:'',
                      forceAudioCodec:'',
                      audioContentHint:'',
                      canvas:'',
                      ctx:'',
                      canvasArea:''
                    });
                    chatClient.joinRoom(receiverSurfaceId, this.XComponentSurfaceId);
                    // initCameraAndCreatTrack

                  })
            })
        }
        .layoutWeight(1)
        Column() {
          Button(this.button1Txt)
          .width(100)
          .height(100)
          .onClick(() => {
            logger.warn('mytest','tetets');
            if (testNapi.pcClientServerIsConnect()) {
              testNapi.pcClientDisConnectServer();
              this.button1Txt = "connect server";
              logger.debug('mytest','tetets');
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
          .width(480)
          .height(640)
          .onLoad(() => {
            // 设置Surface宽高（1920*1080），预览尺寸设置参考前面 previewProfilesArray 获取的当前设备所支持的预览分辨率大小去设置
            // 预览流与录像输出流的分辨率的宽高比要保持一致
            this.mXComponentController.setXComponentSurfaceSize({ surfaceWidth: 480, surfaceHeight: 640 });
          })
      }
      .layoutWeight(480)
    }
    .height('100%')
  }
}