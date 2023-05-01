#include "version.h"

const char index_html[] PROGMEM = R"=====(
<HTML>
	<HEAD>
		<TITLE>Wordclock</TITLE>
        <h1>Wordclock</h1>
        <style>
		footer {
		  text-align: center;
		  padding: 3px;
		  background-color: DarkSalmon;
		  color: white;
		}
		</style>
	</HEAD>
<BODY style='background-color: #EEEEEE;'>
	<CENTER>
		<B>Hello World.... </B>
		<span id='test'>-</span>
		<p><button type='button' id='BTN_RESET_ESP'>
			Reset ESP
		</button></p>

	</CENTER>	
</BODY>

<footer>
  <a href="/update">Software Update</a>
  <p>Author: Tobias Nuss</p>
  <p>Software Version: 0.1.0</p>
</footer>

<script>
	var Socket;

	document.getElementById('BTN_RESET_ESP').addEventListener('click', button_reset_esp);

	function init() {
		Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
		Socket.onmessage = function(event) {
			processCommand(event);
		};
	}

	function processCommand(event) {
		document.getElementById('test').innerHTML = event.data;
		console.log(event);
	}

	function button_reset_esp() {
		Socket.send('btn_reset');
	}

	window.onload = function(event) {
		init();
	}
</script>
</HTML>
)=====";