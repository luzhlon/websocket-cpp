
#include "../src/websocket.hpp"

#include <string>
#include <Windows.h>

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

int main(int argc, char **argv) {
    tstream::server ser(5333);
    WebSocketHandler h = ser.accept();
    if (h) {
        cout << "GET " << h.getPath() << endl;
        cout << "Host: " << h.getHost() << endl;
        cout << "Sec-WebSocket-Key: " << h.getKey() << endl;
        cout << "Sec-WebSocket-Protocol: " << h.getSubProtocol() << endl;
        while (1) {
            if (h.recv())
                cout << h.data() << endl,
                h.send(h.data(), true);
            getchar();
        }
    }
    return 0;
}
