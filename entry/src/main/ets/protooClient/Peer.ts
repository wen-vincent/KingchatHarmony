import Logger from'../utils/Logger';
import Message from './Message';
import { EventEmitter } from '../polyfill/events';

const logger = new Logger('Peer');

export default class Peer extends EventEmitter
{
	/**
	 * @param {protoo.Transport} transport
	 *
	 * @emits open
	 * @emits {currentAttempt: Number} failed
	 * @emits disconnected
	 * @emits close
	 * @emits {request: protoo.Request, accept: Function, reject: Function} request
	 * @emits {notification: protoo.Notification} notification
	 */
	private _closed: Boolean;
  private _transport;
  private _connected;
  private _data;
  private _sents;
	public emit;
	public on;
	// private _logger;
	constructor(transport)
	{
		super();
		logger.debug('constructor()');

		// Closed flag.
		// @type {Boolean}
		this._closed = false;

		// Transport.
		// @type {protoo.Transport}
		this._transport = transport;

		// Connected flag.
		// @type {Boolean}
		this._connected = false;

		// Custom data object.
		// @type {Object}
		this._data = {};

		// Map of pending sent request objects indexed by request id.
		// @type {Map<Number, Object>}
		this._sents = new Map();

		// Handle transport.
		this._handleTransport();
	}

	/**
	 * Whether the Peer is closed.
	 *
	 * @returns {Boolean}
	 */
	get closed()
	{
		return this._closed;
	}

	/**
	 * Whether the Peer is connected.
	 *
	 * @returns {Boolean}
	 */
	get connected()
	{
		return this._connected;
	}

	/**
	 * App custom data.
	 *
	 * @returns {Object}
	 */
	get data()
	{
		return this._data;
	}

	/**
	 * Invalid setter.
	 */
	set data(data) // eslint-disable-line no-unused-vars
	{
		throw new Error('cannot override data object');
	}

	/**
	 * Close this Peer and its Transport.
	 */
	close()
	{
		if (this._closed)
			return;

		logger.debug('close()');

		this._closed = true;
		this._connected = false;

		// Close Transport.
		this._transport.close();

		// Close every pending sent.
		for (const sent of this._sents.values())
		{
			sent.close();
		}

		// Emit 'close' event.
		this.emit('close');
	}

	/**
	 * Send a protoo request to the server-side Room.
	 *
	 * @param {String} method
	 * @param {Object} [data]
	 *
	 * @async
	 * @returns {Object} The response data Object if a success response is received.
	 */
	async request(method, data = undefined)
	{
		logger.debug('request() [method:%s, id:%s]');

		const request = Message.createRequest(method, data);

		logger.debug('request() [method:%s, id:%s]' + method + request.id);

		// This may throw.
		await this._transport.send(request);

		return new Promise<Object>((pResolve, pReject) =>
		{
			const timeout = 1500 * (15 + (0.1 * this._sents.size));
			const sent =
			{
				id      : request.id,
				method  : request.method,
				resolve : (data2) =>
				{
					if (!this._sents.delete(request.id))
						return;

					clearTimeout(sent.timer);
					pResolve(data2);
				},
				reject : (error) =>
				{
					if (!this._sents.delete(request.id))
						return;

					clearTimeout(sent.timer);
					pReject(error);
				},
				timer : setTimeout(() =>
				{
					if (!this._sents.delete(request.id))
						return;

					pReject(new Error('request timeout'));
				}, timeout),
				close : () =>
				{
					clearTimeout(sent.timer);
					pReject(new Error('peer closed'));
				}
			};

			// Add sent stuff to the map.
			this._sents.set(request.id, sent);
		});
	}

	/**
	 * Send a protoo notification to the server-side Room.
	 *
	 * @param {String} method
	 * @param {Object} [data]
	 *
	 * @async
	 */
	async notify(method, data = undefined)
	{
		const notification = Message.createNotification(method, data);

		// this._logger.debug('notify() [method:%s]', method);

		// This may throw.
		await this._transport.send(notification);
	}

	_handleTransport()
	{
		if (this._transport.closed)
		{
			this._closed = true;

			setTimeout(() =>
			{
				if (this._closed)
					return;

				this._connected = false;

				this.emit('close');
			});

			return;
		}

		this._transport.on('open', () =>
		{
			if (this._closed)
				return;

			logger.debug('emit "open"');

			this._connected = true;

			this.emit('open');
		});

		this._transport.on('disconnected', () =>
		{
			if (this._closed)
				return;

			logger.debug('emit "disconnected"');

			this._connected = false;

			this.emit('disconnected');
		});

		this._transport.on('failed', (currentAttempt) =>
		{
			if (this._closed)
				return;

			logger.debug('emit "failed" [currentAttempt:%s]', currentAttempt);

			this._connected = false;

			this.emit('failed', currentAttempt);
		});

		this._transport.on('close', () =>
		{
			if (this._closed)
				return;

			this._closed = true;

			logger.debug('emit "close"');

			this._connected = false;

			this.emit('close');
		});

		this._transport.on('message', (message) =>
		{
			if (message.request)
				this._handleRequest(message);
			else if (message.response)
				this._handleResponse(message);
			else if (message.notification)
				this._handleNotification(message);
		});
	}

	_handleRequest(request)
	{
		try
		{
			this.emit('request',
				// Request.
				request,
				// accept() function.
				(data) =>
				{
					const response = Message.createSuccessResponse(request, data);

					this._transport.send(response)
						.catch(() => {});
				},
				// reject() function.
				(errorCode, errorReason) =>
				{
					if (errorCode instanceof Error)
					{
						errorReason = errorCode.message;
						errorCode = 500;
					}
					else if (typeof errorCode === 'number' && errorReason instanceof Error)
					{
						errorReason = errorReason.message;
					}

					const response =
						Message.createErrorResponse(request, errorCode, errorReason);

					this._transport.send(response)
						.catch(() => {});
				});
		}
		catch (error)
		{
			const response = Message.createErrorResponse(request, 500, String(error));

			this._transport.send(response)
				.catch(() => {});
		}
	}

	_handleResponse(response)
	{
		const sent = this._sents.get(response.id);

		if (!sent)
		{
			logger.error(
				'received response does not match any sent request [id:%s]', response.id);

			return;
		}

		if (response.ok)
		{
			sent.resolve(response.data);
		}
		else
		{
			const error = new Error(response.errorReason);

			// error.code = response.errorCode;
			sent.reject(error);
		}
	}

	_handleNotification(notification)
	{
		this.emit('notification', notification);
	}
}
