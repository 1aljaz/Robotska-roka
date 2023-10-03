const gateway = `ws://${window.location.hostname}/ws`;
let websocket;

window.addEventListener('load', initWebSocket);

function initWebSocket() {
  console.log('Opening WebSocket...');
  websocket = new WebSocket(gateway);
  websocket.addEventListener('open', onOpen);
  websocket.addEventListener('close', onClose);
  websocket.addEventListener('message', onMessage);
}

function onOpen() {
  console.log('WebSocket je odprt');
  getValues();
}

function onClose() {
  console.log('WebSocket je zaprt');
  setTimeout(initWebSocket, 2000);
}

function getValues() {
  if (websocket.readyState === WebSocket.OPEN) {
    websocket.send("getValues");
  } else {
    console.error('WebSocket je vrgl vn');
  }
}

function updateSlider(element) {
  const sliderNumber = element.id.slice(-1);
  const sliderValue = element.value;
  document.getElementById(`sliderValue${sliderNumber}`).innerHTML = sliderValue;
  console.log(sliderValue);
  websocket.send(`${sliderNumber}s${sliderValue}`);
}


function onMessage(event) {
  console.log(event.data);
  var tmp = JSON.parse(event.data);
  
  for (var key in tmp) {
    if  (tmp.hasOwnProperty(key)) {
      document.getElementById(key).innerHTML = tmp[key];
      document.getElementById("slider"+ (parseInt(key.substring(6)))).value = tmp[key];
    }
  }
}

//main.cpp

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("Client se je skonektu iz: %s\n", client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("Client se je dikonektu\n");
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}


