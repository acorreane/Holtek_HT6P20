#include <EEPROM.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(9, 10, 11, 14, 15, 16);

byte index = EEPROM.read(128);

byte startbit =0;
byte antcode  =0;
byte dataok   =0;
byte exec     =0;

int ctr       =0;
const int onebit    =300;
const int zerobit   =600;

 
int txctr =  28;  
int txctr2 = 26;  
int clock =  0;   
int clockon =  500;   
int pilot = 10000;   
int txstartbit = 500;   
int buffer = 0; 
int txexec = 0; 
int trxm = 0;
int push = 0;
int mode = 0;
int list = 0;

unsigned long dur,dur1  =0;     
unsigned long data      =0;

byte escb = 5;
byte modb = 6;
byte selb = 8;

byte lcdp = 12;
byte trxp = 3;

byte rcvd = 7;
byte trxd = 2;

byte sysp = 13;
byte rxdl = 4;

void setup(){
  
//   --BUTTONS--
  pinMode(escb, INPUT_PULLUP);      // ESC/ENTER
  pinMode(modb, INPUT_PULLUP);      // MODE
  pinMode(selb, INPUT_PULLUP);      // SELECT
  
//   --POWER INTERFACES--  
  pinMode(lcdp, OUTPUT);     // LCD MODULE POWER
  pinMode(trxp, OUTPUT);     // TRANSMITTER PTT
  pinMode(sysp, OUTPUT);     // SYS DIAG
  pinMode(rxdl, OUTPUT);     // RXD DIAG
  
//   --DATA INTERFACES-- 
  pinMode(trxd, OUTPUT);     // TRANSMITTER DATA OUTPUT
  pinMode(rcvd, INPUT);      // RF IN SIGNAL DATA
  
//   --PINOUT INIT--
  digitalWrite (lcdp, HIGH); // TURN ON LCD DISPLAY
  digitalWrite (sysp, HIGH);
  
//   --PERIPHERAL INIT--
  delay(500);
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  delay(1000);
  
  lcd.setCursor(0, 0);
  lcd.print("Init...");
  delay(1000);
}

void loop(){
 lcd.setCursor(0, 0);
 lcd.print("SELECT MODE: ");
 
  if (digitalRead(modb) == 0){
   if (push == 0){
     lcd.setCursor(14, 0);
     lcd.print("RX");
       push = 1;
       mode = 0;
       goto END;
   }
    if (push == 1){
     lcd.setCursor(14, 0);
     lcd.print("TX");
       push = 0;
       mode = 1;
       goto END; 
    }
  }
   
   if (digitalRead(escb) == 0){
     if (mode == 0){
       lcd.setCursor(0, 0);
       lcd.print("   SCAN  MODE   ");
       lcd.setCursor(0, 1);
       lcd.print("waiting data...");
       rxmode();
     }
     if (mode == 1){
       lcd.setCursor(0, 0);
       txmode();
     }
   }
   END:
   delay(500);
}

void rxmode(){
  
 MAIN:  
 if (startbit==0){
 dur = pulseIn(rcvd, LOW);
 digitalWrite(rxdl, digitalRead(rcvd));
  if(dur > 8000 && dur < 12000 && startbit==0){    
    startbit=1;
        dur=0;
       data=0;
      dataok=0;
         ctr=0;
  }
 }
 if (startbit==1 && dataok==0 && ctr < 28){ 
    ++ctr;
    dur1 = pulseIn(rcvd, HIGH);
   if(dur1 > onebit && dur1 < onebit*2)     // Se a largura de pulso é entre 1/4000 e 1/3000 segundos
   {
     data = (data << 1) + 1;      // anexar um * 1 * para a extremidade mais à direita do buffer
   }
   else if(dur1 > zerobit && dur1 < zerobit*2)   // Se a largura de pulso é entre 2/4000 e 2/3000 segundos
   {
     data = (data << 1);       // anexar um * 0 * para a extremidade mais à direita do buffer
   }
   else
   {
     startbit = 0;
   }    
  }
  if (ctr==28){
    if (bitRead(data,0)==1)
         { 
           if (bitRead(data,1)==0)
              {
                if (bitRead(data,2)==1)
                   {
                     if (bitRead(data,3)==0)
                        {
                          antcode=1;
                        }
                   }
              }
         }    


  if (antcode==1){
      dataok=1;
      lcd.setCursor(0, 0);
      lcd.print("   DECODE  OK   ");
      index++;
      if (index > 3){
      index = 0;
      EEPROM.write(128, index);
      }
      else{ EEPROM.write(128, index);}
      if (index == 0){
      eepromwrite(0, data);
      }
      if (index == 1){
      eepromwrite(4, data);
      }
      if (index == 2){
      eepromwrite(8, data);
      }
      if (index == 3){
      eepromwrite(12, data);
      }
      delay(500);
      
      ctr=0;
      startbit=0;
      antcode=0;
      list = index;
      txmode();
      }
     
     if (antcode==0){
     reset();
    } 
  }
  goto MAIN;
}


void txmode(){
  
 lcd.setCursor(0, 0);
 lcd.print("TX MODE   IDX: ");
 lcd.print(index);
 lcd.setCursor(0, 1);
 lcd.print("CODE:  ");
 if (data == 0){
 lcd.print("         ");
 }
 else{
 lcd.print(data); //debug
 }
 MAIN:
 if (dataok == 1){
 dataok = 0;
 goto SEL;
 }
 if (digitalRead(selb) == 0){
   
 list++;
 if (list > 3){
 list = 0;
 }
 SEL:
 if (list == 0){
 data = eepromread(0);
 lcd.setCursor(15, 0);
 lcd.print(list);
 lcd.setCursor(7, 1);
 lcd.print(data);
  }
 if (list == 1){
 data = eepromread(4);
 lcd.setCursor(15, 0);
 lcd.print(list);
 lcd.setCursor(7, 1);
 lcd.print(data);
  }
 if (list == 2){
 data = eepromread(8);
 lcd.setCursor(15, 0);
 lcd.print(list);
 lcd.setCursor(7, 1);
 lcd.print(data);
  }
 if (list == 3){
 data = eepromread(12);
 lcd.setCursor(15, 0);
 lcd.print(list);
 lcd.setCursor(7, 1);
 lcd.print(data);
  }
 }
 delay(500);
 
 if (digitalRead(escb) == 0){
 lcd.setCursor(0, 1);
 lcd.print("Transmitting... ");
 trxm = 0;
 digitalWrite(trxp, HIGH);
 txd();
 }
 goto MAIN;
}
   
 void txd(){ 
 END:
 if (trxm >= 20){
 trxm = 0;
 digitalWrite(trxp, LOW);
 txmode();
 } 
 MAIN:
 if (txctr == 28){              
 digitalWrite (trxd, LOW);         
 delayMicroseconds (pilot);   
 digitalWrite (trxd, HIGH);     
 delayMicroseconds (txstartbit); 
 digitalWrite (trxd, LOW);      
 delayMicroseconds (clockoff); 
 --txctr;                        
 // pilot period, startbit ok
 }
 // transmit data
 TX:
 buffer = bitRead(data,txctr);   
 if (buffer == 1){             
   clock = clockon;
 }
 if (buffer == 0){              
   clock = clockoff;
 }
 digitalWrite (trxd, HIGH);    
 delayMicroseconds (clock);     
 digitalWrite (trxd, LOW);      
 buffer = bitRead(data,txctr2);  
 if (buffer == 1){             
   clock = clockoff;
 }
 if (buffer == 0){             
   clock = clockon;
 }
 delayMicroseconds (clock);   
 
 if (txexec == 0){               
 --txctr;                        
 --txctr2;                        
 }
 if (txexec == 1){               
   txexec = 0;                    
   txctr = 28;                    
   txctr2 = 26;                   
   trxm++;
   goto END;                   
 }
 
 if (txctr == 0){                
   txexec = 1;                    
   goto TX;                     
 }
 goto MAIN;
}

void reset()
{
 lcd.clear();
 dataok=0;
 ctr=0;
 startbit=0;
 antcode=0;
 delay(500);
}
  
void eepromwrite(int address, unsigned long value){
   union u_tag {
     byte b[4];
     long fval;
   } u;
   u.fval=value;
 
   EEPROM.write(address  , u.b[0]);
   EEPROM.write(address+1, u.b[1]);
   EEPROM.write(address+2, u.b[2]);
   EEPROM.write(address+3, u.b[3]);
}

unsigned long eepromread(int address){
   union u_tag {
     byte b[4];
     long fval;
   } u;   
   u.b[0] = EEPROM.read(address);
   u.b[1] = EEPROM.read(address+1);
   u.b[2] = EEPROM.read(address+2);
   u.b[3] = EEPROM.read(address+3);
   return u.fval;
}