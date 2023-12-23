let checkbox_adapt_brighntess = null;
let led_power_state = false;


$(document).ready(function() {

    debugMessage("Document fully loaded")

    // Initialize websocket
    initWebsocket();

    // Setup Button Handler
    $("#id_button_reset").on("click", function() {
        const obj = {"CMD": COMMAND_RESET};
        websocketSend(obj);
    });
  
    // Setup Button Handler
    $("#id_button_ledpwr").click(function() {
        let obj = null;
        if(led_power_state===false){
            led_power_state = true;
            $("#id_button_ledpwr").text("LEDs On");
            obj = {"CMD": COMMAND_SET_LEDPWRON};
        } else {
            led_power_state = false;
            $("#id_button_ledpwr").text("LEDs Off");
            obj = {"CMD": COMMAND_SET_LEDPWROFF};
        }
        websocketSend(obj);
    });

    // Setup Checkbox
    $("#setting_adapt_brightness").click(function() {
        if ($('#setting_adapt_brightness').is(":checked")) {
            checkbox_adapt_brighntess = true;
        }
        else {
            checkbox_adapt_brighntess = false;
        }
        const obj = {AdptvBrightness: checkbox_adapt_brighntess};
        websocketSend(obj);
    });

});
