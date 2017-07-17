#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__

#include <string>
#include <regex>
#include <stdint.h>

#include "../third_party/tstream/src/tstream.h"

#include "base64.h"
#include "sha1.hpp"

namespace websocket {
	enum OP_CODE {
		OP_ADDITIONAL = 0,
		OP_TEXT = 1,
		OP_BINARY = 2,
		// 3-7
		OP_CLOSE = 8,
		OP_PING = 9,
		OP_PONG = 0xA
		// 0xB-0xF
	};

	enum READY_STATE {
		STATE_CONNECTING = 0,
		STATE_OPEN = 1,
		STATE_CLOSING = 2,
		STATE_CLOSED = 3
	};

	enum CLOSE_CODE {
		CLOSE_NORMAL = 1000,
		CLOSE_GOING_AWAY = 1001,
		CLOSE_PROTO_ERROR = 1002,
		CLOSE_TYPE_ERROR = 1003,
		CLOSE_DATA_ERROR = 1007,
		CLOSE_TOO_BIG = 1009,
		CLOSE_NOT_EXPECTED = 1011
	};

	struct Frame {
		Frame() = default;
		Frame(Frame&& other) = default;
		enum {
			BIT_FIN = 0x80,
			BIT_MASK = 0x80
		};
		// Is the last slice of a message
		bool FIN() { return _begin[0] & BIT_FIN; }
		Frame& FIN(bool b) {
			if (b) _begin[0] |= BIT_FIN;
			else _begin[0] &= ~BIT_FIN;
			return *this;
		}
		// All flags
		uint8_t flags() { return _begin[0] & 0xF0; }
		// Oprating code
		uint8_t getOpcode() { return _begin[0] & 0x0F; }
		Frame& setOpcode(OP_CODE code) {
			_begin[0] = _begin[0] & 0xF0 | code;
			return *this;
		}
		// Existing maskcode
		bool MASK() { return _begin[1] & BIT_MASK; }
		Frame& MASK(bool b) {
			if (b) _begin[1] |= BIT_MASK;
			else _begin[1] &= ~BIT_MASK;
			return *this;
		}
		Frame& setMaskingKey(const char *key) {
			MASK(true);
			memcpy(masking_key, key, sizeof(masking_key));
			return *this;
		}
		Frame& setMaskingKey(uint32_t key) {
			return setMaskingKey((const char *)&key);
		}
		// Send the frame with data as the application data
		bool send(tstream& ts, const string& data) {
			// Send the length
			if (data.size() < 126)
				paylen(data.size()), ts.write2(_begin);
			else if (data.size() <= 0xFFFF) {
				paylen(126);
				uint16_t len = htons(data.size());
				ts.write2(_begin).write2(len);
			} else {
				paylen(127);
				uint64_t len = htonll(data.size());
				ts.write2(_begin).write2(len);
			}
			if (!ts) return false;
			// Send the application data
			if (data.size()) {
				if (MASK()) {
					string dup = data;
					for (size_t i = 0; i < dup.size(); i++)
						dup[i] ^= masking_key[i % 4];
					ts.write(dup.c_str(), dup.size());
				} else
					ts.write(data.c_str(), data.size());
			}
			return ts.gcount() == data.size();
		}
		// Receive the frame, append the data to out
		bool recv(tstream& ts, string& out) {
			ts.read2(_begin);
			uint16_t short_len;
			uint64_t long_len;
			size_t data_len = paylen();
			if (126 == data_len)
				ts.read2(short_len),
				data_len = ntohs(short_len);
			else if (127 == data_len)
				ts.read2(long_len),
				data_len = ntohll(long_len);
			if (MASK())
				ts.read2(masking_key);
			if (data_len) {
				auto last = out.size();
				out.resize(last + data_len);
				// Append application data to out
				ts.read(&out[last], data_len);
				if (MASK())
					for (size_t i = 0; i < data_len; i++)
						out[i+last] ^= masking_key[i % 4];
			}
			return true;
		}

	private:
		// Payload's length in the 2nd byte
		uint8_t paylen() { return _begin[1] & 0x7F; }
		Frame& paylen(uint8_t len) {
			_begin[1] = _begin[1] & BIT_MASK | len;
			return *this;
		}

		uint8_t _begin[2];
		uint8_t masking_key[4];
	};

	class WebSocketHandler {
	public:
		// Default as the server
		WebSocketHandler(tstream&& ts) : _ts(move(ts)) {}
		WebSocketHandler() { isServer(false); }
		WebSocketHandler(WebSocketHandler&& other) = default;

		bool isServer() { return _server; }
		// Set the handler to be server-point or client-point
		WebSocketHandler& isServer(bool b) {
			_server = b;
			return *this;
		}
		// Open, make the connection
		bool open() {
			_state = STATE_CONNECTING;
			// Receive the protocol header
			readHeader();
			// Find key and response
			auto key = getKey();
			if (!key.empty()) {
				// response header
				responseHeader(getResponseKey(key));
				return _state = STATE_OPEN, true;
			}
			return _state = STATE_CLOSED, false;
		}
		// Get the header
		const string& header() { return _header; }
		// Get the path from header
		string getPath() {
			smatch sm;
			return regex_search(_header, sm,
						regex(R"/(GET\s*(\S+)\s)/")) ?  sm[1].str() : "";
		}
		// Get the key from header
		string getKey() {
			smatch sm;
			return regex_search(_header, sm,
						regex("[Ss]ec-[Ww]eb[Ss]ocket-[Kk]ey:\\s*(.+)\r\n")) ?  sm[1].str() : "";
		}
		// Get the subprotocol from header
		string getSubProtocol() {
			smatch sm;
			return regex_search(_header, sm,
						regex("[Ss]ec-[Ww]eb[Ss]ocket-[P]rotocol:\\s*(.+)\r\n")) ?  sm[1].str() : "";
		}
		// Get the host from header
		string getHost() {
			smatch sm;
			return regex_search(_header, sm,
						regex("[Hh]ost:\\s*(.+)\r\n")) ?  sm[1].str() : "";
		}
		// Receive a message
		WebSocketHandler& recv() {
			_data.resize(0);
			if (!_frame.recv(_ts, _data))
				return _state = STATE_CLOSED, *this;
			switch (_frame.getOpcode()) {
			case OP_ADDITIONAL:
			case OP_TEXT:
			case OP_BINARY:
				while (!_frame.FIN())
					_frame.recv(_ts, _data);
				break;
			case OP_PING:
				_frame.setOpcode(OP_PONG)
					  .MASK(false).send(_ts, "");
				break;
			case OP_PONG:
				break;
			case OP_CLOSE:
				// ...
				_ts.close();
				_state = STATE_CLOSED;
				break;
			};
			return *this;
		}
		// Reference to _data
		string& data() { return _data; }
		// Get the state
		READY_STATE readyState() { return _state; }
		// Send a message
		bool send(const string& data, bool bin = false) {
			return _frame.MASK(!isServer())
						 .setOpcode(bin ? OP_BINARY : OP_TEXT)
						 .send(_ts, data);
		}
		// Is connected(opened)
		operator bool() { return STATE_OPEN == _state; }

	private:
		string getResponseKey(const string& key) {
			static const char *GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			SHA1 sha(key + GUID);
			return base64::encode(sha.digest());
		}
		void responseHeader(const string& key) {
			string resp =
				"HTTP/1.1 101 WebSocket Protocol Handshake\r\n"
				"Upgrade: websocket\r\n"
				"Connection: Upgrade\r\n"
				"Sec-WebSocket-Accept: " + key + "\r\n";
			auto proto = getSubProtocol();
			if (!proto.empty())
				resp.append("Sec-WebSocket-Protocol: ")
					.append(proto).append("\r\n");
			resp += "\r\n";
			_ts.write(resp.c_str(), resp.size());
		}
		void readHeader() {
			char buf[BUFSIZ];
			while (1) {
				auto n = _ts.recv(buf, sizeof(buf));
				if (n) {
					_header.append(buf, n);
					n = _header.size();
					if (n > 4 &&
						!memcmp("\r\n\r\n", &_header[n - 4], 4))
						return;
				}
			}
		}

	private:
		tstream _ts;
		string _header;
		string _data;
		Frame _frame;
		READY_STATE _state = STATE_CLOSED;
		bool _server = true;
	};

}

#endif /* __WEBSOCKET_H__ */
