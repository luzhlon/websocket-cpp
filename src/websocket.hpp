#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__

#include <string>
#include <thread>
#include <stdint.h>

#include "tstream.hpp"

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
        enum { BIT_FIN = 0x80, BIT_MASK = 0x80 };
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
            memcpy(masking_key, key, sizeof(masking_key));
            MASK(true); return *this;
        }
        Frame& setMaskingKey(uint32_t key) {
            return setMaskingKey((const char *)&key);
        }
        // Send the frame with data as the application data
        bool send(tstream& ts, const char *data, size_t size);
        // Receive the frame, append the data to out
        bool recv(tstream& ts, string& out);

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
        static void onopen(WebSocketHandler& h);
        static void onmessage(WebSocketHandler& h);
        static void onerror(WebSocketHandler& h);
        static void onclose(WebSocketHandler& h);

    public:
        WebSocketHandler() = default;
        // Default as the server
        WebSocketHandler(tstream&& ts)
            : _ts(move(ts)) { open(); }
        // Move constructor
        WebSocketHandler(WebSocketHandler&& other)
            : _ts(move(other._ts)) {
            _state = other._state;
            other._state = STATE_CLOSED;
            _server = other._server;
            _frame = other._frame;
            _header = other._header;
            other._header = nullptr;
        }
        ~WebSocketHandler() {
            if (isConnected())
                close();
            delete _header;
        }

        bool isServer() { return _server; }
        void isServer(bool b) { _server = b; }
        // Is connected(opened)
        bool isConnected() { return STATE_OPEN == _state; }
        // Get the header
        const char *header() const { return _header; }
        // Get the path from header
        string getPath();
        // Get the key from header
        string getKey();
        // Get the protocol from header
        string getProtocol();
        // Get the host from header
        string getHost();
        // Reference to _data
        string& data() { return _data; }
        // Get the state
        READY_STATE readyState() { return _state; }
        // Send a message
        bool send(const char *data, size_t len, bool bin = false);
        // Send a message
        bool send(const string& data, bool bin = false) {
            return send(data.c_str(), data.size(), bin);
        }
        // Receive a message
        WebSocketHandler& recv(bool *bin = nullptr);
        // Close
        void close() {
            if (isConnected()) {
                _ts.close();
                _state = STATE_CLOSED;
            }
        }
        // Is connected(opened)
        operator bool() { return isConnected(); }
        // Initialize with tcp stream
        bool init(tstream&& ts) {
            _ts = std::move(ts);
            return open();
        }

    private:
        // Open, make the connection
        bool open();
        void responseHeader(const string& key);
        void readHeader();
        string calcResponseKey(const string& key);

    protected:
        tstream _ts;

    private:
        string _data;
        Frame _frame;
        char *_header = nullptr;
        bool _server = true;// as the server role? no mask
        READY_STATE _state = STATE_CLOSED;
    };

    enum AC_CODE {
        ACCEPT_PASS,    // Passed, the server will create a thread run the handler
        ACCEPT_NONE,    // None, do nothing --- don't create thread
        ACCEPT_DROP,    // Drop, drop the handler
        ACCEPT_STOP     // Stop, stop the run-loop
    };

    template <class HANDLER>
    class WebSocketServer : public tstream::server {
    public:
        typedef WebSocketServer<HANDLER> MyT;

        template <typename... Args>
        WebSocketServer(Args... args) : tstream::server(args...) {}

        static void work_thread(HANDLER&& h) {
            if (!h.isConnected()) return;
            HANDLER::onopen(h);
            while (true) {
                if (h.recv())
                    HANDLER::onmessage(h);
                else if (h.isConnected())
                    HANDLER::onerror(h);
                else
                    return HANDLER::onclose(h);
            }
        }

        HANDLER accept() {
            HANDLER h;
            h.init(tstream::server::accept());
            h.isServer(true);
            return std::move(h);
        }

        bool run() {
            while (true) {
                auto h = accept();
                auto code = onaccept(h);
                if (ACCEPT_PASS == code) {
                    thread t(work_thread, move(h));
                    t.detach();
                } else if (ACCEPT_DROP == code) {
                    h.close();
                } else if (ACCEPT_STOP == code) {
                    return true;
                }
            }
        }
        
    protected:
        virtual AC_CODE onaccept(HANDLER& h) { return ACCEPT_PASS; }
    };
}

#endif /* __WEBSOCKET_H__ */
