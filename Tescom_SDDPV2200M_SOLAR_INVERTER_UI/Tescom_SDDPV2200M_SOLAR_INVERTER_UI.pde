import processing.serial.*;
Serial myPort;
TescomInv inv = new TescomInv(15);

void setup() {
  size(200, 240);
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, Serial.list()[0], 9600);
  background(255);
  frameRate(30);
}


boolean should_run = false;


void draw() {
  String a= inv.write();
  if (a!= "")myPort.write(a);

  inv.draw_ui();



  if (myPort.available()>0) {   
    delay(10);
    String reply = "";
    while (myPort.available()>0) {
      char c =  (char) myPort.read();
      if (!((c>='0' && c<='9') || c==','))continue;
      reply += String.valueOf(c);
    }
    //println(millis(),reply);
    inv.read(reply);
    myPort.clear();
  }
}

void mouseClicked() {
  inv.analyze_click(mouseX, mouseY);
  //should_run = !should_run;
}
