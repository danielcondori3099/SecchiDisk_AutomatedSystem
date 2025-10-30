#include <Wire.h>
#include <AS5600.h>

AS5600 encoder;
int lastRawAngle = 0;
long vueltas = 0;

const float radio_mm = 21.5;  // Radio de la polea en mm
const float circunferencia = 2 * PI * radio_mm;

// Pines del TB6612FNG
#define IN1     14    // Dirección ADELANTE (subir)
#define IN2     27    // Dirección ATRÁS (bajar)
#define STBY    12    // Standby del driver
#define PWM_A   18    // PWM canal A 

// Pines del interruptor de 3 posiciones
#define POS_SUBIR     19
#define POS_BAJAR     26

// Sensores
#define FINAL_CARRERA 25    // Final de carrera (inferior)

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // SDA = GPIO21, SCL = GPIO22
  encoder.begin();
  if (encoder.isConnected()) {
    Serial.println("AS5600 conectado correctamente.");
  } else {
    Serial.println("Error: AS5600 no detectado.");
    while (1);
  }
  lastRawAngle = encoder.readAngle();

  // Configurar pines del motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(PWM_A, OUTPUT);

  digitalWrite(STBY, HIGH);  // Habilita el driver
  digitalWrite(PWM_A, HIGH);  // Mantener siempre activado

  // Pines del interruptor
  pinMode(POS_SUBIR, INPUT_PULLUP);
  pinMode(POS_BAJAR, INPUT_PULLUP);

  // Pines de sensores
  pinMode(FINAL_CARRERA, INPUT_PULLUP);

  detenerMotor();
}

void loop() {
  int rawAngle = encoder.readAngle();
  // Diferencia entre lecturas
  int delta = rawAngle - lastRawAngle;
  // Corrección por salto de 4095 ↔ 0
  if (delta > 2048) {
    vueltas--;  // Sentido antihorario (inverso)
  } else if (delta < -2048) {
    vueltas++;  // Sentido horario
  }
  lastRawAngle = rawAngle;
  // Calcular ángulo en grados y distancia
  float grados = rawAngle * 360.0 / 4096.0;
  float distancia_mm = vueltas * circunferencia + (grados / 360.0) * circunferencia;
  
  
  bool subir = digitalRead(POS_SUBIR) == LOW;
  bool bajar = digitalRead(POS_BAJAR) == LOW;
  bool finCarrera = digitalRead(FINAL_CARRERA);     // HIGH = límite abajo
  int val = 0;

  if (subir && !bajar && !finCarrera ) {
    val = 1;
    if (finCarrera || val == 1 ){
      moverAdelante();
      Serial.println("Subiendo manualmente");}
  } 
  else if (bajar && !subir) {
    val = 0;
    moverAtras();
    Serial.println("Bajando manualmente");
  } 
  else {
    val = 0;
    detenerMotor();
  }
  Serial.print("Ángulo actual: ");
  Serial.print(grados, 2);
  Serial.print("° | Vueltas: ");
  Serial.print(vueltas);
  Serial.print(" | Distancia estimada: ");
  Serial.print(distancia_mm, 2);
  Serial.println(" mm");
  delay(200);
}

// -------- Funciones de motor --------
void moverAdelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void moverAtras() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void detenerMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}


