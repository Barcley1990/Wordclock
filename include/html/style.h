


const char style_css[] PROGMEM = R"=====(
@import url('https://fonts.googleapis.com/css?family=Roboto:300,400,400i,500');
*{
  padding: 0;
  margin: 0;
  list-style: none;
  text-decoration: none;
}
body {
  font-family: 'Roboto', sans-serif;
}
.sidebar {
  position: fixed;
  left: 0px;
  width: 250px;
  height: 100%;
  background: #042331;
  transition: all .5s ease;
}
.sidebar header {
  font-size: 22px;
  color: white;
  line-height: 70px;
  text-align: center;
  background: #063146;
  user-select: none;
}
.sidebar ul a{
  display: block;
  height: 100%;
  width: 100%;
  line-height: 65px;
  font-size: 20px;
  color: white;
  padding-left: 40px;
  box-sizing: border-box;
  border-bottom: 1px solid black;
  border-top: 1px solid rgba(255,255,255,.1);
  transition: .4s;
}
ul li:hover a{
  padding-left: 50px;
}
.sidebar ul a i{
  margin-right: 16px;
}
.version {
  color: white;
  position: absolute;
  bottom: 10px;
  padding-left: 10px;
}
.button {
  background-color: #4CAF50; /* Green */
  border: none;
  color: white;
  padding: 15px 32px;
  margin: 10px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
}
.card-grid {
    max-width: 700px;
    margin: 30px;
    display: grid;
    grid-gap: 2rem;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  }
  .card {
    background-color: darkgrey;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
  }
  .card-title {
    font-size: 1.2rem;
    font-weight: bold;
    color: #034078
  }
  .state {
    font-size: 1.2rem;
    color:#1282A2;
  }
  .slider {
    -webkit-appearance: none;
    margin: 0 auto;
    width: 100%;
    height: 15px;
    border-radius: 10px;
    background: #FFD65C;
    outline: none;
  }
  .slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 30px;
    height: 30px;
    border-radius: 50%;
    background: #034078;
    cursor: pointer;
  }
  .slider::-moz-range-thumb {
    width: 30px;
    height: 30px;
    border-radius: 50% ;
    background: #034078;
    cursor: pointer;
  }
  .switch {
    padding-left: 5%;
    padding-right: 5%;
  }
.main{
  background: #063146;
  background-position: center;
  background-size: cover;
  margin-left: 250px;
  padding: 10px;
  height: 100vh;
  transition: all .5s;
  color: white;
}


)=====";