const int Sensor=A0;
const int fan=3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(fan, OUTPUT);
  pinMode(Sensor, INPUT);
  delay(1000); // Wait for 1 second before starting the loop
}

void loop() {
  // put your main code here, to run repeatedly:
    int nilaiAnalog=analogRead(Sensor);
    Serial.println(nilaiAnalog);
  delay(500);
}