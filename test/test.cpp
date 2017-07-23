
#include "../src/websocket.hpp"

#include <string>

const char *opcodes[] = {
    "Additional",           // 0
    "Text",                 // 1
    "Binary",               // 2
    "", "", "", "", "",     // 3-7
    "Close",                // 8
    "Ping",                 // 9
    "Pong",                 // 0xA
    "", "", "", "", ""      // 0xB-0xF
};

using namespace std;
using namespace websocket;

class MyHandler : public WebSocketHandler {
public:
    static void onopen(MyHandler& h) {
        cout << "OPEN: " << h.getHost() << endl;
        cout << "GET " << h.getPath() << endl;
        cout << "Host: " << h.getHost() << endl;
        cout << "Sec-WebSocket-Key: " << h.getKey() << endl;
        cout << "Sec-WebSocket-Protocol: " << h.getProtocol() << endl;
    }
    static void onmessage(MyHandler& h) {
        cout << h.data() << endl;
    }
    static void onclose(MyHandler& h) {
        cout << "CLOSED" << endl;
    }
};

void test1() {
    tstream::server ser("127.0.0.1", 5333);
    WebSocketHandler h = ser.accept();
    if (h) {
        cout << "GET " << h.getPath() << endl;
        cout << "Host: " << h.getHost() << endl;
        cout << "Sec-WebSocket-Key: " << h.getKey() << endl;
        cout << "Sec-WebSocket-Protocol: " << h.getProtocol() << endl;
        while (1) {
            if (h.recv())
                cout << h.data() << endl,
                h.send(h.data(), true);
            getchar();
        }
    }
}

int main(int argc, char **argv) {
    WebSocketServer<MyHandler> ser("127.0.0.1", 5333);
    ser.run();
    return 0;
}
