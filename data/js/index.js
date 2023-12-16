/** 
 * Wordclock Java Script
*/

let led_brightness = 50;
let dark_color_picker = null;

/**
 * JQuery Ready
 */
$(document).ready(function() {
    
    debugMessage("Document fully loaded")

    // Initialize websocket
    initWebsocket();

    // Create Color Picker (Dark)
    dark_color_picker = 
        new ColorPickerControl({
            useAlpha: false, 
            container: document.querySelector('.color-picker-dark-theme'), 
            theme: 'dark',
            color: {
                r: 1,
                g: 1,
                b: 1
            }
        });

    // Setup On-Change Handler
    dark_color_picker.on('change', (color) =>  {
        debugMessage("Color Picker Dark:",dark_color_picker.color.fromHSVa(color.h, color.s, color.v, color.a));
        $(".button").css("background-color", color.toHEX());
        $(".button").css("opacity", color.a / 255);
        let hsvPacked = ((color.h << 16) | (color.s << 8) | (color.v)) & 0xFFFFFF;
        debugMessage("hsvPacked: ", hsvPacked.toString(16));
        //websocketSend(COMMAND_SET_HSV, color.toHEX());
        websocketSend(COMMAND_SET_HSV, "#" + hsvPacked.toString(16));
    });
});

changeTheme = (e) => document.querySelector('.flip-container').dataset.flipped = (e.value == 'light');



