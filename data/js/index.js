/** 
 * Wordclock Java Script
*/

let debug = true;
let led_power_state = false;
let led_brightness = 50;

// commands
const COMMAND_RESET = 20;
const COMMAND_SET_HSV = 95;
const COMMAND_SET_LEDPWROFF = 96;
const COMMAND_SET_LEDPWRON = 97;

// JSON Keys
const JSON_KEY_AMBIENT = "Light";
const JSON_KEY_TIME = "Time";
const JSON_KEY_VERSION = "Version";
const JSON_KEY_LED_PWR_STATE = "PwrState";
const JSON_KEY_HSV = "HSV";


/**
 * JQuery Ready
 */
$(document).ready(function() {
    debugMessage("Document fully loaded")
    // Initialize websocket
    initWebsocket();

    // Setup Button Handler
    $("#id_button_reset").on("click", function() {
        websocketSend(COMMAND_RESET);
    });

    // Setup Button Handler
    $("#id_button_ledpwr").click(function() {
        if(led_power_state===false){
            websocketSend(COMMAND_SET_LEDPWROFF);
            led_power_state = true;
            $("#id_button_ledpwr").text("LEDs On");
        } else {
            websocketSend(COMMAND_SET_LEDPWRON);
            led_power_state = false;
            $("#id_button_ledpwr").text("LEDs Off");
        }
    });

    // Slider Handler
    $("#id_brightness").on("input", function() {
        var value = $(this).val();
        $("#id_slider_value").text(value);
        debugMessage("slider", value);
    });

    // Create Color Picker (Dark)
    let dark_color_picker = 
        new ColorPickerControl({
            useAlpha: false, 
            container: document.querySelector('.color-picker-dark-theme'), 
            theme: 'dark',
            color: {
                r: 30,
                g: 30,
                b: 30
            }
        });

    dark_color_picker.on('change', (color) =>  {
        debugMessage("Color Picker Dark:",light_color_picker.color.fromHSVa(color.h, color.s, color.v, color.a));
        $(".button").css("background-color", color.toHEX());
        $(".button").css("opacity", color.a / 255);
        websocketSend(COMMAND_SET_HSV, color.toHEX());
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
        let obj = JSON.parse(event.data);
        
        debugMessage("WebSocket data arrived", obj);

        if(obj.hasOwnProperty(JSON_KEY_VERSION)) {
            $("#id_version").text(obj.Version);
        }

        if(obj.hasOwnProperty(JSON_KEY_AMBIENT)) {
            $("#id_ambient").text(obj.Light);
        }

        if(obj.hasOwnProperty(JSON_KEY_TIME)) {
            obj.Time = new Date(obj.Time);
            $("#id_time").text(obj.Time);
        }
    };

    websocket.onerror = function(event) {
		debugMessage("An error occurred while connecting to the websocket.", event);
	};
}

/**
 * websocketSend
 */
function websocketSend(command, addData = "") {
	var data = nstr(command) + addData;
	debugMessage("Send data: '" + data + "'");
	websocket.send(data);
}


changeTheme = (e) => document.querySelector('.flip-container').dataset.flipped = (e.value == 'light');

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
 * JQuery Colorpicker function

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
 */
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


