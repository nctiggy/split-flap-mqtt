const char wifi_ssid[] ="<ssid>";
const char wifi_password[] = "<ssid_pw>";
const char mdns_hostname[] = "flaps-01";
const char* mqtt_server = "<mqtt_server>";
const int mqtt_port = 1883;
const char* mqtt_listen = "/game";
const char* mqtt_registration = "/registration";
const char* mqtt_debug = "/debug";

const int check_in_freq = 60000;
IPAddress ip_address;
char ip_char[16];
unsigned long lastMillis = 0;
String reg;
int displayState[number_units];
const int answersize = 1; //Size of units request answer
const int flapamount = 45; //Amount of Flaps in each unit
const int minspeed = 1; //min Speed
const int maxspeed = 12; //max Speed
const char letters[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '$', '&', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', '-', '?', '!'};
char writtenLast[number_units];
const int flap_speed = 10;
