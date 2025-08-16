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

function updateSensorValue(){        // 0 -> temperature, 1 -> pH, 2 -> pressure
    websocket.send("updateSensorValue");



}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function initButton() {
    //document.getElementById('button').addEventListener('click', function() {
    //    updateSensorValue();
    //});
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

        document.getElementById('buttonTemp').addEventListener('click', function() {
            window.location.href = `index.html`;
        });
        document.getElementById('buttonMainDash').addEventListener('click', function() {
            window.location.href = `index.html`;
        });
        document.getElementById('buttonMainDash').addEventListener('click', function() {
            window.location.href = `index.html`;
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

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        let value = myObj[key];
        value = value.toFixed(2);
        document.getElementById(key).innerHTML = value;
    }
}