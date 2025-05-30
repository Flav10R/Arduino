// Impresor de pasajes - Arduino UNO (Ticket IDA/VUELTA)

#include <SoftwareSerial.h>

// CONFIGURACIÓN PRINCIPAL
const int MAX_LINEAS_TICKET = 30;      // Máximo de líneas del ticket
const int MAX_CHARS_LINEA = 34;        // Máximo caracteres por línea
const int BAUD_RATE_INPUT = 9600;      // Velocidad puerto serial entrada
const int BAUD_RATE_PRINTER = 9600;    // Velocidad puerto serial salida

// Pines para la impresora
const int RX_PIN = 2;  // No se usa pero es requerido para SoftwareSerial
const int TX_PIN = 3;  // Conectar al RX de la impresora
SoftwareSerial printerSerial(RX_PIN, TX_PIN);

// Textos configurables
const char* NOMBRE_EMPRESA = "MONTE GRANDE BUS";
const char* DESTINO_IDA = "MTE. GRANDE - OBELISCO";
const char* DESTINO_VUELTA = "OBELISCO - MTE. GRANDE";
const char* TEXTO_PIE = "Gracias y buen viaje!";
const char* LINEA_CORTE = "---8<----cortar por aqui---8<---";

// Comandos de control para la impresora
const byte CMD_TEXT_NORMAL[] = {27, 33, 0};       // ESC ! 0 - Tamaño normal
const byte CMD_TEXT_LARGE[] = {27, 33, 16};       // ESC ! 16 - Tamaño grande
const byte CMD_TEXT_BOLD_ON[] = {27, 69, 1};      // ESC E 1 - Negrita ON
const byte CMD_TEXT_BOLD_OFF[] = {27, 69, 0};     // ESC E 0 - Negrita OFF
const byte CMD_ALIGN_CENTER[] = {27, 97, 1};      // ESC a 1 - Centrado
const byte CMD_ALIGN_LEFT[] = {27, 97, 0};        // ESC a 0 - Alineado izquierda
const byte CMD_ALIGN_RIGHT[] = {27, 97, 2};       // ESC a 2 - Alineado derecha
const byte CMD_FEED_LINES[] = {27, 100, 3};       // ESC d 3 - Avanzar 3 líneas

// Variables para almacenar datos analizados
char userId[10];       // 9 dígitos + terminador nulo
char equipmentNum[2];  // 1 dígito + terminador nulo
char date[9];          // DD-MM-AA + terminador nulo
char time[9];          // HH:mm:ss + terminador nulo
char formattedDate[11]; // NUEVA VARIABLE: Para la fecha en formato DD-MM-YYYY

// Buffer para datos entrantes
const int BUFFER_SIZE = 50;
char buffer[BUFFER_SIZE];
int bufferIndex = 0;
bool newData = false;

void setup() {
  Serial.begin(BAUD_RATE_INPUT);
  printerSerial.begin(BAUD_RATE_PRINTER);
  
  Serial.println("Impresor de pasajes IDA/VUELTA");
  Serial.println("Listo para recibir datos...");
  Serial.println("Formato: NNNNNNNNN   E DD-MM-AA HH:mm:ss X");
  Serial.println("Ejemplo: 000000006   1 14-08-24 23:46:41 I");
}

void loop() {
  // Leer datos del puerto serie
  readSerialData();
  
  // Procesar los datos si se recibieron
  if (newData) {
    if (parseData()) {
      printTicket();
    }
    
    // Reiniciar para el siguiente dato
    bufferIndex = 0;
    newData = false;
  }
}
//===========================================================
/*
void readSerialData() {
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    
    if (inChar == '\n') {
      buffer[bufferIndex] = '\0'; // Terminar con nulo
      newData = true;
      return;
    }
     
    if (bufferIndex < BUFFER_SIZE - 1) {
      buffer[bufferIndex] = inChar;
      bufferIndex++;
    }
  }
}
*/
//==============================================================
// Nueva funcion para escuchar ambos puertos

void readSerialData() {
  // Comprobar puerto serie hardware
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    processSerialChar(inChar, "Hardware");
  }
  
  // Comprobar puerto serie software por pin 2
  while (printerSerial.available() > 0) {
    char inChar = printerSerial.read();
    processSerialChar(inChar, "Externo");
  }
}

void processSerialChar(char inChar, const char* source) {
  if (inChar == '\n') {
    buffer[bufferIndex] = '\0';
    newData = true;
    Serial.print("Datos recibidos (");
    Serial.print(source);
    Serial.print("): ");
    Serial.println(buffer);
    return;
  }
  
  if (bufferIndex < BUFFER_SIZE - 1) {
    buffer[bufferIndex] = inChar;
    bufferIndex++;
  }
}
//==============================================================
//
bool parseData() {
  // Formato esperado: "NNNNNNNNN E DD-MM-AA HH:mm:ss X"
  
  // Comprobar longitud mínima (debe ser al menos 28 caracteres)
  //if (strlen(buffer) < 28) {
  // Serial.println("Error: Datos demasiado cortos");
  //  return false;
  //}
  // Formato esperado: "NNNNNNNNN  E DD-MM-AA HH:mm:ss X" (dos espacios entre ID y equipo)
  if (strlen(buffer) < 2) {
    Serial.println("Error: Datos demasiado cortos");
    return false;
  }
  
  // Extraer ID de usuario (primeros 9 caracteres)
  strncpy(userId, buffer, 9);
  userId[9] = '\0';
  
  // Validar ID de usuario (debe ser 9 dígitos)
  for (int i = 0; i < 9; i++) {
    if (!isdigit(userId[i])) {
      Serial.println("Error: Formato de ID de usuario inválido");
      return false;
    }
  }
  
// Extraer número de equipo (debería estar en la posición 11)
  
//  if (!isdigit(buffer[11])) {
//    Serial.println("Error: Número de equipo inválido");
//    return false;
//  }

// Extraer número de equipo (ahora está en la posición 12  
if (!isdigit(buffer[12])) {
    Serial.println("Error: Número de equipo inválido");
    return false;
  }


  
// equipmentNum[0] = buffer[11];
//equipmentNum[1] = '\0';

  equipmentNum[0] = buffer[12];
  equipmentNum[1] = '\0';
  
  // Extraer fecha (debería estar en la posición 13-20)
 // strncpy(date, &buffer[13], 8);
  //date[8] = '\0';

// Extraer fecha (debería estar en la posición 14-21)
  strncpy(date, &buffer[14], 8);
  date[8] = '\0';

  
  // Validar formato de fecha (DD-MM-AA)
  if (date[2] != '-' || date[5] != '-') {
    Serial.println("Error: Formato de fecha inválido");
    return false;
  }
  // --- CONVERSIÓN DEL AÑO DE 2 DÍGITOS A 4 DÍGITOS ---
  // Extraemos los componentes numéricos de la fecha original (que está en 'date')
  int day = (date[0] - '0') * 10 + (date[1] - '0');
  int month = (date[3] - '0') * 10 + (date[4] - '0');
  int year_short = (date[6] - '0') * 10 + (date[7] - '0');

  // Lógica para determinar el siglo:
  // Si el año de 2 dígitos es 50 o menos, asumimos 20XX (ej. 23 -> 2023).
  // Si el año de 2 dígitos es 51 o más, asumimos 19XX (ej. 99 -> 1999).
  // Esta lógica es común para evitar problemas con años futuros.
  int year_long;
  if (year_short >= 0 && year_short <= 50) { 
      year_long = 2000 + year_short;
  } else { 
      year_long = 1900 + year_short;
  }
  // --- FIN DE LA CONVERSIÓN ---

  // Guardar la fecha con el año de 4 dígitos en la nueva variable 'formattedDate'
  // Formato: DD-MM-YYYY
  sprintf(formattedDate, "%02d-%02d-%04d", day, month, year_long);
  
  // Extraer hora (debería estar en la posición 22-29)
 // strncpy(time, &buffer[22], 8);
 // time[8] = '\0';

// Extraer hora (debería estar en la posición 23-30)
  strncpy(time, &buffer[23], 8);
  time[8] = '\0';

  
  // Validar formato de hora (HH:mm:ss)
  if (time[2] != ':' || time[5] != ':') {
    Serial.println("Error: Formato de hora inválido");
    return false;
  }
  
  // Imprimir datos analizados para verificación
  Serial.println("Datos analizados:");
  Serial.print("ID Usuario: "); Serial.println(userId);
  Serial.print("Equipo: "); Serial.println(equipmentNum);
  Serial.print("Fecha (Original DD-MM-AA): "); Serial.println(date);
  Serial.print("Fecha (4 dígitos DD-MM-YYYY): "); Serial.println(formattedDate);
  Serial.print("Hora: "); Serial.println(time);
  
  // Extraer hora (debería estar en la posición 22-29)
  //strncpy(time, &buffer[22], 8);
  //time[8] = '\0';
  
  // Validar formato de hora (HH:mm:ss)
  //if (time[2] != ':' || time[5] != ':') {
  //  Serial.println("Error: Formato de hora inválido");
  //  return false;
 // }
  
  // Imprimir datos analizados para verificación
  //Serial.println("Datos analizados:");
  //Serial.print("ID Usuario: "); Serial.println(userId);
  //Serial.print("Equipo: "); Serial.println(equipmentNum);
  //Serial.print("Fecha: "); Serial.println(date);
  //Serial.print("Hora: "); Serial.println(time);
  
  return true;
}

void printTicket() {
  Serial.println("Imprimiendo pasaje...");
  
  // ----- SECCIÓN IDA -----
    printerSerial.println();
  
  // 1. Nombre de empresa (grande y centrado)
  printerSerial.write(CMD_ALIGN_CENTER, 3);
  printerSerial.write(CMD_TEXT_LARGE, 3);
  printerSerial.println(NOMBRE_EMPRESA);
  
  // 2. Espaciado
  printerSerial.write(CMD_TEXT_NORMAL, 3);
  printerSerial.println();
  
  // 3. Destino IDA (normal, negrita)
  printerSerial.write(CMD_TEXT_BOLD_ON, 3);
  printerSerial.println(DESTINO_IDA);
  printerSerial.write(CMD_TEXT_BOLD_OFF, 3);
  printerSerial.println();
  
  // 4. Texto "IDA" (grande y negrita)
  printerSerial.write(CMD_TEXT_LARGE, 3);
  printerSerial.write(CMD_TEXT_BOLD_ON, 3);
  printerSerial.println("IDA");
  printerSerial.write(CMD_TEXT_BOLD_OFF, 3);
  printerSerial.write(CMD_TEXT_NORMAL, 3);
  
  // 5. Espaciado
  printerSerial.println();
  
  //
// 6. Datos de usuario y equipo
  printerSerial.write(CMD_ALIGN_LEFT, 3);
  printerSerial.print("ID: ");
  printerSerial.print(userId);
  printerSerial.print("  Terminal #");
  printerSerial.println(equipmentNum);
  
  // Fecha y hora
  printerSerial.write(CMD_ALIGN_CENTER, 3);
  printerSerial.print(formattedDate);
  printerSerial.print("              ");
  printerSerial.println(time);
    
  // 7. Separador
  printerSerial.println();
  
  // 8. Línea de corte
  printerSerial.write(CMD_ALIGN_CENTER, 3);
  printerSerial.println(LINEA_CORTE);
  printerSerial.println();
  
  // ----- SECCIÓN VUELTA -----
  // 9. Espaciado
    printerSerial.println();
  // 10. Nombre de empresa (grande y centrado)
  printerSerial.write(CMD_ALIGN_CENTER, 3);
  printerSerial.write(CMD_TEXT_LARGE, 3);
  printerSerial.println(NOMBRE_EMPRESA);
  
  // 11. Espaciado
  printerSerial.write(CMD_TEXT_NORMAL, 3);
  printerSerial.println();
  
  // 12. Destino VUELTA (normal, negrita)
  printerSerial.write(CMD_TEXT_BOLD_ON, 3);
  printerSerial.println(DESTINO_VUELTA);
  printerSerial.write(CMD_TEXT_BOLD_OFF, 3);
  printerSerial.println();
  
  // 13. Texto "VUELTA" (grande y negrita)
  printerSerial.write(CMD_TEXT_LARGE, 3);
  printerSerial.write(CMD_TEXT_BOLD_ON, 3);
  printerSerial.println("VUELTA");
  printerSerial.write(CMD_TEXT_BOLD_OFF, 3);
  printerSerial.write(CMD_TEXT_NORMAL, 3);
  
  // 14. Espaciado
  printerSerial.println();
  
  // 15. Datos de usuario y equipo
  printerSerial.write(CMD_ALIGN_LEFT, 3);
  printerSerial.print("ID: ");
  printerSerial.print(userId);
  printerSerial.print("  Terminal #");
  printerSerial.println(equipmentNum);
  
  // Fecha y hora
  printerSerial.write(CMD_ALIGN_CENTER, 3);
  printerSerial.print(formattedDate);
  printerSerial.print("              ");
  printerSerial.println(time);
  printerSerial.write(CMD_ALIGN_CENTER, 3);
  printerSerial.println();
  // Fecha y hora
  //printerSerial.print(date);
  //printerSerial.print("  ");
  //printerSerial.println(time);
  
  // 16. Texto pie
  printerSerial.write(CMD_ALIGN_CENTER, 3);
  printerSerial.println("--------------------------------");
  printerSerial.write(CMD_TEXT_BOLD_ON, 3);
  printerSerial.println(TEXTO_PIE);
  printerSerial.write(CMD_TEXT_BOLD_OFF, 3);
  
  // 17. Alimentar papel para corte final
  printerSerial.println("\n\n\n\n\n\n\n\n");
  
  Serial.println("¡Pasaje impreso con éxito!");
}
