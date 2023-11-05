/** 
 * Wordclock Java Script
*/


var buttonState = false;
var debug = true;
var colorPicker;

// commands
var COMMAND_RESET = 20;
var COMMAND_SET_BRIGHTNESS = 95;
var COMMAND_SET_LEDPWROFF = 96;
var COMMAND_SET_LEDPWRON = 97;


/**
 * Function definitions
 */
/* eslint-disable no-console */
function debugMessage(debugMessage, someObject) {
	if (debug === true) {

		if (console !== undefined) {
			if (someObject) {
				console.log(debugMessage, someObject);
			} else {
				console.log(debugMessage);
			}
		}

		$("#output").text("debugMessage");
	}
}

/**
 * initWebsocket
 */
function initWebsocket() {
    debugMessage('Trying to open a WebSocket connection to ' + 'ws://' + window.location.hostname + ':81/');
    websocket = new WebSocket('ws://'+ window.location.hostname +':81/', ['arduino']);

    // On Open Handler
    websocket.onopen = function(event) {
        debugMessage("The connection with the websocket has been established.", event);
    };

    // On Close Handler
    websocket.onclose = function(event) {
        debugMessage("The connection with the websocket was closed (code " + event.code + ").", event);
    };

    // On Message Received Handler
    websocket.onmessage = function(event) {
        var obj = JSON.parse(event.data);
        
        debugMessage("WebSocket response arrived (command " + obj.command + ").", obj);

        if(obj.hasOwnProperty('version')) {
            $("#id_version").set("value", obj.version);
        }

        if(obj.hasOwnProperty("ldr")) {
            $("#id_ldr").set("value", obj.ldr);
        }

        if(obj.hasOwnProperty('time')) {
            obj.time = new Date(obj.time);
            $("#id_time").set("value", obj.time);
        }
    };

    websocket.onerror = function(event) {
		debugMessage("An error occurred while connecting to the websocket.", event);
	};
}

/**
 * nstr5
  * @param {*} number 
 * Convert integer to string object. Padding given String 
 * to 5 characters.
 */
function nstr5(number) {
	return Math.round(number).toString().padStart(5, "0");
}

/**
 * nstr
 * @param {*} number 
 * Convert integer to string object. Padding given String 
 * to 3 characters.
 */
function nstr(number) {
	return Math.round(number).toString().padStart(3, "0");
}

/**
 * getPaddedString
 */
function getPaddedString(string, maxStringLength) {
	return string.padEnd(maxStringLength, " ");
}

/**
 * websocketSend
 */
function websocketSend(command, addData = "") {
	var data = nstr(command) + addData + "999";
	debugMessage("Send data: '" + data + "'");
	websocket.send(data);
}

/**
 * JQuery Ready
 */
$(document).ready(function() {
    
    // Initialize websocket
    initWebsocket();

    // Setup Button Handler
    $("#btnEspReset").on("click", function() {
        websocketSend(COMMAND_RESET);
        debugMessage("Send");
    });

    // Setup Button Handler
    $("#btnLedPwr").click(function() {
        if(buttonState===false){
            websocketSend(COMMAND_SET_LEDPWROFF);
            buttonState = true;
        } else {
            websocketSend(COMMAND_SET_LEDPWRON);
            buttonState = false;
        }
        debugMessage("Send");
    });
});

/**
 * JQuery Colorpicker function
 */
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
        colorPicker = dColor;
        $('#hexVal').val('#' + ('0000' + dColor.toString(16)).substr(-6));
    });

    // click event handler
    $('#picker').click(function(e) { 
        var data = {'#color' : colorPicker};
        if (websocket.readyState !== WebSocket.CLOSED) {
            websocket.send(data);
        }
        else {
            console.log("Websocket not ready yet");
        }
    }); 
});





