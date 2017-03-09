
#define CAGE_LIGHT_VERSION "24b"
#define _RB_DNS_DEBUG


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#include <WiFiUdp.h>
#include <Wire.h>
#include <EEPROM.h>

//CONFIG ----------------------------------------
#define AMOUNT_OUTPUTS 2
const int switch_0_pin = 4;
const int switch_1_pin = 5;
const int  i2c_scl_pin = 4;
const int i2c_sda_pin = 5;

const int output_relais_pins[2] = {14,12};
int output_relais_states[2] = { 0 };
const bool intert_outputs = true;
#define WEBSERVER_PORT 80 //acess
#define DS1307_ADRESSE 0x68 // i2c adress of the rtc
#define WEBSITE_TITLE "CAGE LIGHT"
#define SERIAL_BAUD_RATE 115200
#define MDNS_NAME "cagelight" //for eg abc.local...
//EDIT YOUR ACCESS POINTS HERE
void setup_wifi(){
    /* SSID NO !"§...  like FRITZ!BOX -> FRITZBOX*/
    wifiMulti.addAP("test_wifi", "213546");
    wifiMulti.addAP("FRITZ!Box Fon WLAN 7390", "6226054527192856");

  }



/* DNS SERVER HOSTES BY ME  
 *  
 *  
 * 
 */  
#define RB_DNS //USE THE RB DNS SERVICE
const String RB_DNS_HOST_BASE_URL = "http://109.230.230.209:80/rb_dns_server/update.php";
//PLEASE USE
#if defined(_RB_DNS_DEBUG)
const String RB_DNS_UUID = "b10c5911-1234-1234-1234-98994d256e76";
#else
const String RB_DNS_UUID = "00000000-0000-0000-0000-000000000000"; //CAHNGE THIS <-------------
#endif
#define RB_DNS_PASSWORD "1234" //change this <-------------------
#define RB_DNS_ACCESS_PORT WEBSERVER_PORT
#define RB_DNS_DEVICE_NAME "CAGE_LIGHT_GITHUB" //<----- change this to your individual username
// END CONFIG ---------------------------------
bool rb_dns_conf_correct = true;




//FUNC DEC
byte decToBcd(byte val);
byte bcdToDec(byte val);
void set_time_to_rtc();
void get_time_from_rtc();

//VARS
ESP8266WebServer server ( WEBSERVER_PORT );
//TIME SEKUNDE
int sekunde, minute, stunde, tag, wochentag, monat, jahr, tag_index;
const String wochentage[7] = { "Sunday", "Monday", "Thuesday", "Wednesday", "Thirstday", "Friday", "Saturday"};
int on_off_times[7][2] = { {8,22} };
bool on_off_enabled = true;
bool on_time_switched = false; //FOR SCHEDULE
bool off_time_switched = false;

const String phead_1 = "<!DOCTYPE html><html><head><title>";
const String phead_2 = "</title>"
  "<meta http-equiv='content-type' content='text/html; charset=utf-8'>"
  "<meta charset='utf-8'>"
  "<link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/themes/base/jquery-ui.css' rel=stylesheet />"
  "<script src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js'></script>"
  "<script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/jquery-ui.min.js'></script>"
  "<style> body {  } #red, #green, #blue { margin: 10px; } #red { background: #f00; } #green { background: #0f0; } #blue { background: #00f; } </style>"
  "<script>"
  "function changeRGB(event, ui) { jQuery.ajaxSetup({timeout: 110}); /*not to DDoS the Arduino, you might have to change this to some threshold value that fits your setup*/ var id = $(this).attr('id'); if (id == 'red') $.post('/rgb', { red: ui.value } ); if (id == 'green') $.post('/rgb', { green: ui.value } ); if (id == 'blue') $.post('/rgb', { blue: ui.value } ); } "
  "$(document).ready(function(){ $('#red, #green, #blue').slider({min: 0, max:255, change:changeRGB, slide:changeRGB}); });"
  "</script>"
  "<style>"
  "html, body {"
  "  background: #F2F2F2;"
  " width: 100%;"
  " height: 100%;"
  " margin: 0px;"
  " padding: 0px;"
  " font-family: 'Verdana';"
  " font-size: 16px;"
  " color: #404040;"
  " }"
  "img {"
  " border: 0px;"
  "}"
  "span.title {"
  " display: block;"
  " color: #000000;"
  " font-size: 30px;"
  "}"
  "span.subtitle {"
  " display: block;"
  " color: #000000;"
  " font-size: 20px;"
  "}"
  ".sidebar {"
  " background: #FFFFFF;"
  " width: 250px;"
  " min-height: 100%;"
  " height: 100%;"
  " height: auto;"
  " position: fixed;"
  " top: 0px;"
  " left: 0px;"
  " border-right: 1px solid #D8D8D8;"
  "}"
  ".logo {"
  " padding: 25px;"
  " text-align: center;"
  " border-bottom: 1px solid #D8D8D8;"
  "}"
  ".menu {"
  " padding: 25px 0px 25px 0px;"
  " border-bottom: 1px solid #D8D8D8;"
  "}"
  ".menu a {"
  " padding: 15px 25px 15px 25px;"
  " display: block;"
  " color: #000000;"
  " text-decoration: none;"
  " transition: all 0.25s;"
  "}"
  ".menu a:hover {"
  " background: #0088CC;"
  " color: #FFFFFF;"
  "}"
  ".right {"
  " margin-left: 250px;"
  " padding: 50px;"
  "}"
  ".content {"
  " background: #FFFFFF;"
  " padding: 25px;"
  " border-radius: 5px;"
  " border: 1px solid #D8D8D8;"
  "}"
  "</style>";

const String pstart = "</head>"
"<body style='font-size:62.5%;'>"
"<div class='sidebar'>"
"<div class='logo'>"
"<span class='title'>CAGE_LIGHT</span>"
"<span class='subtitle'>- Backend -</span>"
"</div>"
"<div class='menu'>"
"<a href='index.html'>Dashboard</a>"
"</div>"
"</div>"
"<div class='right'>"
"<div class='content'>";

const String pend = "</div>"
"</div>"
"</body>"
"</html>";


void switch_channel(int _chid, bool _val){
  output_relais_states[_chid] = _val;
    if(intert_outputs){ 
        digitalWrite(output_relais_pins[_chid], !_val);
    }else{ 
        digitalWrite(output_relais_pins[_chid], _val);
    }
  
    save_values_to_eeprom();
  }  
void switch_all_on(){
   for(int i = 0; i < AMOUNT_OUTPUTS; i++){
    switch_channel(i, true);
    }
}
void switch_all_off(){
  for(int i = 0; i < AMOUNT_OUTPUTS; i++){
    switch_channel(i, false);
    }
    
}

void restore_eeprom_values(){
  //READ SCHEDULE TIMES
  int ei = 0;
  on_off_times[0][0] = EEPROM.read(ei++);
  on_off_times[0][1] = EEPROM.read(ei++);
  on_off_times[1][0] = EEPROM.read(ei++);
  on_off_times[1][1] = EEPROM.read(ei++);
  on_off_times[2][0] = EEPROM.read(ei++);
  on_off_times[2][1] = EEPROM.read(ei++);
  on_off_times[3][0] = EEPROM.read(ei++);
  on_off_times[3][1] = EEPROM.read(ei++);
  on_off_times[4][0] = EEPROM.read(ei++);
  on_off_times[4][1] = EEPROM.read(ei++);
  on_off_times[5][0] = EEPROM.read(ei++);
  on_off_times[5][1] = EEPROM.read(ei++);
  on_off_times[6][0] = EEPROM.read(ei++);
  on_off_times[6][1] = EEPROM.read(ei++);
  on_off_enabled = EEPROM.read(ei++);

   for(int i = 0; i < AMOUNT_OUTPUTS; i++){
   output_relais_states[i] = EEPROM.read(ei++);
    }
    
  }
void save_values_to_eeprom(){
   int ei = 0;
   //TODO DO LOOP HERE
     EEPROM.write(ei++, (byte)on_off_times[0][0]);
     EEPROM.write(ei++, (byte)on_off_times[0][1]);
     EEPROM.write(ei++, (byte)on_off_times[1][0]);
     EEPROM.write(ei++, (byte)on_off_times[1][1]);
     EEPROM.write(ei++, (byte)on_off_times[2][0]);
     EEPROM.write(ei++, (byte)on_off_times[2][1]);
     EEPROM.write(ei++, (byte)on_off_times[3][0]);
     EEPROM.write(ei++, (byte)on_off_times[3][1]);
     EEPROM.write(ei++, (byte)on_off_times[4][0]);
     EEPROM.write(ei++, (byte)on_off_times[4][1]);
     EEPROM.write(ei++, (byte)on_off_times[5][0]);
     EEPROM.write(ei++, (byte)on_off_times[5][1]);
     EEPROM.write(ei++, (byte)on_off_times[6][0]);
     EEPROM.write(ei++, (byte)on_off_times[6][1]);
     EEPROM.write(ei++, on_off_enabled);
     //SAVE LIGHT STATES
       for(int i = 0; i < AMOUNT_OUTPUTS; i++){
    EEPROM.write(ei++,output_relais_states[i]);
    }
     EEPROM.commit();
     Serial.println("eeprom write");
  }

void handleRoot() {

String time_message = "TIME : ";
time_message += "<form name = 'btn_time_set' action = '/' method = 'GET'>";
time_message += "Current Time : <input type='number' name='date_d' min='0' max='31' value='" + String(tag) + "'/>.<input type='number' name='date_m' min='0' max='12' value='" + String(monat) + "'/>.<input type='number' name='date_y' min='0' max='99' value='" + String(jahr) + "'/> (" + wochentage[wochentag] +") <input type='number' name='time_h' min='0' max='23' value='" + String(stunde) + "'/>:<input type='number' name='time_m' min='0' max='59' value='" + String(minute) + "'/>:<input type='number' name='time_s' min='0' max='59' value='" + String(sekunde) + "'/><br /><input type='submit' value='SAVE TIME'/>";
time_message += "</form>";



String control_forms = "<hr><h2>CONTROLS</h2>";


//ADD MAIN BUTTONS
control_forms += "<br><h3> MAIN CONTROLS </h3>";
control_forms += "<form name='btn_on' action='/' method='GET'>"
"<input type='hidden' value='all_on' name='ls' />"
"<input type='submit' value='ALL ON'/>"
"</form>";
control_forms += "<br>";
control_forms += "<form name='btn_off' action='/' method='GET'>"
"<input type='hidden' value='all_off' name='ls' />"
"<input type='submit' value='ALL OFF'/>"
"</form>";


for(int i = 0; i < AMOUNT_OUTPUTS;i++){
  control_forms += "<br><h3> CHANNEL " + String(i) + " CONTROL  </h3>";

if(output_relais_states[i]){
  control_forms += "<form name='btn_ofsf' action='/' method='GET'>"
    "<input type='hidden' value='" + String(i) + "_off' name='ls' />"
    "<input type='submit' value='"+ String(i) + " OFF'/>"
    "</form>";
}else{
   control_forms += "<form name='btn_osaff' action='/' method='GET'>"
    "<input type='hidden' value='" + String(i) + "_on' name='ls' />"
    "<input type='submit' value='" + String(i) +" ON'/>"
    "</form>";
}
  
  }



control_forms += "<br><h3> SCHEDULE  </h3>";
if (on_off_enabled) {
  control_forms += "<form name='btn_sched_tab' action='/' method='GET'>";
  control_forms += "<table><tr><th> DAY </th><th> ON TIME </th><th> OFF TIME </th></tr>";
  control_forms += "<tr><td>SUNDAY</td><td><input type='number' min='0' max='23' name='set_sched_sun_on' value='" + String(on_off_times[0][0]) + "' /></td><td><input type='number' min='0' max='23' name='set_sched_sun_off' value='" + String(on_off_times[0][1]) + "' /></td></tr>";
  control_forms += "<tr><td>MONDAY</td><td><input type='number' min='0' max='23' name='set_sched_mon_on' value='" + String(on_off_times[1][0]) + "'/></td><td><input type='number' min='0' max='23' name='set_sched_mon_off' value='" + String(on_off_times[1][1]) + "'/></td></tr>";
  control_forms += "<tr><td>TUESDAY</td><td><input type='number' min='0' max='23' name='set_sched_tue_on' value='" + String(on_off_times[2][0]) + "'/></td><td><input type='number' min='0' max='23' name='set_sched_tue_off' value='" + String(on_off_times[2][1]) + "'/></td></tr>";
  control_forms += "<tr><td>WEDNESDAY</td><td><input type='number' min='0' max='23' name='set_sched_wed_on' value='" + String(on_off_times[3][0]) + "'/></td><td><input type='number' min='0' max='23' name='set_sched_wed_off' value='" + String(on_off_times[3][1]) + "'/></td></tr>";
  control_forms += "<tr><td>THURSTDAY</td><td><input type='number' min='0' max='23' name='set_sched_thu_on' value='" + String(on_off_times[4][0]) + "'/></td><td><input type='number' min='0' max='23' name='set_sched_thu_off' value='" + String(on_off_times[4][1]) + "'/></td></tr>";
  control_forms += "<tr><td>FRIDAY</td><td><input type='number' min='0' max='23' name='set_sched_fri_on' value='" + String(on_off_times[5][0]) + "'/></td><td><input type='number' min='0' max='23' name='set_sched_fri_off' value='" + String(on_off_times[5][1]) + "'/></td></tr>";
  control_forms += "<tr><td>SATURDAY</td><td><input type='number' min='0' max='23' name='set_sched_sat_on' value='" + String(on_off_times[6][0]) + "'/></td><td><input type='number' min='0' max='23' name='set_sched_sat_off' value='" + String(on_off_times[6][1]) + "'/></td></tr>";
  control_forms += "</table><br><input type = 'submit' value = 'SAVE TIMES' / ></form><br>";

  control_forms += "<form name='btn_sch_on' action='/' method='GET'>"
    "<input type='hidden' value='sched_disable' name='sched_disable' />"
    "<input type='submit' value='DISABLE SCHEDULE'/>"
    "</form>";
}
else {
  control_forms += "<br><form name='btn_sch_on' action='/' method='GET'>"
    "<input type='hidden' value='sched_enable' name='sched_enable' />"
    "<input type='submit' value='ENABLE SCHEDULE'/>"
    "</form>";
}



String api_calls = "<hr><h2>CONFIGURATION API</h2><br><br><table>"
"<tr>"
"<th>PARAMETER</th>"
"<th>VALUE</th>"
"<th>DESCRIPTION</th>"
"</tr>"
"<tr>"
"<td>ls</td>"
"<td>all_on</td>"
"<td>Set all channels on</td>"
"</tr>"
"<tr>"
"<td>ls</td>"
"<td>all_off</td>"
"<td>Set all channels off</td>"
"</tr>"
"<tr>"
"<td>ls</td>"
"<td>#channelid#_on</td>"
"<td>Set channel one on</td>"
"</tr>"
"<tr>"
"<td>ls</td>"
"<td>#channelid#_off</td>"
"<td>Set channel one off</td>"
"</tr>"
"<tr>"
"<td>time_s</td>"
"<td>&lt; seconds&gt; </td>"
"<td>Set the seconds of the clock</td>"
"</tr>"
"<tr>"
"<td>time_m</td>"
"<td>&lt; minute&gt; </td>"
"<td>Set the minutes of the clock</td>"
"</tr>"
"<tr>"
"<td>time_h</td>"
"<td>&lt; hours&gt; </td>"
"<td>Set the hour of the clock</td>"
"</tr>"
"<tr>"
"<td>date_d</td>"
"<td>&lt; day&gt; </td>"
"<td>Set the current day of the date</td>"
"</tr>"
"<tr>"
"<td>date_m</td>"
"<td>&lt; month&gt; </td>"
"<td>Set the current month of the date</td>"
"</tr>"
"<tr>"
"<td>date_y</td>"
"<td>&lt; year&gt; </td>"
"<td>Set the current year of the date<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_mon_on</td>"
"<td>&lt; hour &gt</td>"
"<td>Set the light on hour at monday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_tue_on</td>"
"<td>&lt; hour &gt</td>"
"<td>Set the light on hour at tuesday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_wed_on</td>"
"<td>&lt; hour &gt</td>"
"<td>Set the light on hour at wednesday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_thu_on</td>"
"<td>&lt; hour &gt</td>"
"<td>Set the light on hour at thuesday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_fri_on</td>"
"<td>&lt; hour&gt </td>"
"<td>Set the light on hour at friday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_sat_on</td>"
"<td>&lt; hour &gt</td>"
"<td>Set the light on hour at saturday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_sun_on</td>"
"<td>&lt; hour&gt </td>"
"<td>Set the light on hour at sunday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_mon_off</td>"
"<td>&lt; hour &gt</td>"
"<td>Set the light off hour at monday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_tue_off</td>"
"<td>&lt; hour&gt </td>"
"<td>Set the light off hour at tuesday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_wed_off</td>"
"<td>&lt; hour&gt </td>"
"<td>Set the light off hour at wednesday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_thu_off</td>"
"<td>&lt; hour&gt </td>"
"<td>Set the light off hour at thuesday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_fri_of</td>"
"<td>&lt; hour &gt</td>"
"<td>Set the light off hour at friday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_sat_off</td>"
"<td>&lt; hour&gt </td>"
"<td>Set the light off hour at saturday<br></td>"
"</tr>"
"<tr>"
"<td>set_sched_sun_off</td>"
"<td>&lt; hour&gt </td>"
"<td>Set the light off hour at sunday<br></td>"
"</tr>"
"<tr>"
"<td>sched_enable</td>"
"<td> </td>"
"<td>Enabled the timer<br></td>"
"</tr>"
"<tr>"
"<td>sched_disable</td>"
"<td> </td>"
"<td>Disable the timer<br></td>"
"</tr>"
"</table>";





String msg = "";
msg = phead_1 + WEBSITE_TITLE + phead_2 + pstart + time_message + control_forms +api_calls + pend;


  String message = "";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
Serial.println(message);
volatile bool was_time_changed = false;
volatile bool was_timer_changes = false;
   for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
   if(server.argName(i) == "ls" && server.arg(i) == "all_on"){
switch_all_on();
   }
      else if(server.argName(i) == "ls" && server.arg(i) == "all_off"){
switch_all_off();
   }else{
  



for(int ic = 0; ic < AMOUNT_OUTPUTS;ic++){
      if(server.arg(i) == String(ic) + "_on"){
switch_channel(ic,true);
   }
}
for(int ic = 0; ic < AMOUNT_OUTPUTS;ic++){
   if(server.arg(i) == String(ic) + "_off"){
switch_channel(ic,false);
   }
}

}

  
  
  





    //SET ITME VARS
     if (server.argName(i) == "time_s") {
       sekunde = server.arg(i).toInt();
       was_time_changed = true;
     }
     if (server.argName(i) == "time_m") {
       minute = server.arg(i).toInt();
       was_time_changed = true;
     }
     if (server.argName(i) == "time_h") {
       stunde = server.arg(i).toInt();
       was_time_changed = true;
     }
     if (server.argName(i) == "date_d") {
       tag = server.arg(i).toInt();
       was_time_changed = true;
     }
     if (server.argName(i) == "date_m") {
       monat = server.arg(i).toInt();
       was_time_changed = true;
     }
     if (server.argName(i) == "date_y") {
       jahr = server.arg(i).toInt();
       was_time_changed = true;
     }
     
     if (server.argName(i) == "set_sched_sun_on") {
       on_off_times[0][0] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_sun_off") {
       on_off_times[0][1] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_mon_on") {
       on_off_times[1][0] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_mon_off") {
       on_off_times[1][1] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_tue_on") {
       on_off_times[2][0] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_tue_off") {
       on_off_times[2][1] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_wed_on") {
       on_off_times[3][0] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_wed_off") {
       on_off_times[3][1] = server.arg(i).toInt(); was_timer_changes = true;
     }

     if (server.argName(i) == "set_sched_thu_on") {
       on_off_times[4][0] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_thu_off") {
       on_off_times[4][1] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_fri_on") {
       on_off_times[5][0] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_fri_off") {
       on_off_times[5][1] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_sat_on") {
       on_off_times[6][0] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "set_sched_sat_off") {
       on_off_times[6][1] = server.arg(i).toInt(); was_timer_changes = true;
     }
     if (server.argName(i) == "sched_enable") {
       on_off_enabled = true;
       was_time_changed = true;
     }     
     if (server.argName(i) == "sched_disable") {
       on_off_enabled = false;
       was_time_changed = true;
     }


  }
   if (was_time_changed) {
     set_time_to_rtc(); //send time
   }
   if (was_timer_changes) {
     //store to eram
    save_values_to_eeprom();
  }
  server.send ( 200, "text/html", msg );


}
void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";


  server.send (404 , "text/html", "<html><head>header('location: /'); </head></html>" );

}
void process_schedule(){
  if (on_off_enabled) {
    Serial.println(stunde == on_off_times[wochentag][0] && !on_time_switched);
   //ON
    if (stunde == on_off_times[wochentag][0]  && !on_time_switched) {
     switch_all_on();
      on_time_switched = true;
    }
    else {
      if ( on_off_times[wochentag][0] != stunde) {
        on_time_switched = false;
      }
    }
    //OFF
    if (stunde == on_off_times[wochentag][1] && !off_time_switched) {
      switch_all_off();
      off_time_switched = true;
    }
    else {
      if ( on_off_times[wochentag][1] != stunde) {
        off_time_switched = false;
      }
    }

  }
  }

void make_http_requiest_to_dns_server(){
#if defined(RB_DNS)
if(!rb_dns_conf_correct){return;}
   HTTPClient http;  //Declare an object of class HTTPClient
   http.begin(RB_DNS_HOST_BASE_URL + "?uuid=" + RB_DNS_UUID + "&type=cagelight&version=" + CAGE_LIGHT_VERSION + "&pass=" + RB_DNS_PASSWORD + "&tl=1" + "&port=" + RB_DNS_ACCESS_PORT + "&device_name=" + RB_DNS_DEVICE_NAME);  //Specify request destination
  int httpCode = http.GET();  
   if (httpCode > 0) { 
      String payload = http.getString();   
      Serial.println("RBDNS RESPONSE : " + payload);                     
    }
    http.end();   //Close connection
#endif
  }
  
void setup ( void ) {
  Serial.begin ( SERIAL_BAUD_RATE );

#if defined(RB_DNS)
if(RB_DNS_UUID == "00000000-0000-0000-0000-000000000000"){
  Serial.println("please generate a other uuid at https://www.uuidgenerator.net/version4");
  rb_dns_conf_correct = false;
  }
#endif

  
      //SET PINMODE FOR OUTPUTS
     for(int i = 0; i < AMOUNT_OUTPUTS; i++){
    pinMode ( output_relais_pins[i], OUTPUT );
    switch_channel(i, 1);
    }

    
  //READ TIMES
  EEPROM.begin( 16 +(7*2) + AMOUNT_OUTPUTS);
  
restore_eeprom_values();
 for(int i = 0; i < AMOUNT_OUTPUTS; i++){
  switch_channel(i,output_relais_states[i]);
  }


  off_time_switched = false;
  on_time_switched = false;
  //CLOCK SETUP
  Wire.begin(i2c_scl_pin,i2c_sda_pin);
  jahr = 17;
  monat = 11;
  tag = 28;
  stunde = 21;
  minute = 28;
  sekunde = 0;
    
    #if defined(_DEBUG)
  set_time_to_rtc();
    #endif
    
  get_time_from_rtc();
  process_schedule();
    
setup_wifi();


    
   
  //SWITCH IO CONF
pinMode(switch_0_pin, INPUT);
digitalWrite(switch_0_pin, HIGH);
pinMode(switch_1_pin, INPUT);
digitalWrite(switch_1_pin, HIGH);


  // Wait for connection
  while ( wifiMulti.run() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
    

  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );



  
  if ( MDNS.begin ( MDNS_NAME ) ) {
    Serial.println ( "MDNS responder started" );
  }


make_http_requiest_to_dns_server();

  server.on ( "/", handleRoot );
  server.on ( "/index.html", handleRoot );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
}



  
void loop ( void ) {

  

 get_time_from_rtc();
  process_schedule();
 
//GET SWITCH READINGS
if(digitalRead(switch_0_pin) == LOW && digitalRead(switch_1_pin) == HIGH){
  switch_all_on();
    delay(50);
}else if(digitalRead(switch_1_pin) == LOW && digitalRead(switch_0_pin) == HIGH){
  switch_all_off();
    delay(50)
}else if(digitalRead(switch_1_pin) == HIGH && digitalRead(switch_0_pin) == HIGH){
    delay(50);
}else{
  }
 
      //HANDLE WIFI CONNECTION LOST
      if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println(".");
        yield();
        delay(500);
        return;
    }
//HANDLE WEBSERVER
    server.handleClient();

    delay(30);
}


// Hilfsfunktionen
int dayofweek1(int day, int month, int year){
  /** Variation of Sillke for the Gregorian calendar. **/
  /** With 0=Sunday, 1=Monday, ... 6=Saturday         **/
  if ((month -= 2) <= 0) {
    month += 12;
    year--;
  }
  return (83 * month / 32 + day + year + year / 4 - year / 100 + year / 400) % 7;
}
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}
byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}
void set_time_to_rtc() {
  Serial.println("SAVE TIME TO RTC");
  // Setzt die aktuelle Zeit
  Wire.beginTransmission(DS1307_ADRESSE);
  Wire.write(0x00);
  Wire.write(decToBcd(sekunde));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(stunde));
  Wire.write(decToBcd(wochentag));
  Wire.write(decToBcd(tag));
  Wire.write(decToBcd(monat));
  Wire.write(decToBcd(jahr));
  Wire.write(0x00);
  Wire.endTransmission();
}
void get_time_from_rtc() {
  Serial.println("READ TIME FROM RTC");
  // True=Zeit ausgeben. False = Datum ausgeben
  // Initialisieren
  Wire.beginTransmission(DS1307_ADRESSE);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADRESSE, 7);
  sekunde = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  stunde = bcdToDec(Wire.read() & 0b111111);
  wochentag = bcdToDec(Wire.read());
  tag = bcdToDec(Wire.read());
  monat = bcdToDec(Wire.read());
  jahr = bcdToDec(Wire.read());
  wochentag = dayofweek1(tag, monat, jahr);
}

