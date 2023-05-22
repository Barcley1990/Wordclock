

const buttonESPReset = document.getElementById('#btn_esp_rst');
const buttonLEDPower = document.getElementById('#btn_led_pwr');


// Reset Button
if(buttonESPReset) {
    buttonESPReset.addEventListener('click', function() {
        console.log('button clicked');
        websocket.send('#esp_reset');
    });
} else {
    console.log('Button does not exist')
}

// LED Power Button
if(buttonLEDPower) {
    buttonLEDPower.addEventListener('click', function() {
        console.log('button clicked');
        websocket.send('#pwr_on_off');
    });
} else {
    console.log('Button does not exist')
}


//Window Init Function
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
            else if(key == 'version') {
                console.log[key]; console.log(JSONObj[key]);
                document.getElementById(key).innerHTML = ('Version: ' + JSONObj[key]);
            }
            else {
                document.getElementById(key).innerHTML = JSONObj[key];
                document.getElementById("slider"+ (i+1).toString()).value = JSONObj[key];
            } 
        }
    };
}

// Colorpicker function
$(function(){
    // create canvas and context objects
    var canvas = document.getElementById('picker');
    var ctx = canvas.getContext('2d');

    // drawing active image
    var image = new Image();

    image.src = 'images/colorwheel5.png';
    //image.crossOrigin = "Anonymous"

    image.onload = function () {
        // draw the image on the canvas
        ctx.drawImage(image, 0, 0, image.width, image.height); 
    }    

    // mouse move handler
    $('#picker').mousemove(function(e) { 
        // get coordinates of current position
        var canvasOffset = $(canvas).offset();
        var canvasX = Math.floor(e.pageX - canvasOffset.left);
        var canvasY = Math.floor(e.pageY - canvasOffset.top);

        // get current pixel
        var imageData = ctx.getImageData(canvasX, canvasY, 1, 1);
        var pixel = imageData.data;

        // update controls
        $('#rVal').val(pixel[0]);
        $('#gVal').val(pixel[1]);
        $('#bVal').val(pixel[2]);
        $('#rgbVal').val(pixel[0]+','+pixel[1]+','+pixel[2]);

        var dColor = pixel[2] + 256 * pixel[1] + 65536 * pixel[0];
        $('#hexVal').val('#' + ('0000' + dColor.toString(16)).substr(-6));
    });

    // click event handler
    $('#picker').click(function(e) { 
        let data = {'#color' : dColor};
        const JSONObj = JSON.parse(data);
        console.log(JSONObj);

        websocket.send(JSONObj);
    }); 
});

// Window Load Function
window.onload = function(event) {
    init();
}



