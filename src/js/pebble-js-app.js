var mConfig = {};

Pebble.addEventListener("ready", function(e) {
  loadLocalData();
  returnConfigToPebble();
});

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL(mConfig.configureUrl);
});

Pebble.addEventListener("webviewclosed",
  function(e) {
    if (e.response) {
      var config = JSON.parse(e.response);
      saveLocalData(config);
      returnConfigToPebble();
    }
  }
);

function saveLocalData(config) {

  //console.log("loadLocalData() " + JSON.stringify(config));

  localStorage.setItem("bluetoothvibe", parseInt(config.bluetoothvibe)); 
  localStorage.setItem("hourlyvibe", parseInt(config.hourlyvibe)); 
  localStorage.setItem("flip", parseInt(config.flip));  
  localStorage.setItem("font", parseInt(config.font));  
  localStorage.setItem("invert", parseInt(config.invert));
  localStorage.setItem("blink", parseInt(config.blink)); 
  
  loadLocalData();

}
function loadLocalData() {
  
	mConfig.bluetoothvibe = parseInt(localStorage.getItem("bluetoothvibe"));
	mConfig.hourlyvibe = parseInt(localStorage.getItem("hourlyvibe"));
	mConfig.flip = parseInt(localStorage.getItem("flip"));
	mConfig.font = parseInt(localStorage.getItem("font"));
	mConfig.invert = parseInt(localStorage.getItem("invert"));
	mConfig.blink = parseInt(localStorage.getItem("blink"));
	mConfig.configureUrl = "http://www.themapman.com/pebblewatch/taller90-03.html";
	

	if(isNaN(mConfig.bluetoothvibe)) {
		mConfig.bluetoothvibe = 0;
	}
	if(isNaN(mConfig.hourlyvibe)) {
		mConfig.hourlyvibe = 0;
	}
	if(isNaN(mConfig.flip)) {
		mConfig.flip = 0;
	} 
	if(isNaN(mConfig.font)) {
		mConfig.font = 0;
	}
	if(isNaN(mConfig.invert)) {
		mConfig.invert = 0;
	}
	if(isNaN(mConfig.blink)) {
		mConfig.blink = 0;
	}

  //console.log("loadLocalData() " + JSON.stringify(mConfig));
}
function returnConfigToPebble() {
  //console.log("Configuration window returned: " + JSON.stringify(mConfig));
  Pebble.sendAppMessage({
    "bluetoothvibe":parseInt(mConfig.bluetoothvibe), 
    "hourlyvibe":parseInt(mConfig.hourlyvibe),
    "flip":parseInt(mConfig.flip),
    "font":parseInt(mConfig.font),
	"invert":parseInt(mConfig.invert),
	"blink":parseInt(mConfig.blink),
  });    
}