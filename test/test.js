function connect() {
    ws = new WebSocket('ws://localhost:5333');
    // ws.binaryType = 'arraybuffer'
    ws.onopen = function(ev) {
        console.log('opened');
        ws.send('abcdefg');
        ws.send('12345678');
        ws.send('ABCDEFG');
    };
    ws.onmessage = function(ev) {
        console.log(ev);
        last = ev.data;
    };
    ws.onclose = function(ev) {
        console.log('CLOSED');
    };
    ws.onerror = function(ev) {
        console.log('ERROR', ev);
    };
}
connect();
