

void setup() {
  delay(5000);
  Serial.begin(9600);
  configure_lora_node();
}

unsigned long tic = 0;
void loop() {   
   
  operate_LoRa_node();
  
  /*if(millis()-tic >5000){
    print_queries();
    tic = millis();
  }*/
  
}
