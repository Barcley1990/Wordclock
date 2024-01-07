

$(document).ready(function() {

    debugMessage("Document fully loaded")

    // Initialize websocket
    initWebsocket();

    $("#setting_adapt_brightness").prop('checked', ClockSettings.AdptvBrightness);
    $("#settings-slope").prop('value', ClockSettings.AdptvBrightnessA);
    $("#settings-offset").prop('value', ClockSettings.AdptvBrightnessB);
    if(ClockSettings.JSON_KEY_LED_PWR_STATE == false) {
        $("#id_button_ledpwr").text("LEDs On");
    } else {
        $("#id_button_ledpwr").text("LEDs Off");
    }

    // Setup Button Handler
    $("#id_button_reset").on("click", function() {
        const obj = {"CMD": COMMAND_RESET};
        websocketSend(obj);
    });
  
    // Setup Button Handler
    $("#id_button_ledpwr").click(function() {
        let obj = null;
        const state = this.textContent || this.innerText;
        if(state ==="LEDs Off"){
            $("#id_button_ledpwr").text("LEDs On");
            obj = {"CMD": COMMAND_SET_LEDPWRON};
        } else {
            $("#id_button_ledpwr").text("LEDs Off");
            obj = {"CMD": COMMAND_SET_LEDPWROFF};
        }
        websocketSend(obj);
    });

    // Setup Checkbox
    $("#setting_adapt_brightness").click(function() {
        let checkbox_adapt_brighntess = $('#setting_adapt_brightness').is(":checked")
        const obj = {AdptvBrightness: checkbox_adapt_brighntess};
        websocketSend(obj);
    });

    const input1 = document.getElementById("settings-slope");
    const input2 = document.getElementById("settings-offset");
    const button = document.getElementById("settings-save-btn");
    // Add an event listener for the input event
    input1.addEventListener("input", () => {
        if (input1.value.length > 0) {
            button.disabled = false;
        } else {
            button.disabled = true;
        }
    });
    input2.addEventListener("input", () => {
        if (input2.value.length > 0) {
            button.disabled = false;
        } else {
            button.disabled = true;
        }
    });

    $("#settings-save-btn").click(() => {
        button.disabled = true;
        const obj = {"AdptvBrightnessA": parseFloat(input1.value), "AdptvBrightnessB": parseInt(input2.value)};
        websocketSend(obj)
    });

});
