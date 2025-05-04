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
  Serial.println("Nuevo cliente conectado.");
  String lineaActual = "";

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);

      if (c == '\n') {
        if (lineaActual.length() == 0) {
          temperatura = dht.readTemperature();
          humedad = dht.readHumidity();
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
  Serial.println("Cliente desconectado.");
}

// Enviar la página HTML con los datos de temperatura y humedad
void enviarPaginaHTML(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  // Estructura básica del HTML
  client.println("<!DOCTYPE html><html lang='es'><head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta http-equiv='refresh' content='5'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");

  client.println("<style>");
  client.println("body{font-family: Helvetica; text-align: center;}");
  client.println("</style></head><body>");

  // Muestra los datos en la página
  client.println("<h1>Web Server con ESP32</h1>");

  // ---------- Temperatura ---------- //
  client.print("<h3>TEMPERATURA: ");
  client.println(temperatura);
  client.println(" °C</h3>");

  client.print("<h3>TEMPERATURA (°F): ");
  client.println(convertirDeCelsiusA(FAHRENHEIT, true));
  client.println("</h3>");

  client.print("<h3>TEMPERATURA (K): ");
  client.println(convertirDeCelsiusA(KELVIN, true));
  client.println("</h3>");

  client.print("<h3>TEMPERATURA (°R): ");
  client.println(convertirDeCelsiusA(RANKINE, true));
  client.println("</h3>");

  client.println("<br>"); // Salto de línea

  // ---------- Humedad ---------- //
  client.print("<h3>HUMEDAD: ");
  client.println(humedad);
  client.println(" %</h3>");

  client.println("<br>");

  //Punto de Rocío
  float puntoRocio = calcularPuntoRocio(temperatura, humedad);
  client.print("<h3>PUNTO DE ROCÍO: ");
  client.println(puntoRocio);
  client.println(" %</h3>");

  client.println("<br>");

  // ---------- Humedad Absoluta ---------- //
  float humedadAbsoluta = calcularHumedadAbsoluta(temperatura, humedad);
  client.print("<h3>HUMEDAD ABSOLUTA: ");
  client.println(humedadAbsoluta);
  client.println(" g/m³</h3>");

  client.println("<br>");

  // ---------- Sensación Térmica ---------- //
  float sensacionTermica = calcularSensacionTermica(temperatura, humedad);
  client.print("<h3>SENSACIÓN TÉRMICA: ");
  client.println(sensacionTermica);
  client.println(" °C</h3>");

  client.println("<br>");

  // ---------- Bulbo Húmedo ---------- //
  float bulboHumedo = calcularBulboHumedo(temperatura, humedad);
  client.print("<h3>BULBO HÚMEDO: ");
  client.println(bulboHumedo);
  client.println(" °C</h3>");

  client.println("<br>"); // Salto de línea

  // ---------- Presión de Vapor ---------- //
  float presionVapor = calcularPresionVapor(temperatura, humedad);
  client.print("<h3>PRESIÓN DE VAPOR: ");
  client.println(presionVapor);
  client.println(" hPa</h3>");

  client.println("<br>");

  // ---------- Presión de Saturación ---------- //
  float presionSaturacion = calcularPresionSaturacion(temperatura);
  client.print("<h3>PRESIÓN DE SATURACIÓN: ");
  client.println(presionSaturacion);
  client.println(" hPa</h3>");

  // Si hay un error
  if(errorMensaje != ""){
    client.println("<h3 style='color:red;'>ERROR: " + errorMensaje + "</h3>");
  }

  client.println("</body></html>");
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
  conectarWiFi();
  dht.begin();
}

void loop() {
  if(isnan(temperatura) || isnan(humedad)){
    errorMensaje = "Error al leer datos del sensor DHT.";
    Serial.println(errorMensaje);
  }
  else  errorMensaje = "";

  // Atender las solicitudes
  WiFiClient client = server.available();
  if (client) {
    manejarCliente(client);
  }
}
