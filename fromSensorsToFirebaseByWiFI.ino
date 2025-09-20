/*
  Questo sketch legge i dati dei sensori e li scrive su un Database Firebase real-time via WIFI  
  sensore di temperatura collegato a A1
  sensore di umidità del terreno collegato a A0
  
 */

#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <DHT.h>


bool wifiConnect();
bool dbPost(String pathJson, String jsonBody);
bool dbPut(String pathJson, String jsonBody);
void readSensors();


// ====== IMPOSTAZIONI UTENTE ======
//WIFI SCUOLA: DA CAMBIARE CON I DATI DEL WIFI UTILIZZATO
#define WIFI_SSID "Galilei TEST"
#define WIFI_PASSWORD "Coniglio21"



#define DATABASE_HOST "lostinrome-sensori-default-rtdb.firebaseio.com"


#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(2, DHT11);

#define SOULANALOGPIN 8
#define SOULDIGITALPIN A1


// ====== Client HTTPS + HttpClient verso il Realtime Database ======
WiFiSSLClient ssl;
HttpClient httpDB(ssl, DATABASE_HOST, 443);


// ====== Valori sensori ======
float temperatureC   = 0.0;
float humidity       = 0.0;
int   soil_moisture  = 0;




void setup() {
 Serial.begin(115200);
 dht.begin();
 while (!Serial) { ; }
 //randomSeed(analogRead(A0));
  pinMode(SOULDIGITALPIN, INPUT);


 Serial.println("Connessione WiFi...");
 if (!wifiConnect()) {
   Serial.println("WiFi non connesso.");
   return;
 }
 Serial.print("Connesso. IP: ");
 Serial.println(WiFi.localIP());
 Serial.println("Sistema pronto.");
}


void loop() {
 if (WiFi.status() != WL_CONNECTED) {


   Serial.println("WiFi disconnesso. Riconnessione...");
   if (!wifiConnect()) {
     delay(2000);
     return;
   }
 }


 readSensors();


 // Costruisci payload JSON da scrivere sul DB Firebase
 DynamicJsonDocument doc(512);
 doc["temperature"]   = temperatureC;
 doc["humidity"]      = humidity;
 doc["soil_moisture"] = soil_moisture;


 String jsonStr;
 serializeJson(doc, jsonStr);


 Serial.println("Payload JSON:");
 Serial.println(jsonStr);



 // ===== OPZIONE B: SOVRASCRIVI (PUT) percorso fisso =====
 bool ok = dbPut("/sensors.json", jsonStr);


 Serial.println(ok ? "Dati inviati!" : "Invio fallito.");


 delay(10000); // 10 secondi tra un invio e l'altro
}




// ------- Helper: connessione WiFi -------
bool wifiConnect() {
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 unsigned long start = millis();


 // Attende connessione
 while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
   delay(300);
 }
 if (WiFi.status() != WL_CONNECTED) return false;


 // Attende un IP valido (evita 0.0.0.0)
 IPAddress ip = WiFi.localIP();
 start = millis();
 while ((ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) && millis() - start < 10000) {
   delay(200);
   ip = WiFi.localIP();
 }
 return !(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0);
}
//const String&
// ------- Helper: POST JSON su un path del DB -------
bool dbPost(String pathJson, String jsonBody) {
 // Esempio pathJson: "/sensors.json"
 httpDB.beginRequest();
 httpDB.post(pathJson.c_str(), "application/json", jsonBody);
 httpDB.endRequest();
 readSensors();


 int status = httpDB.responseStatusCode();
 String body = httpDB.responseBody();


 Serial.print("[POST] Status: ");
 Serial.println(status);
 if (body.length()) {
   Serial.println("Body:");
   Serial.println(body);
 }
 return (status >= 200 && status < 300);
}


// ------- Helper: PUT JSON (sovrascrive il nodo) -------
bool dbPut(String pathJson, String jsonBody) {
 // Esempio pathJson: "/sensors.json"
 httpDB.beginRequest();
 httpDB.put(pathJson.c_str(), "application/json", jsonBody);
 httpDB.endRequest();


 int status = httpDB.responseStatusCode();
 String body = httpDB.responseBody();


 Serial.print("[PUT] Status: ");
 Serial.println(status);
 if (body.length()) {
   Serial.println("Body:");
   Serial.println(body);
 }
 return (status >= 200 && status < 300);
}






void readSensors() {
 
 //temperatureC  = 20.0 + (random(0, 50) / 10.0);
 temperatureC = (((analogRead(A0)*5.0) / 1023.0) - 0.5) * 100;


 //humidity = 60.0 + (random(0, 30) / 10.0);
 humidity = dht.readHumidity();
 
 //soil_moisture = 2000 + random(0, 100);
 soil_moisture = digitalRead(SOULDIGITALPIN);
 //int a = analogRead(SOULANALOGPIN);


 Serial.println("Letture sensori:");
 Serial.print("  T: "); Serial.println(temperatureC);
 Serial.print("  RH: "); Serial.println(humidity);
 Serial.print("  Soil: "); Serial.println(soil_moisture);
}






