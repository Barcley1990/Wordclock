
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

document.getElementById('#btn_esp_rst').addEventListener('click', function() {
    console.log('button clicked');
    websocket.send('#esp_reset');
});
document.getElementById('#btn_led_pwr').addEventListener('click', function() {
    console.log('button clicked');
    websocket.send('#pwr_on_off');
});
document.getElementById('#btn_rst_sldr').addEventListener('click', function() {
    console.log('button clicked');
    websocket.send('#color_reset');
});

function init() {
    console.log('Trying to open a WebSocket connection...')
    websocket = new WebSocket(gateway + ':81/');

    websocket.onopen = function() {
        console('Open connection');
        document.getElementById('slider1').innerHTML = 50;
        document.getElementById('slider2').innerHTML = 50;
        document.getElementById('sliderValue3').innerHTML = 50;
    };
      websocket.onclose = function() {
          log.console('Close connection');
      };

    websocket.onmessage = function(event) {
        console.log(event.data);
        var JSONObj = JSON.parse(event.data);
        var keys = Object.keys(JSONObj);

        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            if(key == 'Light') {
                document.getElementById(key).innerHTML.value = JSONObj[key];
            }
            else if(key == 'Time') {
                document.getElementById(key).innerHTML.value = JSONObj[key];
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



