
void setup() {
  Serial.begin(9600);
  configure_master();
}

void loop() {
  master_operate();
}

//254,6,0,0,0,1  15,6,32,0,0,1
//15,6,32,0,0,1
