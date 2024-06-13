import webSocket from '@ohos.net.webSocket';
import Logger from'../../utils/Logger';
import Message from '../Message';
import { BusinessError } from '@ohos.base';
import type { AsyncCallback, ErrorCallback, Callback } from '@ohos.base';
import UIAbility from '@ohos.app.ability.UIAbility';
import emitter from '@ohos.events.emitter';
import { EventEmitter } from '../../polyfill/events';


const logger = new Logger('WebSocketTransport');
export default class WebSocketTransport extends EventEmitter
{

	/**
	 * @param {String} url - WebSocket URL.
	 * @param {Object} [options] - Options for WebSocket-Node and retry.
	 */
	private _closed:Boolean;
	private _url: String;
	private _ws;
	public emit;
	public on:Function;
	constructor(url, options)
	{
		super();
		logger.debug(`constructor() [url:${url}, options:${options}]`);

		// Closed flag.
		// @type {Boolean}
		this._closed = false;

		// WebSocket URL.
		// @type {String}
		this._url = url;

		// WebSocket instance.
		// @type {WebSocket}
		this._ws = null;

		// Run the WebSocket.
		this._runWebSocket();

	}

	get closed()
	{
		return this._closed;
	}


	close()
	{
		if (this._closed)
			return;

		logger.debug('close()');

		// Don't wait for the WebSocket 'close' event, do it now.
		this._closed = true;
		this.emit('close');

		try
		{
			this._ws.onopen = null;
			this._ws.onclose = null;
			this._ws.onerror = null;
			this._ws.onmessage = null;
			this._ws.close();
		}
		catch (error)
		{
			logger.error('close() | error closing the WebSocket: %o', error);
		}
	}

	async send(message)
	{
		if (this._closed)
			throw new Error('transport closed');

		try
		{
			this._ws.send(JSON.stringify(message));
		}
		catch (error)
		{
			logger.warn('send() failed:%o', error);

			throw error;
		}
	}

	_runWebSocket()
	{
			if (this._closed)
			{
				return;
			}

			logger.debug('_runWebSocket()');

			this._ws = webSocket.createWebSocket();
			this._ws.on('open',(err: BusinessError, value: Object)=>{
				if (this._closed)
					return;
				logger.debug('onopen'+' '+JSON.stringify(value));
				this.emit('open',value);
			});
			this._ws.on('close',(err: BusinessError, value: Object)=>{
				if (this._closed)
					return;
				// logger.warn(
				// 	'WebSocket "close" event [wasClean:%s, code:%s, reason:"%s"]',
				// 	event.wasClean, event.code, event.reason);
				logger.warn('closed!');
				this._closed = true;
				// Emit 'close' event.
				this.emit('close',value);


			});
			this._ws.on('error',(err: BusinessError, value: Object)=>{
				if (this._closed)
					return;
				logger.debug('error'+' '+JSON.stringify(value));
				this.emit('error',value);


			});
			this._ws.on('message',(err: BusinessError, value: Object)=>{
				if (this._closed)
					return;
				logger.debug('message'+' '+JSON.stringify(value));
				const message = Message.parse(value);

				if (!message)
					return;
				this.emit('message',message);

			});

		this._ws.connect(this._url, (err: BusinessError, value: boolean) => {
			if (!err) {
				logger.debug("Connected successfully");
			} else {
				logger.debug("Connection failed. Err:" + JSON.stringify(err));
			}
		});
		};
	}
