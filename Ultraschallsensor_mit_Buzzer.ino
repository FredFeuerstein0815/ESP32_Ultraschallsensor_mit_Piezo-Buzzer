bool bufferVoll = false;
bool beepStatus = false;

const int TRIGGER = 19;
const int ECHO    = 18;
const int BUZZER  = 22;
const int PWM     = 21;
const int Mittelwert = 10;
const int Messintervall = 100;

int fehlerDistanz   = 7;
int maxDistanz      = 70;
int weiteDistanz    = 60;
int mittlereDistanz = 50;
int naheDistanz     = 40;
int minDistanz      = 30;
int freq_tiefste  = 800;
int freq_tief     = 1400;
int freq_mittel   = 1800;
int freq_hoch     = 2200;
int freq_max      = 3400;
int langsamste_wiederholung  = 500;
int langsame_wiederholung     = 400;
int mittlere_wiederholung   = 300;
int schnellere_wiederholung     = 100;
int maximale_wiederholung      = 0;
int messwerte[Mittelwert];
int messIndex = 0;
int distanz = 0;

unsigned long letzterBeep = 0;

void playTone(int freq, int volume, int interval) {
  unsigned long jetzt = millis();
  analogWrite(PWM, volume);
  if (interval == 0) {
    tone(BUZZER, freq);
    return;
  }
  if (jetzt - letzterBeep >= interval) {
    letzterBeep = jetzt;
    beepStatus = !beepStatus;
    if (beepStatus) tone(BUZZER, freq);
    else noTone(BUZZER);
  }
}

void Distanzmessung(void *pvParameters) {
  const TickType_t delayTicks = pdMS_TO_TICKS(Messintervall);
  for (;;) {
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER, LOW);
    long dauer = pulseIn(ECHO, HIGH, 30000);
    int aktuelleDistanz = dauer * 0.034 / 2;
    messwerte[messIndex] = aktuelleDistanz;
    messIndex++;
    if (messIndex >= Mittelwert) {
      messIndex = 0;
      bufferVoll = true;
    }
    int summe = 0;
    int anzahl = bufferVoll ? Mittelwert : messIndex;
    for (int i = 0; i < anzahl; i++) {
      summe += messwerte[i];
    }
    distanz = summe / anzahl;
//    Serial.print("Mittelwert: ");
//    Serial.println(distanz);
    if (distanz >= maxDistanz || distanz <= fehlerDistanz) {
      noTone(BUZZER);
      analogWrite(PWM, 0);
    }
    else if (distanz >= weiteDistanz) {
      playTone(freq_max, 255, langsamste_wiederholung);
      Serial.print("Objekt in ");
      Serial.print(distanz);
      Serial.print(" cm erkannt (weit)\n");
    }
    else if (distanz >= mittlereDistanz) {
      playTone(freq_max, 255, langsame_wiederholung);
      Serial.print("Objekt in ");
      Serial.print(distanz);
      Serial.print(" cm erkannt (mittel)\n");

    }
    else if (distanz >= naheDistanz) {
      playTone(freq_max, 255, mittlere_wiederholung);
      Serial.print("Objekt in ");
      Serial.print(distanz);
      Serial.print(" cm erkannt (nah)\n");
    }
    else if (distanz >= minDistanz) {
      playTone(freq_max, 255, schnellere_wiederholung);
      Serial.print("Objekt in ");
      Serial.print(distanz);
      Serial.print(" cm erkannt\n");
    }
    else {
      playTone(freq_max, 255, maximale_wiederholung);
    }
    vTaskDelay(delayTicks);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(PWM, OUTPUT);

  xTaskCreatePinnedToCore(
    Distanzmessung,
    "Distanzmessung_Task",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}

void loop() {}
