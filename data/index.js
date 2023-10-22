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

function initWebsocket() {
    debugMessage('Trying to open a WebSocket connection to ' + 'ws://' + window.location.hostname + ':81/');
    websocket = new WebSocket('ws://'+ window.location.hostname +':81/', ['arduino']);

    websocket.onopen = function(event) {
        debugMessage("The connection with the websocket has been established.", event);
    };

    websocket.onclose = function(event) {
        debugMessage("The connection with the websocket was closed (code " + event.code + ").", event);
    };

    websocket.onmessage = function(event) {
        var data = JSON.parse(event.data);
        
        debugMessage("WebSocket response arrived (command " + data.command + ").", data);

        if(data.command === "version") {
            $("#version").set("value", data.version);
        }

        if(data.command === "ldr") {
            $("#ldr").set("value", data.ldr);
        }

        if(data.command === "time") {
            
        }
    };

    websocket.onerror = function(event) {
		debugMessage("An error occurred while connecting to the websocket.", event);
	};
}

function nstr5(number) {
	return Math.round(number).toString().padStart(5, "0");
}

function nstr(number) {
	return Math.round(number).toString().padStart(3, "0");
}

function getPaddedString(string, maxStringLength) {
	return string.padEnd(maxStringLength, " ");
}

function websocketSend(command, addData = "") {
	var data = nstr(command) + addData + "999";
	debugMessage("Send data: '" + data + "'");
	websocket.send(data);
}

$(document).ready(function() {
    
    initWebsocket();

    $("#btnEspReset").on("click", function() {
        websocketSend(COMMAND_RESET);debugMessage("Send");
    });

    $("#btnLedPwr").click(function() {
        websocketSend(COMMAND_SET_LEDPWROFF);debugMessage("Send");
    });
});

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





