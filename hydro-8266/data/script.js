var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    initButton();
}

function getReadings(){
    websocket.send("getReadings");
}

function updateSensorValue(sensor){        // 0 -> temperature, 1 -> pH, 2 -> pressure, 3 -> TDS, 4 -> Flying Fish
    //websocket.send("updateSensorValue");
    if(sensor === 0) {
        let element = document.getElementById('inputTemp');
        let value = element.value;
        element.value = ''; // Clear the input field after sending
        websocket.send(`refTemp:${value}`);
    }else if(sensor === 1) {
        let element = document.getElementById('inputpH');
        let value = element.value;
        element.value = '';
        websocket.send(`refpH:${value}`);
    } else if(sensor === 2) {
        let element = document.getElementById('inputPres');
        let value = element.value;
        element.value = '';
        websocket.send(`refPres:${value}`);
    }else if(sensor === 3) {
        let element = document.getElementById('inputTDS');
        let value = element.value;
        element.value = '';
        websocket.send(`refTDS:${value}`);
    }else if(sensor === 4) {
        let element = document.getElementById('inputFF');
        let value = element.value;
        element.value = '';
        websocket.send(`refFF:${value}`);
    }
}

function adjustMotors(){        
    //websocket.send("updateSensorValue");

    let element = document.getElementById('adjMotorIDinput');
    let id = element.value;
    element.value = ''; // Clear the input field after sending
    element = document.getElementById('adjMotorValueinput');
    let value = element.value;
    element.value = '';
    element.value = ''; // Clear the input field after sending
    websocket.send(`${id}:${value}`);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function initButton() {
    const btnAdmin = document.getElementById('buttonAdmin');
    const btnMainDash = document.getElementById('buttonMainDash');
    if(btnAdmin) {          // In main dashboard
        btnAdmin.addEventListener('click', function() {
            window.location.href = `admin.html`;
        });
    }
    
    if(btnMainDash) {       // In admin panel
        btnMainDash.addEventListener('click', function() {
            window.location.href = `index.html`;
        });

        document.getElementById('buttonTempRef').addEventListener('click', function() {
            updateSensorValue(0);
        });
        document.getElementById('buttonpHRef').addEventListener('click', function() {
            updateSensorValue(1);
        });
        document.getElementById('buttonPresRef').addEventListener('click', function() {
            updateSensorValue(2);
        });
        document.getElementById('buttonTDSRef').addEventListener('click', function() {
            updateSensorValue(3);
        });
        document.getElementById('buttonFFRef').addEventListener('click', function() {
            updateSensorValue(4);
        });
        document.getElementById('buttonAdjMotor').addEventListener('click', function() {
            adjustMotors();
        });
    }
    
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    getReadings();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP with the readings
function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        let value = myObj[key];
        value = value.toFixed(2);
        let docElement = document.getElementById(key);
        if (docElement) docElement.innerHTML = value;
    }
}