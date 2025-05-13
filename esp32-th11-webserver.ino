#include <UtilidadesClima.h>
#include <DHT.h>
#include <WiFi.h>
#include "include/secrets.h"


// ---------- DEFINICIONES ---------- //
#define DHTPIN 19
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- CONFIGURACIÓN DE WI-FI ---------- //

// Credenciales de WiFi
const char* WIFI_SSID = Secrets::WIFI_SSID;
const char* WIFI_PSWD = Secrets::WIFI_PSWD;

// Creación de un servidor en el puerto 80
WiFiServer server(80);

// ---------- VARIABLES DINÁMICAS ---------- //
// Variables para almacenar los valores de temperatura y humedad
float temperaturaCelsius, humedadRelativa;
String errorMensaje = ""; // Mensaje de error en caso de fallos
bool clienteConectado = false, clienteDesconectado = false;

// ---------- PROGRAMA PRINCIPAL ---------- //

void setup() {
  conectarWiFi(); // Conecta el ESP32 a la red WiFi y arranca el servidor
  dht.begin();    // Inicializa el sensor DHT11
}

void loop() {
  
  // Espera y atiende a los clientes que se conecten al servidor
  WiFiClient client = server.available();
  if (client) {
    manejarCliente(client); // Procesa la solicitud del cliente
  }
}

// ---------- FUNCIONES ---------- //

void conectarWiFi() {
  Serial.begin(115200);
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n¡Conectado!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Servidor web iniciado.");
}

// Manejo de clientes conectados al servidor
void manejarCliente(WiFiClient client) {
  String clienteIP = "";
  if(!clienteConectado){
    clienteConectado = true;
    clienteIP = client.remoteIP().toString();
    Serial.print("Nuevo cliente conectado desde :");
    Serial.println(clienteIP);
  }
  
  String lineaActual = "";

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      // Serial.write(c);  //No imprimir cada carácter recibido

      if (c == '\n') {
        if (lineaActual.length() == 0) {
          temperaturaCelsius = dht.readTemperature();
          humedadRelativa = dht.readHumidity();
          if(isnan(temperaturaCelsius) || isnan(humedadRelativa)){
            errorMensaje = "Error al leer datos del sensor DHT.";
            Serial.println(errorMensaje);
          }
          else  errorMensaje = "";
          enviarPaginaHTML(client);
          break;
        } else {
          lineaActual = "";
        }
      } else if (c != '\r') {
        lineaActual += c;
      }
    }
  }

  client.stop();
  
}

// Enviar la página HTML con los datos de temperatura y humedad
void enviarPaginaHTML(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println("Cache-Control: no-store, no-cache, must-revalidate");
  client.println("Pragma: no-cache"); 
  client.println();

  client.println("<!DOCTYPE html><html lang='es'><head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<meta http-equiv='refresh' content='10'>");
  client.println("<title>Monitor Climático ESP32</title>");
  client.println("<style>");
  client.println("body{font-family: Arial, sans-serif; background-color:#f5f7fa; color:#333; margin:0; padding:15px; line-height:1.5;}");
  client.println(".container{max-width:500px; margin:0 auto; background:white; border-radius:10px; box-shadow:0 3px 10px rgba(0,0,0,0.1); padding:20px;}");
  client.println("h1{color:#2c3e50; text-align:center; margin-bottom:5px; font-size:1.5rem;}");
  client.println(".sensor-data{margin-top:20px;}");
  client.println(".data-group{margin-bottom:15px; border-bottom:1px solid #eee; padding-bottom:15px;}");
  client.println(".data-group:last-child{border-bottom:none;}");
  client.println(".data-title{color:#2c3e50; font-weight:bold; margin-bottom:5px; font-size:1.1rem;}");
  client.println(".data-row{display:flex; justify-content:space-between; margin-bottom:5px;}");
  client.println(".data-label{color:#7f8c8d;}");
  client.println(".data-value{font-weight:bold;}");
  client.println(".temperature .data-value{color:#e74c3c;}");
  client.println(".humedad .data-value{color:#3498db;}");
  client.println(".informacion-actualizada{text-align:center; margin-top:20px; font-size:0.8rem; color:#95a5a6;}");
  client.println("</style></head><body><div class='container'>");

  client.println("<h1 style='color:#2c3e50; font-size:2rem; text-shadow:1px 1px 2px rgba(0,0,0,0.1); margin-bottom: 20px;'>🌤️ Monitor Climático ESP32 🌡️</h1>");
  client.println("<div class='sensor-data'>");

  // Temperatura
  client.println("<div class='data-group temperature'>");
  client.println("<div class='data-title'>TEMPERATURA 🌡️</div>");
  client.println("<div class='data-row'><span class='data-label'>Celsius:</span><span class='data-value'>" + String(temperaturaCelsius) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Fahrenheit:</span><span class='data-value'>" + convertirDeCelsiusA(FAHRENHEIT, true, temperaturaCelsius) + "</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Kelvin:</span><span class='data-value'>" + convertirDeCelsiusA(KELVIN, true, temperaturaCelsius) + "</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Rankine:</span><span class='data-value'>" + convertirDeCelsiusA(RANKINE, true, temperaturaCelsius) + "</span></div>");
  client.println("</div>");

  // Humedad
  client.println("<div class='data-group humedad'>");
  client.println("<div class='data-title'>HUMEDAD 💧</div>");
  client.println("<div class='data-row'><span class='data-label'>Relativa:</span><span class='data-value'>" + String(humedadRelativa) + " %</span></div>");
  float humedadAbs = calcularHumedadAbsoluta(temperaturaCelsius, humedadRelativa);
  client.println("<div class='data-row'><span class='data-label'>Absoluta:</span><span class='data-value'>" + String(humedadAbs) + " g/m³</span></div>");
  client.println("</div>");

  // Cálculos
  client.println("<div class='data-group'>");
  client.println("<div class='data-title'>CÁLCULOS 🍃🌧☔💦</div>");
  client.println("<div class='data-row'><span class='data-label'>Punto de Rocío:</span><span class='data-value'>" + String(calcularPuntoRocio(temperaturaCelsius, humedadRelativa)) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Sensación Térmica:</span><span class='data-value'>" + String(calcularSensacionTermica(temperaturaCelsius, humedadRelativa)) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Bulbo Húmedo:</span><span class='data-value'>" + String(calcularBulboHumedo(temperaturaCelsius, humedadRelativa)) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Presión de Vapor:</span><span class='data-value'>" + String(calcularPresionVapor(temperaturaCelsius, humedadRelativa)) + " hPa</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Presión Saturación:</span><span class='data-value'>" + String(calcularPresionSaturacion(temperaturaCelsius)) + " hPa</span></div>");
  client.println("</div>");

  // Error (si existe)
  if (errorMensaje != "") {
    client.println("<h3 style='color:red; text-align:center;'>ERROR: " + errorMensaje + "</h3>");
  }

  client.println("</div></div></body></html>");
  client.println();
}

