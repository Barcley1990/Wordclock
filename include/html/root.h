
const char* PARAM_INPUT_1 = "input_ssid";
const char* PARAM_INPUT_2 = "input_password";

const char index_html[] = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
        <title>Wordclock</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>\
            body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 1.5em; Color: #000000; }
            h1 { Color: #AA0000; }
        </style>\
    </head>
    <body>
        <form action="/get">
            input_ssid: <input type="text" name="input_ssid">
            <input type="submit" value="Submit">
        </form><br>
        <form action="/get">
            input_password: <input type="text" name="input_password">
            <input type="submit" value="Submit">
        </form><br>
    </body>
</html>)rawliteral";