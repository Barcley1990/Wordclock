#include "version.h"

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<HTML lang="en" dir="ltr">
	<HEAD>
		<TITLE>Wordclock</TITLE>
        
        <meta charset="utf-8">
	    <title>Wordclocl</title>
	    <link rel="stylesheet" href="/style.css">
	    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css"/>
	</HEAD>

	<BODY>
		<div>
			<div class="sidebar">
				<header>Wordclock</header>
				<ul>
				 <li><a href="/index"><i class="fas fa-qrcode"></i>Dashboard</a></li>
				 <li><a href="/update"><i class="fas fa-link"></i>Update</a></li>
				 <li><a href="/about"><i class="far fa-question-circle"></i>About</a></li>
				 <li><a href="/contact"><i class="far fa-envelope"></i>Contact</a></li>
				</ul>

				<a class="version">Version: 0.1.0</a>
			</div>

			<div class="main">
				<section>
					Wordclock main page. 
					<br><br> 
				</section>
				<p>Current RTC time:</p>
				<p class="state">Time: <span id="Time"></span> Uhr</p>
				<p>Current ambient brightness:</p>
				<p class="state">Brightness: <span id="Light"></span> Lux;</p>

				

				<div class="card-grid">
		            <div class="card">
		                <p class="card-title">Red Channel</p>
		                <p class="switch">
		                    <input type="range" onchange="updateSliderPWM(this)" id="slider1" min="0" max="100" step="1" value ="0" class="slider">
		                </p>
		                <p class="state">Brightness: <span id="sliderValue1"></span> &percnt;</p>
		            </div>
		            <div class="card">
		                <p class="card-title"> Blue Channel</p>
		                <p class="switch">
		                    <input type="range" onchange="updateSliderPWM(this)" id="slider2" min="0" max="100" step="1" value ="0" class="slider">
		                </p>
		                <p class="state">Brightness: <span id="sliderValue2"></span> &percnt;</p>
		            </div>
		            <div class="card">
		                <p class="card-title"> Green Channel</p>
		                <p class="switch">
		                    <input type="range" onchange="updateSliderPWM(this)" id="slider3" min="0" max="100" step="1" value ="0" class="slider">
		                </p>
		                <p class="state">Brightness: <span id="sliderValue3"></span> &percnt;</p>
		            </div>
		        </div>
				




				<div>
					<button class="button" type='button' id='#btn_rst_sldr'>Reset Settings</button>
				</div>
				<div>
					<button class="button" type='button' id='#btn_led_pwr'>Turn Power Off</button>
				</div>
				<br><br> 
				<div>
					<button class="button" type='button' id='#btn_esp_rst'>Reset ESP</button>
				</div>
			</div>
		</div>
	</BODY>

	<script>
		var gateway = `ws://${window.location.hostname}`;
		var websocket;

		document.getElementById('#btn_esp_rst').addEventListener('click', function() {
			console.log('button clicked');
			websocket.send('#esp_reset');
		});
		document.getElementById('#btn_led_pwr').addEventListener('click', function() {
			console.log('button clicked');
			websocket.send('#pwr_on_off');
		});
		document.getElementById('#btn_rst_sldr').addEventListener('click', function() {
			console.log('button clicked');
		});


		

		function init() {
			console.log('Trying to open a WebSocket connection...')
			websocket = new WebSocket(gateway + ':81/');

			websocket.onopen = function() {
				console('Open connection');
			};
  			websocket.onclose = function() {
  				log.console('Close connection');
  			};

			websocket.onmessage = function(event) {
				console.log(event.data);
			    var JSONObj = JSON.parse(event.data);
			    var keys = Object.keys(JSONObj);

			    for (var i = 0; i < keys.length; i++){
			        var key = keys[i];
			        if(key == 'Light') {
			        	document.getElementById('Light').innerHTML = JSONObj[key];
			        }
			        else if(key == 'Time') {
			        	document.getElementById('Time').innerHTML = JSONObj[key];
			        }
			        else {
			        	document.getElementById(key).innerHTML = JSONObj[key];
			        	document.getElementById("slider"+ (i+1).toString()).value = JSONObj[key];
			        } 
			    }
			};
		}

	

		function updateSliderPWM(element) {
		    var sliderNumber = element.id.charAt(element.id.length-1);
		    var sliderValue = document.getElementById(element.id).value;
		    document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
		    console.log(sliderValue);
		    websocket.send(sliderNumber+"s"+sliderValue.toString());
		}



		window.onload = function(event) {
			init();
		}

	

		
	</script>
</HTML>



)=====";