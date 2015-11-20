Pebble.addEventListener("ready", function() {
  console.log("PebbleKit JS ready!");
});

Pebble.addEventListener("showConfiguration", function() {
  Pebble.openURL("https://mlb406.github.io/CircularFace");
});

Pebble.addEventListener("webviewclosed", function(e) {
  var options = JSON.parse(decodeURIComponent(e.response));
  
  console.log(JSON.stringify(options));
  
  var bars_on = options.bars.on;
  var date_on = options.date.on;
  var date_format = options.date.format;
  
  var bars, date, format;
  
  if (bars_on == "true") {
    bars = 1;
  } else {
    bars = 0;
  }
  
  if (date_on == "true") {
    date = 1;
  } else {
    date = 0;
  }
  
  //check passed values
  if (date_format == "DDMMYY") { // 25/07/15
    format = 1;
  } else if (date_format == "MMDDYY") { // 07/25/15
    format = 2;
  } else if (date_format == "DDDDMMM") { // SAT 25 JUL
    format = 3;
  } else if (date_format == "DDDMMMD") { // SAT JUL 25
    format = 4;
  }
  
  var dict = {
    "KEY_BARS": bars,
    "KEY_DATE": date,
    "KEY_FORMAT": format
  };
  
});
