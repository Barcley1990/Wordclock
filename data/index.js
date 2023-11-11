/** 
 * Wordclock Java Script
*/

var debug = true;
var led_power_state = false;
var led_brightness = 0;
var color_picker;

// commands
var COMMAND_RESET = 20;
var COMMAND_SET_BRIGHTNESS = 95;
var COMMAND_SET_LEDPWROFF = 96;
var COMMAND_SET_LEDPWRON = 97;

// JSON Keys
const JSON_KEY_AMBIENT = "Light";
const JSON_KEY_TIME = "Time";
const JSON_KEY_VERSION = "Version";
const JSON_KEY_LED_PWR_STATE = "PwrState";
const JSON_KEY_BRIGHTNESS = "Brightness";


/**
 * JQuery Ready
 */
$(document).ready(function() {
    debugMessage("Document fully loaded")
    // Initialize websocket
    initWebsocket();
    colorPicker();

    // Setup Button Handler
    $("#id_button_reset").on("click", function() {
        websocketSend(COMMAND_RESET);
    });

    // Setup Button Handler
    $("#id_button_ledpwr").click(function() {
        if(buttonState===false){
            websocketSend(COMMAND_SET_LEDPWROFF);
            buttonState = true;
        } else {
            websocketSend(COMMAND_SET_LEDPWRON);
            buttonState = false;
        }
    });

    // Slider Handler
    $("#id_brightness").on("input", function() {
        var value = $(this).val();
        $("#id_slider_value").text(value);
        debugMessage("slider", value);
    });
});


/**
 * initWebsocket
 */
function initWebsocket() {
    const HOST = window.location.hostname;

    debugMessage('Trying to open a WebSocket connection to ' + 'ws://' + HOST + ':81/');
    websocket = new WebSocket('ws://'+ HOST +':81/', ['arduino']);

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
        
        debugMessage("WebSocket data arrived", obj);

        if(obj.hasOwnProperty(JSON_KEY_VERSION)) {
            $("#id_version").text(obj.version);
        }

        if(obj.hasOwnProperty(JSON_KEY_AMBIENT)) {
            $("#id_ldr").text(obj.Light);
        }

        if(obj.hasOwnProperty(JSON_KEY_TIME)) {
            obj.time = new Date(obj.time);
            $("#id_time").text(obj.Time);
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
 * JQuery Colorpicker function
 */
function colorPicker(){

    debugMessage("Initialize Color Picker")

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
        color_picker = dColor;
        $('#hexVal').val('#' + ('0000' + dColor.toString(16)).substr(-6));
    });

    // click event handler
    $('#picker').click(function(e) { 
        var data = {'#color' : color_picker};
        if (websocket.readyState !== WebSocket.CLOSED) {
            websocket.send(data);
        }
        else {
            console.log("Websocket not ready yet");
        }
    }); 
}

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


