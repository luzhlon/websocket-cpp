
#include <regex>

#include "base64.hpp"
#include "sha1.hpp"
#include "websocket.hpp"

namespace websocket {
    using namespace std;

    bool Frame::send(tstream& ts, const char *data, size_t size) {
        // Send the header
        if (size < 126)
            paylen(size), ts.write2(_begin);
        else if (size <= 0xFFFF) {
            paylen(126);
            uint16_t len = htons(size);
            ts.write2(_begin).write2(len);
        } else {
            paylen(127);
            uint64_t len = htonll(size);
            ts.write2(_begin).write2(len);
        }
        // Send the application data
        if (size) {
            if (MASK()) {
                char buf[4000];
                auto rest = size;
                do {
                    auto n = sizeof(buf) < rest ? sizeof(buf) : rest;
                    for (size_t i = 0; i < n; i++)
                        buf[i] = data[i] ^ masking_key[i % 4];
                    ts.write(buf, n);
                    rest -= n;
                } while (rest);
            } else
                ts.write(data, size);
        }
        return ts.good();
    }

    bool Frame::recv(tstream& ts, string& out) {
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

    void WebSocketHandler::onopen(WebSocketHandler& h) {}
    void WebSocketHandler::onmessage(WebSocketHandler& h) {}
    void WebSocketHandler::onerror(WebSocketHandler& h) {}
    void WebSocketHandler::onclose(WebSocketHandler& h) {}

    string WebSocketHandler::calcResponseKey(const string& key) {
        static const char *GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        SHA1 sha(key + GUID);
        return base64::encode(sha.digest());
    }

    bool WebSocketHandler::send(const char *data, size_t len, bool bin) {
        if (!isServer())
            _frame.setMaskingKey(0xFFFFFFFF);
        return _frame.setOpcode(bin ? OP_BINARY : OP_TEXT)
                     .send(_ts, data, len);
    }

    WebSocketHandler& WebSocketHandler::recv(bool *bin) {
        _data.resize(0);
        if (!_frame.recv(_ts, _data))
            return _state = STATE_CLOSED, *this;
        switch (_frame.getOpcode()) {
        case OP_ADDITIONAL:
            while (!_frame.FIN())
                _frame.recv(_ts, _data);
            break;
        case OP_TEXT:
            if (bin) *bin = false;
            while (!_frame.FIN())
                _frame.recv(_ts, _data);
            break;
        case OP_BINARY:
            if (bin) *bin = true;
            while (!_frame.FIN())
                _frame.recv(_ts, _data);
            break;
        case OP_PING:
            _frame.setOpcode(OP_PONG)
                  .MASK(false).send(_ts, nullptr, 0);
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

    void WebSocketHandler::responseHeader(const string& key) {
        string resp =
            "HTTP/1.1 101 WebSocket Protocol Handshake\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: " + key + "\r\n";
        auto proto = getProtocol();
        if (!proto.empty())
            resp.append("Sec-WebSocket-Protocol: ")
                .append(proto).append("\r\n");
        resp += "\r\n";
        _ts.write(resp.c_str(), resp.size());
    }

    string WebSocketHandler::getPath() {
        cmatch cm;
        return regex_search(_header, cm,
                    regex(R"/(GET\s*(\S+)\s)/")) ?  cm[1].str() : "";
    }
    // Get the key from header
    string WebSocketHandler::getKey() {
        cmatch cm;
        return regex_search(_header, cm,
                    regex("[Ss]ec-[Ww]eb[Ss]ocket-[Kk]ey:\\s*(.+)\r\n")) ?  cm[1].str() : "";
    }
    // Get the subprotocol from header
    string WebSocketHandler::getProtocol() {
        cmatch cm;
        return regex_search(_header, cm,
                    regex("[Ss]ec-[Ww]eb[Ss]ocket-[P]rotocol:\\s*(.+)\r\n")) ?  cm[1].str() : "";
    }
    // Get the host from header
    string WebSocketHandler::getHost() {
        cmatch cm;
        return regex_search(_header, cm,
                    regex("[Hh]ost:\\s*(.+)\r\n")) ?  cm[1].str() : "";
    }

    bool WebSocketHandler::open() {
        _state = STATE_CONNECTING;
        // Receive the protocol header
        readHeader();
        // Find key and response
        auto key = getKey();
        if (!key.empty()) {
            // response header
            responseHeader(calcResponseKey(key));
            return _state = STATE_OPEN, true;
        }
        return _state = STATE_CLOSED, false;
    }

    void WebSocketHandler::readHeader() {
        char buf[BUFSIZ];
        string header;
        header.reserve(BUFSIZ);
        while (1) {
            auto n = _ts.recv(buf, sizeof(buf));
            if (n) {
                header.append(buf, n);
                n = header.size();
                if (n > 4 && !memcmp("\r\n\r\n", &header[n - 4], 4)) {
                    _header = new char[header.size() + 1];
                    memcpy(_header, header.c_str(), header.size());
                    _header[header.size()] = '\0';
                    return;
                }
            }
        }
    }
}
