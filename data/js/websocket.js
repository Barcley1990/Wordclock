
let debug = true;

// commands
const COMMAND_RESET = 20;
const COMMAND_WEBSOCKET_RDY = 21
const COMMAND_SET_HSV = 95;
const COMMAND_SET_LEDPWROFF = 96;
const COMMAND_SET_LEDPWRON = 97;
const COMMAND_SET_ADPTV_BRIGHNTESS = 98;

// JSON Keys
const JSON_KEY_COMMAND = "CMD";
const JSON_KEY_AMBIENT = "Light";
const JSON_KEY_TIME = "Time";
const JSON_KEY_VERSION = "Version";
const JSON_KEY_LED_PWR_STATE = "PwrState";
const JSON_KEY_HSV = "HSV";


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
        const obj = {"CMD": COMMAND_WEBSOCKET_RDY};
        websocketSend(obj);
    };

    // On Close Handler
    websocket.onclose = function(event) {
        debugMessage("The connection with the websocket was closed (code " + event.code + ").", event);
    };

    // On Message Received Handler
    websocket.onmessage = function(event) {
        let obj = JSON.parse(event.data);
        
        debugMessage("WebSocket data arrived", obj);

        if(obj.hasOwnProperty("DEBUG_MSG")) {
            let msg = obj.DEBUG_MSG;
            var myTextArea = $('#console');
            myTextArea.val(myTextArea.val() + '\n'+msg);
        }

        if(obj.hasOwnProperty(JSON_KEY_VERSION)) {
            $("#id_version").text(obj.Version);
        }

        if(obj.hasOwnProperty(JSON_KEY_AMBIENT)) {
            $("#id_ambient").text(obj.Light);
        }

        if(obj.hasOwnProperty(JSON_KEY_LED_PWR_STATE)) {
            state = obj.PwrState;
            if(state===true) {
                $("#id_button_ledpwr").text("LEDs Off");
            } else {
                $("#id_button_ledpwr").text("LEDs On");
            }
        }

        if(obj.hasOwnProperty(JSON_KEY_TIME)) {
            obj.Time = new Date(obj.Time);
            $("#id_time").text(obj.Time);
        }

        if (typeof dark_color_picker !== 'undefined') {
            if(obj.hasOwnProperty(JSON_KEY_HSV)) {
                let hsvColor = obj.HSV;
                hsvColor = hsvColor.toString(16);
                dark_color_picker.color.fromHEX(hsvColor);
            }
        }
    };

    websocket.onerror = function(event) {
		debugMessage("An error occurred while connecting to the websocket.", event);
	};
}

/**
 * websocketSend
 */
function websocketSend(command) {
    obj = JSON.stringify(command)
	debugMessage("Websocket send data: '" + obj + "'");
	websocket.send(obj);
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