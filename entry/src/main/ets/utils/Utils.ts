/**
 * Generates a random positive integer.
 *
 * @returns {Number}
 */
const  numbers = '0123456789',
	letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz',
	specials = '!$%^&*()_+|~-=`{}[]:;<>?,./';

function _defaults (opts) {
	opts || (opts = {});
	return {
		length: opts.length || 8,
		numeric: typeof opts.numeric === 'boolean' ? opts.numeric : true,
		letters: typeof opts.letters === 'boolean' ? opts.letters : true,
		special: typeof opts.special === 'boolean' ? opts.special : false,
		exclude: Array.isArray(opts.exclude)       ? opts.exclude : []
	};
}

function _buildChars (opts) {
	var chars = '';
	if (opts.numeric) { chars += numbers; }
	if (opts.letters) { chars += letters; }
	if (opts.special) { chars += specials; }
	for (var i = 0; i <= opts.exclude.length; i++){
		chars = chars.replace(opts.exclude[i], "");
	}
	return chars;
}

export function generateRandomString(opts) {
	opts = _defaults(opts);
	var i, rn,
		rnd = '',
		len = opts.length,
		exclude = opts.exclude,
		randomChars = _buildChars(opts);
	for (i = 1; i <= len; i++) {
		rnd += randomChars.substring(rn = Math.floor(Math.random() * randomChars.length), rn + 1);
	}
	return rnd;
};

export function generateRandomNumber() {
	return Math.round(Math.random() * 10000000);
};

export const deviceInfo = () => {

	return {
		flag:'Harmony', // 浏览器名字
		name: 'Harmony', // 浏览器名字
		version: 'Harmony OS Next 4.2', // 浏览器版本
		OSName: 'Harmony OS Next 4.2', // 系统名字
		OSVersion: 'Harmony OS Next 4.2', // 系统版本
		isIosWechat: 'false',
		kingchatVersion: '1.0'
	};
}