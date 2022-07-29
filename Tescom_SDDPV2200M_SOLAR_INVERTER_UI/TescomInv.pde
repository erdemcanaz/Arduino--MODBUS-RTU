class TescomInv {
  int ID = 0;
  //int nom_power_02_01 = 0; // kW*10
  //int nom_freq_02_02 = 0; // Hz*100
  //int nom_rpm_02_03 = 0;  // rpm
  //int nom_voltage_02_04 = 0; // V_rms
  //int nom_current_02_05 = 0; // I_rms*10

  int inverter_temp_07_12 = 0; // c*10
  boolean ask_07_12 = false;
  int ref_frequency_17_00=0; // Hz*100
  boolean ask_17_00 = false;
  int out_frequency_17_01=0; // Hz*100
  boolean ask_17_01 = false;
  int out_voltage_17_03=0;   // V
  boolean ask_17_03 = false;
  int out_current_17_04=0;   // Irms*10
  boolean ask_17_04=false;
  int motor_rpm_17_05=0;   // V
  boolean ask_17_05 =false;
  int dc_bar_voltage_17_11=0; //
  boolean ask_17_11 = false;
  int input_grid_current_17_35=0;
  boolean ask_17_35 = false;

  boolean shouldRun = false;


  public TescomInv(int ID) {
    this.ID = ID;
  }

  void draw_ui() {
    //Start-stop
    strokeWeight(2);
    textSize(22);
    fill(255);
    stroke(0);
    rect(0, 0, 200, 30);
    if (shouldRun) {

      fill(0, 255, 0);
      text("Running", 50, 22);
    } else {
      fill(255, 0, 0);
      text("Stopped", 50, 22);
    }
    //inv_temp
    fill(255);
    stroke(0);
    rect(0, 30, 200, 30);
    fill(255, 0, 0);
    rect(0, 30, 200.0*((inverter_temp_07_12/10.0)/50.0), 30);
    if (ask_07_12)fill(0);
    else fill(125, 125, 125);
    text(inverter_temp_07_12/10.0+"C", 75, 52);
    //ref_freq
    fill(255);
    stroke(0);
    rect(0, 60, 200, 30);
    fill(255, 0, 0);
    rect(0, 60, 200.0*((ref_frequency_17_00/100.0) / 50.0), 30);
    if (ask_17_00)fill(0);
    else fill(125, 125, 125);
    text("Ref freq: "+((ref_frequency_17_00)/100.0)+" Hz", 25, 82);
    //out_freq
    fill(255);
    stroke(0);
    rect(0, 90, 200, 30);
    fill(255, 0, 0);
    rect(0, 90, 200.0*((out_frequency_17_01/100)/ 50.0), 30);
    if (ask_17_01)fill(0);
    else fill(125, 125, 125);
    text("Out freq: "+(out_frequency_17_01/100.0)+" Hz", 25, 112);

    //out_voltage
    fill(255);
    stroke(0);
    rect(0, 120, 200, 30);
    fill(255, 0, 0);
    rect(0, 120, 200.0*(out_voltage_17_03/ 300.0), 30);
    if (ask_17_03)fill(0);
    else fill(125, 125, 125);
    text("Out Voltage: "+(out_voltage_17_03)+" Vrms", 0, 142);

    //out_current
    fill(255);
    stroke(0);
    rect(0, 150, 200, 30);
    fill(255, 0, 0);
    rect(0, 150, 200.0*(out_current_17_04/ 125.0), 30);
    if (ask_17_04)fill(0);
    else fill(125, 125, 125);
    text("Out Current: "+(out_current_17_04/10.0)+" Irms", 0, 172);

    //motor_rpm
    fill(255);
    stroke(0);
    rect(0, 180, 200, 30);
    fill(255, 0, 0);
    rect(0, 180, 200.0*(motor_rpm_17_05/ 3500.0), 30);
    if (ask_17_05)fill(0);
    else fill(125, 125, 125);
    text("Rpm: "+(motor_rpm_17_05), 0, 202);

    //DC_bar_voltage
    fill(255);
    stroke(0);
    rect(0, 210, 200, 30);
    fill(255, 0, 0);
    rect(0, 210, 200.0*((dc_bar_voltage_17_11/10.0)/ 500), 30);
    if (ask_17_11)fill(0);
    else fill(125, 125, 125);
    text("DC bar voltage: "+(dc_bar_voltage_17_11/10.0)+"V", 0, 232);
  }

  long tic = 0;
  int counter = 0;
  String write() {
    String r_string = "";
    if (millis()-tic < 200)return "";
    tic = millis();


    counter= counter %8;
    if (    counter == 0 ) {
      if (shouldRun) r_string = ("15,6,32,0,0,1\n");
      else  r_string = ("15,6,32,0,0,5\n");
    } else if ( counter == 1) {
      if (ask_07_12) {
        r_string = ("15,3,7,12,0,1\n");
      } else {
        counter++;
        tic=0;
      }
    } else if ( counter == 2) {
      if (ask_17_00) {
        r_string = ("15,3,17,0,0,1\n");
      } else {
        counter++;
        tic=0;
      }
    } else if ( counter == 3) {
      if (ask_17_01) {
        r_string = ("15,3,17,1,0,1\n");
      } else {
        counter++;
        tic=0;
      }
    } else if ( counter == 4) {
      if (ask_17_03) {
        r_string = ("15,3,17,3,0,1\n");
      } else {
        counter++;
        tic=0;
      }
    }else if ( counter == 5) {
      if (ask_17_04) {
        r_string = ("15,3,17,4,0,1\n");
      } else {
        counter++;
        tic=0;
      }
    }else if ( counter == 6) {
      if (ask_17_05) {
        r_string = ("15,3,17,5,0,1\n");
      } else {
        counter++;
        tic=0;
      }
    }else if ( counter == 7) {
      if (ask_17_11) {
        r_string = ("15,3,17,11,0,1\n");
      } else {
        counter++;
        tic=0;
      }
    }
    
    
    return r_string;
  }


  void read(String reply) {
    String[] arrOfStr = reply.split(",");
    println(arrOfStr);
    if (counter == 1 && arrOfStr.length == 5) {
      int sig_byte = Integer.valueOf(arrOfStr[3]);
      int least_byte = Integer.valueOf(arrOfStr[4]);
      inverter_temp_07_12 = (sig_byte<<8) + least_byte;
    } else if (counter == 2 && arrOfStr.length == 5) {
      int sig_byte = Integer.valueOf(arrOfStr[3]);
      int least_byte = Integer.valueOf(arrOfStr[4]);
      ref_frequency_17_00 = (sig_byte<<8) + least_byte;
    } else if (counter == 3 && arrOfStr.length == 5) {
      int sig_byte = Integer.valueOf(arrOfStr[3]);
      int least_byte = Integer.valueOf(arrOfStr[4]);
      out_frequency_17_01 = (sig_byte<<8) + least_byte;
    } else if (counter == 4 && arrOfStr.length == 5) {
      int sig_byte = Integer.valueOf(arrOfStr[3]);
      int least_byte = Integer.valueOf(arrOfStr[4]);
      out_voltage_17_03 = (sig_byte<<8) + least_byte;
    }else if (counter == 5 && arrOfStr.length == 5) {
      int sig_byte = Integer.valueOf(arrOfStr[3]);
      int least_byte = Integer.valueOf(arrOfStr[4]);
      out_current_17_04 = (sig_byte<<8) + least_byte;
    } else if (counter == 6 && arrOfStr.length == 5) {
      int sig_byte = Integer.valueOf(arrOfStr[3]);
      int least_byte = Integer.valueOf(arrOfStr[4]);
      motor_rpm_17_05 = (sig_byte<<8) + least_byte;
    }else if (counter == 7 && arrOfStr.length == 5) {
      int sig_byte = Integer.valueOf(arrOfStr[3]);
      int least_byte = Integer.valueOf(arrOfStr[4]);
      dc_bar_voltage_17_11 = (sig_byte<<8) + least_byte;
    }else {
    }


    counter++;
  }

  void analyze_click(int mouse_x, int mouse_y) {
    if (mouse_y>0 && mouse_y<30)shouldRun=!shouldRun;
    if (mouse_y>30&& mouse_y<60)ask_07_12=!ask_07_12;
    if (mouse_y>60&& mouse_y<90)ask_17_00=!ask_17_00;
    if (mouse_y>90&& mouse_y<120)ask_17_01=!ask_17_01;
    if (mouse_y>120&& mouse_y<150)ask_17_03=!ask_17_03;
    if (mouse_y>150&& mouse_y<180)ask_17_04=!ask_17_04;
    if (mouse_y>180&& mouse_y<210)ask_17_05=!ask_17_05;
    if (mouse_y>210&& mouse_y<240)ask_17_11=!ask_17_11;
  }
}
