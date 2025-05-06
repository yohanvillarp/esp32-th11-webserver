#include <DHT.h>
#include <WiFi.h>


// ---------- DEFINICIONES ---------- //
#define DHTPIN 19
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Definición de unidades para conversiones
#define FAHRENHEIT 1
#define KELVIN 2
#define RANKINE 3

// ---------- CONFIGURACIÓN DE WI-FI ---------- //

// Credenciales de WiFi
const char* ssid = "Yohan :)";
const char* password = "pandita19";

// Creación de un servidor en el puerto 80
WiFiServer server(80);

// ---------- VARIABLES DINÁMICAS ---------- //
// Variables para almacenar los valores de temperatura y humedad
float temperatura, humedad;
String errorMensaje = ""; // Mensaje de error en caso de fallos
bool clienteConectado = false, clienteDesconectado = false;

// ---------- FUNCIONES ---------- //

void conectarWiFi() {
  Serial.begin(115200);
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid, password);

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
          temperatura = dht.readTemperature();
          humedad = dht.readHumidity();
          if(isnan(temperatura) || isnan(humedad)){
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
  client.println();

  client.println("<!DOCTYPE html><html lang='es'><head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<meta http-equiv='refresh' content='5'>");
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
  client.println("<div class='data-row'><span class='data-label'>Celsius:</span><span class='data-value'>" + String(temperatura) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Fahrenheit:</span><span class='data-value'>" + convertirDeCelsiusA(FAHRENHEIT, true) + "</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Kelvin:</span><span class='data-value'>" + convertirDeCelsiusA(KELVIN, true) + "</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Rankine:</span><span class='data-value'>" + convertirDeCelsiusA(RANKINE, true) + "</span></div>");
  client.println("</div>");

  // Humedad
  client.println("<div class='data-group humedad'>");
  client.println("<div class='data-title'>HUMEDAD 💧</div>");
  client.println("<div class='data-row'><span class='data-label'>Relativa:</span><span class='data-value'>" + String(humedad) + " %</span></div>");
  float humedadAbs = calcularHumedadAbsoluta(temperatura, humedad);
  client.println("<div class='data-row'><span class='data-label'>Absoluta:</span><span class='data-value'>" + String(humedadAbs) + " g/m³</span></div>");
  client.println("</div>");

  // Cálculos
  client.println("<div class='data-group'>");
  client.println("<div class='data-title'>CÁLCULOS 🍃🌧☔💦</div>");
  client.println("<div class='data-row'><span class='data-label'>Punto de Rocío:</span><span class='data-value'>" + String(calcularPuntoRocio(temperatura, humedad)) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Sensación Térmica:</span><span class='data-value'>" + String(calcularSensacionTermica(temperatura, humedad)) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Bulbo Húmedo:</span><span class='data-value'>" + String(calcularBulboHumedo(temperatura, humedad)) + " °C</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Presión de Vapor:</span><span class='data-value'>" + String(calcularPresionVapor(temperatura, humedad)) + " hPa</span></div>");
  client.println("<div class='data-row'><span class='data-label'>Presión Saturación:</span><span class='data-value'>" + String(calcularPresionSaturacion(temperatura)) + " hPa</span></div>");
  client.println("</div>");

  // Error (si existe)
  if (errorMensaje != "") {
    client.println("<h3 style='color:red; text-align:center;'>ERROR: " + errorMensaje + "</h3>");
  }

  client.println("</div></div></body></html>");
  client.println();
}


// ---------- FUNCIONES DE CÁLCULO ---------- //

// Convertir la temperatura de Celsius a otras unidades
String convertirDeCelsiusA(int unidad, bool incluirGrado){
  String grado;
  float conversion;

  if(unidad == FAHRENHEIT){
    conversion = temperatura * 9/5+32;
    grado = "°F";
  }else{
    conversion = temperatura + 273.15;
    if(unidad == KELVIN)  grado = "K";
    else if(unidad == RANKINE){
      conversion = conversion * 9/5;
      grado = "°R";
    }
  }

  if(incluirGrado)  return String(conversion) +" "+ grado;
  else  return String(conversion);
}

// Calcular el punto de rocío
float calcularPuntoRocio(float tempC, float humedadRel) {
  float a = 17.62;
  float b = 243.12;
  float gamma = (a * tempC) / (b + tempC) + log(humedadRel / 100.0);
  float puntoRocio = (b * gamma) / (a - gamma);
  return puntoRocio;
}
// Calcular la humedad absoluta
float calcularHumedadAbsoluta(float tempC, float humedadRel) {
  float Pv = calcularPresionVapor(tempC, humedadRel);
  return (Pv * 2.1674) / (273.15 + tempC); // g/m³
}

// Calcular la sensación térmica (índice de calor)
float calcularSensacionTermica(float tempC, float humedadRel) {
  float tempF = tempC * 9/5 + 32;
  float HI = -42.379 + 2.04901523*tempF + 10.14333127*humedadRel 
             - 0.22475541*tempF*humedadRel - 0.00683783*pow(tempF,2)
             - 0.05481717*pow(humedadRel,2) + 0.00122874*pow(tempF,2)*humedadRel 
             + 0.00085282*tempF*pow(humedadRel,2) - 0.00000199*pow(tempF,2)*pow(humedadRel,2);
  return (HI - 32) * 5/9; // Devuelve en °C
}
// Calcular la temperatura de bulbo húmedo
float calcularBulboHumedo(float tempC, float humedadRel) {
  return tempC * atan(0.151977 * sqrt(humedadRel + 8.313659)) +
         atan(tempC + humedadRel) - atan(humedadRel - 1.676331) +
         0.00391838 * pow(humedadRel, 1.5) * atan(0.023101 * humedadRel) -
         4.686035;
}
// Calcular la presión de vapor actual
float calcularPresionVapor(float tempC, float humedadRel) {
  return calcularPresionSaturacion(tempC) * (humedadRel / 100.0); // presión de vapor actual
}
// Calcular la presión de saturación
float calcularPresionSaturacion(float tempC) {
  return 6.112 * exp((17.67 * tempC) / (tempC + 243.5));  // hPa
}


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
