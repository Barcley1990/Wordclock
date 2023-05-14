

const buttonESPReset = document.getElementById('#btn_esp_rst');
const buttonLEDPower = document.getElementById('#btn_led_pwr');
const buttonSliderReset = document.getElementById('#btn_rst_sldr');
const sliderChannelRed =  document.getElementById('slider1');
const sliderChannelGreen =  document.getElementById('slider2');
const sliderChannelBlue =  document.getElementById('slider3');


if(buttonESPReset) {
    buttonESPReset.addEventListener('click', function() {
        console.log('button clicked');
        websocket.send('#esp_reset');
    });
} else {
    console.log('Button does not exist')
}

if(buttonLEDPower) {
    buttonLEDPower.addEventListener('click', function() {
        console.log('button clicked');
        websocket.send('#pwr_on_off');
    });
} else {
    console.log('Button does not exist')
}

if(buttonSliderReset) {
    buttonSliderReset.addEventListener('click', function() {
        console.log('button clicked');
        websocket.send('#color_reset');
    });
} else {
    console.log('Button does not exist')
}


function init() {
    console.log('Trying to open a WebSocket connection to ' + 'ws://' + window.location.hostname + ':81/');
    websocket = new WebSocket('ws://'+ window.location.hostname +':81/', ['arduino']);

    websocket.onopen = function() {
        console.log('Open connection');
    };

    websocket.onclose = function() {
        console.log('Close connection');
    };

    websocket.onmessage = function(event) {
        var JSONObj = JSON.parse(event.data);
        var keys = Object.keys(JSONObj);

        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            if(key == 'Light') {
                console.log(key); console.log(JSONObj[key]);
                document.getElementById(key).innerHTML = JSONObj[key];
            }
            else if(key == 'Time') {
                console.log(key); console.log(JSONObj[key]);
                document.getElementById(key).innerHTML = JSONObj[key];
            }
            else {
                document.getElementById(key).innerHTML = JSONObj[key];
                document.getElementById("slider"+ (i+1).toString()).value = JSONObj[key];
            } 
        }
    };
}

function updateSliderPWM(element) {
    var sliderNumber = element.id.charAt(element.id.length-1);
    var sliderValue = document.getElementById(element.id).value;
    document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
    console.log(sliderValue);
    websocket.send(sliderNumber+"s"+sliderValue.toString());
}



window.onload = function(event) {
    init();
}



