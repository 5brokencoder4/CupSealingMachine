// ------------------ Röle Tanımlamaları ------------------
#define RELAY_COIL         3   // Bobin rölesi
#define RELAY_TABLE        4   // Tabla hareket rölesi
#define RELAY_WRAP         5   // Ambalaj rölesi
#define RELAY_KNIFE        6   // Bıçak rölesi

// ------------------ Sensör Girişleri ---------------------
#define SENSOR_METAL       12  // Metal sensörü
#define SWITCH_KNIFE_OPEN   2  // Bıçak açma switch
#define SWITCH_KNIFE_CLOSE  7  // Bıçak kapama switch

// ------------------ Manuel Anahtarlar --------------------
#define SWITCH_AUTONOM     10  // Otonom mod anahtarı
#define SWITCH_TABLE        9  // Tabla hareket anahtarı
#define SWITCH_KNIFE        8  // Bıçak kontrol anahtarı

// ------------------ Analog Girişler -----------------------
#define LDR_SENSOR         A0
#define POT_DURATION       A2

// ------------------ Zamanlayıcı ve Durum Değişkenleri ------------------
unsigned long wrap_duration;
unsigned long t1, t2, t3;
bool metal_detected, knife_open, knife_close;
bool autonomous_switch, table_switch, knife_switch;
bool initial_table_move = true;
bool autonomous_active = false;
bool autonomous_first_press = true;

int ldr_value, pot_value;

void rotateTable() {
  t3 = millis();
  digitalWrite(RELAY_TABLE, LOW);
  delay(50);

  while (analogRead(LDR_SENSOR) < 200) {
    digitalWrite(RELAY_TABLE, LOW);
    delay(15);
  }

  while (analogRead(LDR_SENSOR) > 200) {
    delay(15);
  }

  digitalWrite(RELAY_TABLE, HIGH);
}

void setup() {
  Serial.begin(9600);

  pinMode(SENSOR_METAL, INPUT);
  pinMode(SWITCH_KNIFE_OPEN, INPUT_PULLUP);
  pinMode(SWITCH_KNIFE_CLOSE, INPUT_PULLUP);
  pinMode(SWITCH_AUTONOM, INPUT_PULLUP);
  pinMode(SWITCH_TABLE, INPUT_PULLUP);
  pinMode(SWITCH_KNIFE, INPUT_PULLUP);

  pinMode(RELAY_COIL, OUTPUT);
  pinMode(RELAY_KNIFE, OUTPUT);
  pinMode(RELAY_TABLE, OUTPUT);
  pinMode(RELAY_WRAP, OUTPUT);

  digitalWrite(RELAY_COIL, HIGH);
  digitalWrite(RELAY_KNIFE, HIGH);
  digitalWrite(RELAY_TABLE, HIGH);
  digitalWrite(RELAY_WRAP, HIGH);
}

void loop() {
  ldr_value = analogRead(LDR_SENSOR);
  pot_value = map(analogRead(POT_DURATION), 0, 1023, 500, 1100);

  metal_detected = digitalRead(SENSOR_METAL);
  knife_switch = digitalRead(SWITCH_KNIFE);
  table_switch = digitalRead(SWITCH_TABLE);
  autonomous_switch = digitalRead(SWITCH_AUTONOM);
  knife_open = digitalRead(SWITCH_KNIFE_OPEN);
  knife_close = digitalRead(SWITCH_KNIFE_CLOSE);

  if ((millis() - t1) > 1000) {
    Serial.print("LDR = "); Serial.println(ldr_value);
    Serial.print("Metal = "); Serial.println(metal_detected);
    Serial.print("Knife = "); Serial.println(knife_switch);
    Serial.print("Table = "); Serial.println(table_switch);
    Serial.print("Autonomous = "); Serial.println(autonomous_switch);
    Serial.print("Open Switch = "); Serial.println(knife_open);
    Serial.print("Close Switch = "); Serial.println(knife_close);
    Serial.print("Potentiometer = "); Serial.println(pot_value);
    Serial.println("-----------------------------");
    t1 = millis();
  }

  if (initial_table_move) {
    rotateTable();
    initial_table_move = false;
  }

  if (!table_switch) rotateTable();

  if (autonomous_switch == LOW) {
    if (autonomous_first_press && knife_switch == LOW) {
      autonomous_active = true;
      autonomous_first_press = false;
    }
  } else {
    autonomous_first_press = true;
    autonomous_active = false;
  }

  int knife_check = 0;
  for (int i = 0; i < 2; i++) {
    knife_check += digitalRead(SWITCH_KNIFE);
    delay(20);
  }
  knife_switch = (knife_check == 0) ? LOW : HIGH;

  if (knife_switch == LOW || autonomous_active) {
    digitalWrite(RELAY_KNIFE, LOW);
    digitalWrite(RELAY_COIL, LOW);

    while (knife_open) {
      knife_open = digitalRead(SWITCH_KNIFE_OPEN);
      delay(20);
    }
    while (!knife_close) {
      knife_close = digitalRead(SWITCH_KNIFE_CLOSE);
      delay(20);
    }

    digitalWrite(RELAY_COIL, HIGH);
    digitalWrite(RELAY_KNIFE, HIGH);
    delay(1000);

    digitalWrite(RELAY_KNIFE, LOW);
    while (knife_close) {
      knife_close = digitalRead(SWITCH_KNIFE_CLOSE);
      delay(20);
    }
    while (!knife_open) {
      knife_open = digitalRead(SWITCH_KNIFE_OPEN);
      delay(20);
    }
    digitalWrite(RELAY_KNIFE, HIGH);

    delay(500);

    digitalWrite(RELAY_WRAP, LOW);
    delay(300);
    digitalWrite(RELAY_TABLE, LOW);
    wrap_duration = millis();
    digitalWrite(RELAY_WRAP, LOW);

    do {
      delay(30);
    } while (millis() - wrap_duration < pot_value);

    digitalWrite(RELAY_WRAP, HIGH);
    rotateTable();
  }
}
