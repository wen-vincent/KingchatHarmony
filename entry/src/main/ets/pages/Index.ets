import testNapi from 'libentry.so';
import image from '@ohos.multimedia.image';
import abilityAccessCtrl from '@ohos.abilityAccessCtrl';
import camera from '@ohos.multimedia.camera';
import Logger from '../utils/Logger'
import { RoomClient } from '../kingchat/roomClient';
import XComponentContext from "../interface/XComponentContext";

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
  private xComponentContext: XComponentContext | undefined = undefined;
  private XComponentSurfaceId: string = 'localVideo';
  private XComponentSurfaceId2: string = 'remoteVideo';

  build() {
    Column() {
      Row() {
        XComponent({
          id: this.XComponentSurfaceId,
          type: 'surface',
          libraryname: '',
          controller: this.mXComponentController
        })
          .width(240)
          .height(320)
          .onLoad(() => {
            // 设置Surface宽高（1920*1080），预览尺寸设置参考前面 previewProfilesArray 获取的当前设备所支持的预览分辨率大小去设置
            // 预览流与录像输出流的分辨率的宽高比要保持一致
            // this.mXComponentController.setXComponentSurfaceSize({ surfaceWidth: 48, surfaceHeight: 64 });
          })
      }
      .width('100%')
      .height('45%')
      .justifyContent(FlexAlign.Center)
      .shadow(ShadowStyle.OUTER_DEFAULT_SM)
      .layoutWeight(1)

      Row() {
        XComponent({ id: 'xcomponentId', type: 'surface', libraryname: 'entry' })
          .onLoad((xComponentContext) => {
            this.xComponentContext = xComponentContext as XComponentContext;
          })
          .width('640px') // Multiples of 64
          .height('480px')
      }
      .height('45%')
      .justifyContent(FlexAlign.Center)
      .shadow(ShadowStyle.OUTER_DEFAULT_SM)
      .layoutWeight(1)

      Row() {
        Button("connect")
          .width("33%")
          .height(40)
          .margin({ bottom: 24, right: 12 })
          .shadow(ShadowStyle.OUTER_DEFAULT_LG)
          .onClick(() => {
            abilityAccessCtrl.createAtManager()
              .requestPermissionsFromUser(getContext(), ['ohos.permission.CAMERA','ohos.permission.MICROPHONE'])
              .then(async () => {

                let receiverSurfaceId: string = await this.receiver.getReceivingSurfaceId();
                this.XComponentSurfaceId = this.mXComponentController.getXComponentSurfaceId();
                // testNapi.initCameraAndCreatTrack(receiverSurfaceId, this.XComponentSurfaceId);
                // testNapi.openCamera(receiverSurfaceId, this.XComponentSurfaceId);
                // testNapi.changeCamera();
                this.receiver.on('imageArrival', () => {
                  let img: image.Image = testNapi.createFromReceiver(this.receiver);
                  if (img) {
                    img.release();
                  }
                });

                const chatClient = new RoomClient({
                  roomId: '0099',
                  localStream: '',
                  displayName: '',
                  mode: '2',
                  protooUrl: 'wss://kingchat.szkingdom.vip/kingchat/',
                  videoContentHint: '',
                  forceAudioCodec: '',
                  audioContentHint: '',
                  canvas: '',
                  ctx: '',
                  canvasArea: ''
                });
                chatClient.joinRoom(receiverSurfaceId, this.XComponentSurfaceId);
                // initCameraAndCreatTrack
              })
          })

        Button("record")
          .width("33%")
          .height(40)
          .margin({ bottom: 24, right: 12 })
          .onClick(() => {
            logger.warn('mytest', 'tetets');
            testNapi.startThread('xcomponentId');
          })


      }
      .width('100%')
      .height('10%')
      .justifyContent(FlexAlign.Center)
      .shadow(ShadowStyle.OUTER_DEFAULT_SM)
      .layoutWeight(1)
    }
  }
}