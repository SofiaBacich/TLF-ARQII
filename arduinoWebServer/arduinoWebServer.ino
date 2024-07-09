#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char* ssid = "AndroidAP0137";
const char* password = "wendy12345";

ESP8266WebServer server(80);

String scores = "";
String input = "";
bool receivedData = false;

const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Scores del PONG</title>
</head>
<body style="background-color: #f9e79f ">
<center>
<div>
<h1>Scores del PONG</h1>
</div>
 <br>
<div><h2>
  <span id="scoresData"></span><br><br>
</h2>
</div>
<script>
setInterval(function() 
{
  getData();
}, 2000); 
function getData() {
  var ajax = new XMLHttpRequest();
  ajax.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("scoresData").innerHTML =
      this.responseText;
    }
  };
  ajax.open("GET", "dataRead", true);
  ajax.send();
}
</script>
</center>
</body>
</html>
)=====";

void scoresData(){
    server.send(200, "text/plane", scores);
}

void handleRoot() 
{
 String s = webpage;
 server.send(200, "text/html", s);
}

void setup(void)
{
  Serial.begin(115200);
  input.reserve(200);
  
  WiFi.begin(ssid, password);
  Serial.println("");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting...");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/dataRead", scoresData);
  server.begin();
}


void loop(void)

{
  server.handleClient();
  if(receivedData){
    Serial.print(input);
    scores = scores + "Player Score: " + input + "<br>";
    scoresData();
    receivedData = false;
    input = "";
  }
  delay(2000);
}

void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    if(inChar == '_'){
      receivedData = true;
    }else{
      input += inChar;
    }
  }
}
