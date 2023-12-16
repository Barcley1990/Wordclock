let checkbox_adapt_brighntess = null;
let led_power_state = false;


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

    // Setup Checkbox
    $("#setting_adapt_brightness").click(function() {
        if ($('#setting_adapt_brightness').is(":checked")) {
            checkbox_adapt_brighntess = true;
        }
        else {
            checkbox_adapt_brighntess = false;
        }
        websocketSend(COMMAND_SET_ADPTV_BRIGHNTESS, checkbox_adapt_brighntess);
    });

});


