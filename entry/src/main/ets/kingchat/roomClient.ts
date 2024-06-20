import Logger from '../utils/Logger'
import {generateRandomString,generateRandomNumber, deviceInfo } from '../utils/Utils'
import WebSocketTransport from '../protooClient/transports/WebSocketTransport'
import Peer from '../protooClient/Peer'
import { EventEmitter } from '../polyfill/events';
import { PC_PROPRIETARY_CONSTRAINTS, TRANSPORT_DIRECTION, VIDEO_MODE, MEDIA_SEND_STATE, MEDIA_IDENTITY } from '../utils/define';
import testNapi from 'libentry.so';

const  logger = new Logger('RoomClient');
const uri = 'wss://inward.szkingdom.vip/kingchat/?roomId=zvu77sru&peerId=afuztkimrcojm6xy&videoMode=1';
// const ws = new WebSocketTransport(uri,'protoo');

// const peer = new Peer(ws);
// peer.on('open',async ()=>{
//   logger.warn("peer open!");
//   const routerRtpCapabilities = await peer.request('getRouterRtpCapabilities');
//   logger.warn(JSON.stringify(routerRtpCapabilities));
// });
interface RoomInfo{
  roomId,
  localStream,
  displayName,
  mode,
  protooUrl,
  videoContentHint,
  forceAudioCodec,
  audioContentHint,
  canvas,
  ctx,
  canvasArea
}

export class RoomClient extends EventEmitter {
  private _device;
  private _peerId;
  private _closed;
  private _roomId:String = '';
  private _displayName;
  private _videoMode;
  private _produced;
  private _consume;
  private _useDataChannel;
  private _canUseDataChannel;
  private _videoContentHint;
  private _audioContentHint;
  private _forceAudioCodec;
  private canvas;
  private ctx;
  private drawdown;
  private startPointX;
  private startPointY;
  private toolcontrol;
  private convasData;
  private drawcolor;
  private curtool;
  private textvalue;
  private canvasArea;
  private shareDesktopWidth;
  private shareDesktopHeight;
  private widthRatio;
  private heeightRatio;
  private _produce;
  private _fileServiceUrl;
  private _protooUrl;
  private _protoo;
  private _mediasoupDevice;
  private _sendTransport;
  private _recvTransport;
  private _micProducer;
  private _webcamProducer;
  private _recordVideoProducer;
  private _recordAudioProducer;
  private _shareProducer;
  private _chatDataProducer;
  private _consumers;
  private _dataConsumers;
  private _localStream;
  private _remoteStream;
  private _shareDesktopStream;
  private _recordMixer;
  private _protooTransport;
  private _videoCodecOptions;
  private _getBitrateInterval;
  private _trackMp3;
  private _lastBytesRecv;
  private _lastBytesSent;
  private _lastRecordBytesSent;
  private _lastPackets;
  private _recordedChunks;
  private _mediaRecorder;
  private _stopRecord;
  private _recordMixedStream;
  public emit;
  constructor({
                roomId,
                localStream,
                displayName,
                mode,
                protooUrl,
                videoContentHint,
                forceAudioCodec,
                audioContentHint,
                canvas,
                ctx,
                canvasArea
              }:RoomInfo) {
    super();
    // 设备标识标识
    this._device = deviceInfo();
    logger.debug('this._device', JSON.stringify(this._device));

    const peerId = generateRandomString({length:16});
    this._peerId = peerId;

    // Closed flag.
    // @type {Boolean}
    this._closed = false;

    // Room num
    // @type {String}
    this._roomId = roomId ? roomId :generateRandomString({length:8});

    // Display name.
    // @type {String}
    this._displayName = displayName ? displayName : generateRandomString({length:8});

    // video mode
    // @type {VIDEO_MODE}
    this._videoMode = mode;

    // 由于双向视频,是在对方加入房间后推流,对于多次加入房间要对推流状态进行判断
    // @type {Boolean}
    // TODO: 由于新需求需要多次推不同的流,不该在这里做全局判断
    this._produced = false;

    // Whether we should consume.
    // @type {Boolean}
    this._consume = true;

    // Whether we want DataChannels.
    // @type {Boolean}
    this._useDataChannel = true;

    // Whether we want DataChannels.
    // @type {Boolean}
    this._canUseDataChannel = false;

    this._videoContentHint = videoContentHint ? videoContentHint : "detail";
    this._audioContentHint = audioContentHint ? audioContentHint : "";

    this._forceAudioCodec = forceAudioCodec ? forceAudioCodec : "audio/opus";

    this.canvas = canvas;
    this.ctx = ctx;
    this.drawdown = false;
    this.startPointX;
    this.startPointY;
    this.toolcontrol = {
      pencil: false,
      line: false,
    }
    this.convasData = null;
    this.drawcolor = "#000";
    this.curtool;
    this.textvalue;
    this.canvasArea = canvasArea;
    this.shareDesktopWidth = 0;
    this.shareDesktopHeight = 0;
    this.widthRatio = 0;
    this.heeightRatio = 0;

    switch (this._videoMode) {
      case VIDEO_MODE.SINGLE:
        this._produce = true;
        this._consume = true;
        this._useDataChannel = false;
        break;
      case VIDEO_MODE.TWOWAY:
        this._produce = true;
        this._consume = true;
        this._useDataChannel = false;
        break;
      default:
    // TODO: 抛出异常
    }
    // http://192.168.80.184:3000/fileServicer/upload

    this._fileServiceUrl = 'http' + protooUrl.slice(2) + '/fileServicer/upload';
    logger.debug('FileServiceUrl', this._fileServiceUrl);
    // Protoo URL.
    // @type {String}
    if (protooUrl.slice(-1) != '/') {
      protooUrl += '/';
    }
    this._protooUrl = protooUrl + "?roomId=" + this._roomId + "&peerId=" + peerId + "&videoMode=" + this._videoMode;
    logger.warn('_protooUrl:',this._protooUrl);

    // protoo-client Peer instance.
    // @type {protooClient.Peer}
    this._protoo = undefined;

    // mediasoup-client Device instance.
    // @type {mediasoupClient.Device}
    this._mediasoupDevice = undefined;

    // mediasoup Transport for sending.
    // @type {mediasoupClient.Transport}
    this._sendTransport = undefined;

    // mediasoup Transport for receiving.
    // @type {mediasoupClient.Transport}
    this._recvTransport = undefined;

    // Local mic mediasoup Producer.
    // @type {mediasoupClient.Producer}
    this._micProducer = undefined;

    // Local webcam mediasoup Producer.
    // @type {mediasoupClient.Producer}
    this._webcamProducer = undefined;

    // Local mic mediasoup Producer.
    // @type {mediasoupClient.Producer}
    this._recordVideoProducer = undefined;

    // Local webcam mediasoup Producer.
    // @type {mediasoupClient.Producer}
    this._recordAudioProducer = undefined;

    // Local share mediasoup Producer.
    // @type {mediasoupClient.Producer}
    this._shareProducer = undefined;

    // Local chat DataProducer.
    // @type {mediasoupClient.DataProducer}
    this._chatDataProducer = undefined;

    // mediasoup Consumers.
    // @type {Map<String, mediasoupClient.Consumer>}
    this._consumers = new Map();

    // mediasoup DataConsumers.
    // @type {Map<String, mediasoupClient.DataConsumer>}
    this._dataConsumers = new Map();

    // Local stream
    // @type {Mediastream}
    this._localStream = localStream;

    // Remote stream
    // @type {Mediastream}
    // this._remoteStream = new MediaStream();
    this._remoteStream = undefined;
    this._shareDesktopStream = undefined;

    // Mixed stream
    // @type {Mediastream}
    this._recordMixedStream = undefined;

    // Mixer
    // @type {MultiStreamsMixer}
    this._recordMixer = undefined;

    // Signal transport
    // @type {protooClient.WebSocketTransport}
    this._protooTransport = undefined;

    this._videoCodecOptions = {
      videoGoogleStartBitrate: 0,
      videoGoogleMinBitrate: 0,
      videoGoogleMaxBitrate: 0
    }

    this._getBitrateInterval = null;
    this._trackMp3 = null;

    this._lastBytesRecv = 0;
    this._lastBytesSent = 0;
    this._lastRecordBytesSent = 0;
    this._lastPackets = 0; //调试用

    this._recordedChunks = [[]];
    this._mediaRecorder;
    this._stopRecord = false;
  }

  async _setRtpCapabilities() {

    // TODO:  获取Device信息
    // this._mediasoupDevice = new mediasoupClient.Device();

    // 获取媒体能力
    // routerRtpCapabilities.codecs 服务的能力
    const routerRtpCapabilities =
      await this._protoo.request('getRouterRtpCapabilities');
    logger.debug('getRouterRtpCapabilities',JSON.stringify(routerRtpCapabilities));
    // {
    //   // urn:3gpp:video-orientation 字段
    //   // 会让视频自动旋转
    //   // 删除之后能得到正确视频,手机旋转之后图像跟着旋转
    //   // producer中videoorientationchange事件,但是在服务器端拿到方向
    //   // https://mediasoup.org/documentation/v3/mediasoup/api/
    //
    //   // 视频模糊和这里无关
    //   routerRtpCapabilities.headerExtensions = routerRtpCapabilities.headerExtensions.
    //   filter((ext) => ext.uri !== 'urn:3gpp:video-orientation');
    // }

    // this._mediasoupDevice._extendedRtpCapabilities 本地和服务器都支持的编解码器,包含服务器的约束
    // this._mediasoupDevice.rtpCapabilities
    // 如果服务器没有支持的编解码器
    // 1.单向视频修改为默认本地的编解码器,服务器保存该编解码
    // 2.双向视频此处应该为对方能力,但是目前双向视频的码率等设置还在服务上
    // TODO: 加载媒体能力 发送到服务器
    // await this._mediasoupDevice.load({
    //   routerRtpCapabilities
    // });
    //
    // await this._protoo.request(
    //   'setRtpCapabilities', {
    //   rtpCapabilities: this._mediasoupDevice._extendedRtpCapabilities
    // });
    // JSON rtpCapabilities;
    const initInfo = testNapi.initMediasoup();
    if(initInfo)
      logger.debug("初始化mediasoup成功",initInfo.toString());

    const roomRtpCapabilities = testNapi.getMediasoupDevice(JSON.stringify(routerRtpCapabilities));
    logger.debug('gerRtpCapabilities',JSON.stringify(roomRtpCapabilities));


    const transportInfo =
      await this._protoo.request('createWebRtcTransport',{
        forceTcp: false,
        producing: true,
        consuming: false,
        sctpCapabilities: true
      }).catch((err)=>{
        logger.error('获取服务器信息失败',JSON.stringify(err));
      });
    logger.debug('transportInfo: ',JSON.stringify(transportInfo));

    testNapi.connectMediastream(JSON.stringify(transportInfo));

    // // wss连接
    // await this._protoo.request(
    //   'join', {
    //   displayName: this._displayName,
    //   device: this._device,
    //   rtpCapabilities: this._mediasoupDevice.rtpCapabilities,
    //   sctpCapabilities: this._useDataChannel && this._consume ?
    //   this._mediasoupDevice.sctpCapabilities : undefined
    // });
  }


  async joinRoom() {
    // 创建websockt通信,WebSocketTransport 是一种特殊处理过的websocket
    logger.debug("protooUrl: %s", this._protooUrl);

    // 无法获取连接出错误后错误原因
    if (!this._protooTransport) {
      try {
        this._protooTransport = new WebSocketTransport(this._protooUrl,'protoo');
        this._protooTransport.on('open', () => {
          logger.debug('创建WebSocketTransport成功!');
        });

        this._protooTransport.on('failed', (currentAttempt) => {
          this.emit('reconnection', currentAttempt);
          if (this._getBitrateInterval) {
            clearInterval(this._getBitrateInterval);
            this._getBitrateInterval = null;
          }
          if (currentAttempt >= 100) {
            this._protooTransport.close();
            this._protooTransport = null;
            this.emit('error', '和视频服务器建立连接失败');
          }
        });

        this._protooTransport.on('close', (event) => {
          logger.error('WebSocketTransport closed: %O', event.reason);
          this.emit('error', event.reason);
        });

      } catch (error) {
        logger.error('_protooTransport', error);
        this.emit('error', error);
      }
    }
    else {
      logger.warn('protooTransport 已经创建!')
    }

    // this._protoo 多次初始化时候,不会有多个连接
    this._protoo = new Peer(this._protooTransport);
    this._protoo.on('open', async () => {
      logger.debug('创建protoo client成功!');
      this.emit('connected');

      //初始化视频
      // 获取媒体信息
      await this._setRtpCapabilities();

      // await this._connectMediastream();

      if (this._videoMode === VIDEO_MODE.SINGLE) { // 单向在这里推流
        logger.warn('pushed signal stream');
        let audioTracks = this._localStream.getAudioTracks();
        if (audioTracks.length) {
          // await this._enableRecordAudio(audioTracks[0]);
        } else {
          logger.warn('Can not find audioTracks')
        }
        let videoTracks = this._localStream.getVideoTracks();
        if (videoTracks.length) {
          // await this._enableRecordVideo(videoTracks[0]);
        } else {
          logger.warn('Can not find videoTracks')
        }
      }

    });

    this._protoo.on('failed', () => {
      logger.error("protoo websocket failed !");
    });

    this._protoo.on('disconnected', () => {
      if (this._closed) {
        logger.warn('disconnected: The connection is down and closed!');
        return;
      }
      logger.debug('disconnected: The connection is down !');
      // this.close();
      // this.emit('closed');

      if (this._sendTransport) {
        this._sendTransport.close();
        this._sendTransport = null;
      }

      if (this._recvTransport) {
        this._recvTransport.close();
        this._recvTransport = null;
      }
      // closeStream(this._remoteStream);
      this._remoteStream = undefined;
      // closeStream(this._shareDesktopStream);
      this._shareDesktopStream = undefined;
    });

    this._protoo.on('close', () => {
      if (this._closed) {
        logger.warn('close: The connection is down and closed!');
        return;
      }
      logger.debug('close: The connection is down !');
      // this.close();
      // this.emit('closed');
    });

    this._protoo.on('request', async (request, accept, reject) => {
      logger.debug(
        'proto "request" event [method:%s, data:%o]',
        request.method, request.data);

      switch (request.method) {
        case 'newConsumer': {

          if (!this._consume) {
            reject(403, 'I do not want to consume');
            break;
          }

          const {
            peerId,
            producerId,
            id,
            kind,
            rtpParameters,
            appData,
          } = request.data;

          try {
            const consumer = await this._recvTransport.consume({
              id,
              producerId,
              kind,
              rtpParameters,
              appData: {
                ...appData,
                peerId
              }
            });

            this._consumers.set(consumer.id, consumer);
            consumer.track.deviceInfo = consumer.appData.deviceInfo;


            logger.warn("获得新的track,consumerId:%o,appData:%o", consumer.id, appData);
            // if (!appData) {
            //   if (!this._remoteStream) this._remoteStream = new MediaStream();
            //   this._remoteStream.addTrack(consumer.track);
            //   this.emit('getRemoteStream', kind, this._remoteStream);
            // }
            // else if (appData.shareDesktop) {
            //   this.shareDesktopWidth = appData.shareDesktopWidth;
            //   this.shareDesktopHeight = appData.shareDesktopHeight
            //   if (!this._shareDesktopStream) this._shareDesktopStream = new MediaStream();
            //   this._shareDesktopStream.addTrack(consumer.track);
            //   this.emit('getShareDesktopStream', kind, this._shareDesktopStream);
            // }
            // else {
            //   if (!this._remoteStream) this._remoteStream = new MediaStream();
            //   this._remoteStream.addTrack(consumer.track);
            //   this.emit('getRemoteStream', kind, this._remoteStream);
            // }
            // this.emit('getRemoteStream', kind, consumer.track);

            consumer.on('transportclose', () => {
              this._consumers.delete(consumer.id);
            });

            // We are ready. Answer the protoo request so the server will
            // resume this Consumer (which was paused for now if video).
            accept();

            // If audio-only mode is enabled, pause it.
            // if (consumer.kind === 'video')
            // 	this._pauseConsumer(consumer);
          } catch (error) {
            logger.error('newConsumer', error);
            this.emit('error', '获取对方视频流错误!');
          }

          break;
        }
        case 'newDataConsumer': {
          if (!this._consume) {
            reject(403, 'I do not want to data consume');

            break;
          }

          if (!this._useDataChannel) {
            reject(403, 'I do not want DataChannels');

            break;
          }

          const {
            peerId, // NOTE: Null if bot.
            dataProducerId,
            id,
            sctpStreamParameters,
            label,
            protocol,
            appData
          } = request.data;

          try {
            const dataConsumer = await this._recvTransport.consumeData({
              id,
              dataProducerId,
              sctpStreamParameters,
              label,
              protocol,
              appData: {
                ...appData,
                peerId
              } // Trick.
            });

            // Store in the map.
            this._dataConsumers.set(dataConsumer.id, dataConsumer);

            dataConsumer.on('transportclose', () => {
              this._dataConsumers.delete(dataConsumer.id);
              this.emit('textchat', 'close');
            });

            dataConsumer.on('open', () => {
              this.emit('textchat', 'open');
              this._canUseDataChannel = true;
            });

            dataConsumer.on('close', () => {
              this._dataConsumers.delete(dataConsumer.id);
              this.emit('textchat', 'close');
            });

            dataConsumer.on('error', (error) => {
              const errMsg = 'DataConsumer "error" event:%o' + error;
              this.emit('error', errMsg);
            });

            dataConsumer.on('message', (message) => {
              switch (dataConsumer.label) {
                case 'chat': {
                  this.emit('getRemoteMsg', message);
                  break;
                }

                case 'bot': {
                  break;
                }
              }
            });


            // We are ready. Answer the protoo request.
            accept();
          } catch (error) {
            logger.error(error);
            this.emit('error', '"newDataConsumer" request failed!');
          }

          break;
        }
      }
    });

    this._protoo.on('notification', (notification) => {
      // logger.debug(
      // 	'proto "notification" event [method:%s, data:%o]',
      // 	notification.method, notification.data);

      switch (notification.method) {
        case 'getOtherRtpCapabilities': {
          const rtpCapabilities = notification.data;
          logger.debug('getOtherRtpCapabilities', rtpCapabilities);
          break;
        }
        case 'producerScore': {
          const {
            producerId,
            score
          } = notification.data;
          logger.debug('producerId score:', producerId, score);
          this.emit('score', producerId, score);
          break;
        }
        case 'recordError': {
          const {
            peerId,
            error
          } = notification.data;
          logger.error('peerId:%s', peerId, error);
          this.emit('recordError', error);
          break;
        }
        case 'licenseError': {
          const {
            peerId,
            error
          } = notification.data;
          logger.error('peerId:%s', peerId, error);
          break;
        }
        case 'newPeer': {
          const {
            id,
            displayName,
            device,
            rtpCapabilities
          } = notification.data;
          this._mediasoupDevice.otherRtpCapabilities = rtpCapabilities;
          // logger.debug('otherRtpCapabilities', this._mediasoupDevice.otherRtpCapabilities);
          // logger.debug('id: %s, displayName: %s, device: %o', id, displayName, device);
          if (this._videoMode === VIDEO_MODE.TWOWAY) { // 双向在这里推流
            // this._startProduce(); // TODO: 应该在这里断了之前的视频再推流,而且要验证拿到对方的编码器
          } else if (this._videoMode === VIDEO_MODE.SINGLE) {
            // logger.error('newPeer: single -> newPeer!');
          } else {
            // 扩展位
          }
          this.emit('otherJoined');
          break;
        }
        case 'textMsg': {
          const {
            consumerId,
            textMsg
          } = notification.data;
          // logger.debug(consumerId, textMsg);
          this.emit('getRemoteMsg', textMsg);
          break;
        }
        case 'drawMsg': {
          const {
            consumerId,
            drawMsg
          } = notification.data;
          // logger.debug(consumerId, textMsg);
          this.emit('getDrawMsg', drawMsg);

          break;
        }
        case 'peerClosed': {
          const {
            peerId
          } = notification.data;
          this.emit('otherLeaved');
          break;
        }

        case 'downlinkBwe': {
          // logger.debug('\'downlinkBwe\' event:%o', notification.data);
          break;
        }

        case 'consumerClosed': {
          const {
            consumerId
          } = notification.data;
          logger.warn('consumerClosed', consumerId);

          const consumer = this._consumers.get(consumerId);

          if (!consumer)
            break;

          this._remoteStream.removeTrack(consumer.track);
          if (this._shareDesktopStream)
            this._shareDesktopStream.removeTrack(consumer.track);
          consumer.close();
          this._consumers.delete(consumerId);
          const {
            peerId
          } = consumer.appData;

          break;
        }

        case 'consumerPaused': {
          const {
            consumerId
          } = notification.data;
          const consumer = this._consumers.get(consumerId);

          if (!consumer)
            break;

          consumer.pause();

          break;
        }

        case 'consumerResumed': {
          const {
            consumerId
          } = notification.data;
          const consumer = this._consumers.get(consumerId);

          if (!consumer)
            break;

          consumer.resume();

          break;
        }

        case 'consumerLayersChanged': {
          const {
            consumerId,
            spatialLayer,
            temporalLayer
          } = notification.data;
          const consumer = this._consumers.get(consumerId);

          if (!consumer)
            break;
          break;
        }

        case 'consumerScore': {
          const {
            consumerId,
            score
          } = notification.data;
          logger.debug('consumerScore', score);
          break;
        }

        case 'dataConsumerClosed': {
          const {
            dataConsumerId
          } = notification.data;
          const dataConsumer = this._dataConsumers.get(dataConsumerId);

          if (!dataConsumer)
            break;

          dataConsumer.close();
          this._dataConsumers.delete(dataConsumerId);

          const {
            peerId
          } = dataConsumer.appData;
          break;
        }

        case 'activeSpeaker': {
          const {
            peerId
          } = notification.data;
          break;
        }
        case 'rtp2webrtcStart': {
          const {
            fileName
          } = notification.data;
          this.emit('rtp2webrtcStart', fileName);
          // logger.warn('开始播报');
          // this.producerCtrl('mic', 'pause');
          break;
        }
        case 'rtp2webrtcEnded': {
          const {
            fileName
          } = notification.data;
          this.emit('rtp2webrtcEnded', fileName);
          // logger.warn('结束播报');
          // this.producerCtrl('mic', 'resume');
          break;
        }
        default: {
          logger.error(
            'unknown protoo notification.method "%s"', notification.method);
        }
      }
    });
  }
}
