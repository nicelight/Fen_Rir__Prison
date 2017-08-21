// по кнопке "старт "включается плеер, релюшкой перебрасываются каналы усилителя
// на динамики тюрьмы.
//по сработке концевика каналы усилка перебрасываются на библиотеку,
// плеер 1 - некст трек.
//по приходу сигнала с библиотеки, каналы усилка перебрасываются на тюрьму,
// перезарядка, включение плеера на 5 секунд, некст трек, некст трек и пауза, релюшка на каналы тюрьмы
#define A7 =S_G_O_R_E_L!!!!!! // не подключать аналоговые пины как входы, горят ((
#define A6 =SGOREL!!!!!!

#define RECHARGE 12 // перезарядка 
#define START 4    // кнопка старт
#define SIGNAL_FROM_LIB A5 // с библиотеки поступает сигнал ("0") о том что
                          // пора бы переключить фон с тюрьмы на библиотеку 
#define KONCEVIK A4 // концевик с двери для переключения плееров

#define CHANGE_CHANNEL 2 // переключение колонок на соседнюю комнату

#define LAMP1 3
#define LAMP2 5
#define LAMP3 9
#define LAMP4 10
#define LAMP5 11

#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
SoftwareSerial mySerial(7, 6); // RX, TX

int inByte = 0;         // incoming serial byte
int i = 0; // переменная для цикла перебора повторяющихся автоматов.
unsigned long currentMillis=0; // переменная для сравнения счетчика милисикунд с внутренним счетчиком AVR
byte game_over=1; // флаг что игру прошли

//переменные главного автомата программы
byte auto1=0; // гланый автомат на подготовку к перезарядке
byte auto1_temp=0; // гланый автомат на подготовку к перезарядке

unsigned long del_auto1=0; // милисекундная задержка между автоматами
unsigned long prevMillisauto1=0; // милисекундный счетчик для задержки автомата 1
// переменные автоматаотв lamp[i] ( эффект свечи) 
byte lamps=0; // автомат ламп (пины 3, 6)
byte fakels=0; //автомат факелов (пины 3, 6, 9);
byte bright_fakel1=0; // очередная яркость 
byte bright_fakel2=0; // очередная яркость 
byte bright_fakel3=0; // очередная яркость 
byte bright_lamp1=0; // очередная яркость 
byte bright_lamp2=0; // очередная яркость 
unsigned long del_lamps=10; // задержка в милисекундах между переходами состояний мерцания лам
unsigned long del_fakels=10; // задержка в милисекундах между мерцаниями факелов
unsigned long prevMillislamps=0; // счетчик милисекундный для проверки не прошло ли еще время задержки
unsigned long prevMillisfakels=0; // счетчик милисекундный для проверки не прошло ли еще время задержки
unsigned long prevMslastfakels=0; // счетчик милисекундный для проверки не прошло ли еще время задержки



void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);  
  mySerial.begin (9600);
delay(100);
  mp3_set_serial (mySerial);  //set softwareSerial for DFPlayer-mini mp3 module 
  mp3_set_volume (25);
  mp3_stop();
pinMode(START, INPUT_PULLUP);
pinMode(RECHARGE, INPUT_PULLUP);
pinMode(SIGNAL_FROM_LIB, INPUT_PULLUP);
pinMode(KONCEVIK, INPUT_PULLUP);

pinMode(CHANGE_CHANNEL, OUTPUT); 
digitalWrite(CHANGE_CHANNEL, 1); 

}// setup

void loop() {
 currentMillis = millis(); // объявляем переменную для определения текущего кол-ва милисекунд


 // **************** отладка по парсингу кнопок клавиатуры 1-9 ****************//
 if (Serial.available() > 0) { // парсинг цифр с серийного порта для управления
    // get incoming byte:
    inByte = Serial.read();
    if ((inByte-48)==1) { // парсинг цифры 1 ( по таблице ASCI )
      Serial.println("Fon paused");

    }//if 1
    if ((inByte-48)==2) { // парсинг цифры 2 ( по таблице ASCI )
      Serial.println("Fon NEXT");
      
    }//if 
    if ((inByte-48)==3) { // парсинг цифры 3 ( по таблице ASCI )
      Serial.println("Fon NEXT and PAUSED.   Complex!");
     
    }//if 
    if ((inByte-48)==4) { // парсинг цифры 3 ( по таблице ASCI )
      Serial.println("Speakers for prison");
     digitalWrite(CHANGE_CHANNEL, 1);
    }//if 
    if ((inByte-48)==5) { // парсинг цифры 3 ( по таблице ASCI )
      Serial.println("Speakers for Library");
     digitalWrite(CHANGE_CHANNEL, 0);
    }//if 
 }// отладка клавиатуре

////////////////////////// ВЫЗОВ ПЕРЕЗАРЯДКИ ///////////////////////
// в основном цикле, доступен постоянно.
// если после нажатия перезарядки играет не фоновая музыка, нажать старт и перезарядка одновременно в течении 4 сек 
// с момента начала фонового звука.
// если игра не презаряжается, кнопка старт не удерживается, плеера не отрабатывают, можно дать команду на перезарядку

if((!digitalRead(RECHARGE))&&(auto1<60)){
  delay(50);
  if(!digitalRead(RECHARGE)){
    Serial.println(" !!!! RECHARGING !!!!");
    auto1=60; // го на перезарядку
  }// if 2
}//if recharge
//отладка: вывод изменений состояний автоматов


if(auto1_temp!=auto1){
  auto1_temp=auto1;
  Serial.print("auto1=");
  Serial.println(auto1);
}// печать состояний автомата

////***************  ГЛАНВЫЙ АВТОМАТ  *********************////////
switch (auto1){ // automat 1 - main
/* образцовый кейс
case xx: //ожидание переключения плееров по концевику
  if (currentMillis - prevMillisauto1 >= del_auto1) { 
    prevMillisauto1 = currentMillis; //сброс милисекундного счетчика
  }
break;
*/

case 0: //инициализация работы
    del_auto1=100; // задержка в мс для переключения в следующее состояние автомата
    prevMillisauto1 = currentMillis;
    auto1=1; // го на подготовку к старту
break;
case 1: // подготовка к старту, проверка концевика.
  if(digitalRead(KONCEVIK)){ // проверка поднятой двери-моста
    delay(20);
    if(digitalRead(KONCEVIK)){ // проверка поднятой двери-моста
      analogWrite(LAMP3, 255); // мигаем если не поднята дверь
       delay(300);
      analogWrite(LAMP3, 0);
      delay(200);
    }//1 if
  }else if(!digitalRead(SIGNAL_FROM_LIB)){ // если с библиотеки идет сигнал, она не перезаряжена
    delay(20);
    if(!digitalRead(SIGNAL_FROM_LIB)){
      analogWrite(LAMP4, 255); // мигаем если не поднята дверь
      analogWrite(LAMP5, 255); // мигаем если не поднята дверь
      delay(300);
      analogWrite(LAMP4, 0);
      analogWrite(LAMP5, 0);
      delay(200);
    }
  } else { // иначе на ожидание нажатия кнопки старт
    prevMillisauto1 = currentMillis;
    del_auto1=3000;
    auto1=4; // го на подготовку к старту
  }
break;
case 4: //ожидание нажатия кнопки СТАРТ
  if (currentMillis - prevMillisauto1 >= del_auto1) { 
    prevMillisauto1 = currentMillis; //сброс милисекундного счетчика
    del_auto1=100;
    if(!digitalRead(START)){
      delay(20);
      if(!digitalRead(START)){
        Serial.println(" !!!!! STARTED !!!! ");
        mp3_play(1);
        game_over=0; // игра началась - сброс флага (чтобы ориентироваться, музыка сейчас играет или нет) 
        lamps=1; // свет в рабочий режим
        fakels=1; // факелы в рабочий режим
        auto1=6;
      }// if del
    }//if start     
  }//if ms
break;
case 6: //ожидание переключения плееров по концевику
if(digitalRead(KONCEVIK)){ // если сработал концевик 
      delay(20);
      if(digitalRead(KONCEVIK)){
    digitalWrite(CHANGE_CHANNEL, 0); // перебрасываем колонки усилителя с тюрьмы на библиотеку
    for( int i=220; i>20; i--){
      //спецэффект!!!
      analogWrite(LAMP1, i); //яркая вспышка в комнате, как только мост опустили
      analogWrite(LAMP2, i); // с затуханием
      analogWrite(LAMP3,(220-i)); // а факел разгорается
        delay(5);
    }
    mp3_play(2);
    auto1=10; 
    fakels=10;   // факела на яркий третий факел
      }//if del
}// if koncevik
break;
case 10: // ожидание остановки фона библиотеки
if(!digitalRead(SIGNAL_FROM_LIB)){
  delay(100);
  if(!digitalRead(SIGNAL_FROM_LIB)){
    digitalWrite(CHANGE_CHANNEL, 1); // перебрасываем колонки усилителя с библиотеки на тюрьму
    delay(300);
    mp3_stop(); // музыка выключается
    game_over=1; // игра закончена
    Serial.println("game over");
    auto1=12; // 
    prevMillisauto1 = currentMillis; //сброс милисекундного счетчика
    del_auto1=4000;
  }
}//if if from lib
break;

case 12: // перезарядка через 5 минут
prevMillisauto1 = currentMillis;
break;


case 60: // ПЕРЕЗАРЯДКА 
  lamps=0; // включаем свет на полную
  fakels=0;// включаем факелы на полную
  digitalWrite(CHANGE_CHANNEL, 1); // перебрасываем колонки усилителя с библиотеки на тюрьму
  prevMillisauto1 = currentMillis;
  del_auto1=100;
  mp3_stop();
    analogWrite(LAMP3, 250); // мигаем мол перезаряжены
    analogWrite(LAMP4, 250);
    analogWrite(LAMP5, 250);
    delay(500);
    analogWrite(LAMP3, 0);
    analogWrite(LAMP4, 0);
    analogWrite(LAMP5, 0);
    delay(200);
    analogWrite(LAMP3, 250);
    analogWrite(LAMP4, 250);
    analogWrite(LAMP5, 250);
    delay(400);
    analogWrite(LAMP3, 0);
    analogWrite(LAMP4, 0);
    analogWrite(LAMP5, 0);
    delay(200);
    analogWrite(LAMP3, 250);
    analogWrite(LAMP4, 250);
    analogWrite(LAMP5, 250);
    delay(200);
    analogWrite(LAMP3, 0);
    analogWrite(LAMP4, 0);
    analogWrite(LAMP5, 0);
    delay(100);
    
  auto1=1; // го напроверку откидной двери
break;

case 80: // ожидание 
prevMillisauto1 = currentMillis;
break;
}// switch(auto1)


///////////////////////////////////////////////////
///////////////// ЛАМПЫ ///////////////////////////
///////////////////////////////
  
switch(fakels){// автомат для свечения факелов в тюрьмах
 case 0: // перезарядка
    analogWrite(LAMP3, 255);
    analogWrite(LAMP4, 255);
    analogWrite(LAMP5, 255);
 break;
 case 1:
 prevMillisfakels=currentMillis;
 fakels=2;
 del_fakels=100;
 break;
 case 2: // игровой свет
  if (currentMillis - prevMillisfakels>= del_fakels) { 
    prevMillisfakels=prevMslastfakels=currentMillis;
    if((random(1, 100))==1) {// раз в 40 тактов потухнем. 
      bright_fakel1=random(0, 15); // яркость в ноль
      bright_fakel2=random(0, 15); // яркость в ноль
      bright_fakel3=random(0, 15); // яркость в ноль
      del_fakels=random(200, 500); // задержка длинная
    }else{ //остальные такты горим
      bright_fakel1=random(10, 20); // выбор следующей яркости(random( min до max))
      bright_fakel2=random(10, 20); // выбор следующей яркости(random( min до max))
      bright_fakel3=random(10, 20); // выбор следующей яркости(random( min до max))
      del_fakels=random(150, 300); // задержка между переходами яркостей
    }//else
    analogWrite(LAMP3, bright_fakel1);
    analogWrite(LAMP4, bright_fakel2);
    analogWrite(LAMP5, bright_fakel3);
    
  }//if ms
 break;
 case 10: // третья лампа пин 9 горит ярко, остальные мигают 30 секунд 
  if (currentMillis - prevMillisfakels>= del_fakels) { 
    prevMillisfakels=currentMillis;
    del_fakels=random(150, 300); // задержка между переходами яркостей
    analogWrite(LAMP3, random(150, 200));
    analogWrite(LAMP4, random(60, 70));
    analogWrite(LAMP5, random(60, 70));
    if (currentMillis-prevMslastfakels>30000) { // через 30 секунд уходим с кейса 10 - яркого горения факела 3
      fakels=1; // го на тусклое мигание
    }
  }//if ms
  break;
  case 80:
   prevMillisfakels=currentMillis;
   break;
}//switch fakels
  
switch(lamps){// автомат для свечения лампам в маленькой комнате
  case 0:
    analogWrite(LAMP1, 255);
    analogWrite(LAMP2, 255);
 break;
 case 1:
 prevMillislamps=currentMillis;
 del_lamps=100;
 lamps=2;
  break;
 case 2: // игровой свет
  if (currentMillis - prevMillislamps>= del_lamps) { 
    prevMillislamps=currentMillis;
    if((random(1, 60))==1) {// раз в 40 тактов мигнем
      bright_lamp1=random(0, 20); // яркость в максимум
      bright_lamp2=random(0, 20); // яркость в максимум
      del_lamps=random(300, 500); // задержка длинная
    }else{    //остальные такты горим
      bright_lamp1=random(1, 2); // выбор следующей яркости(random( min до max))
      bright_lamp2=random(1, 2); // выбор следующей яркости(random( min до max))
      del_lamps=random(150, 300); // задержка между переходами яркостей
    }//else
    analogWrite(LAMP1, bright_lamp1);
    analogWrite(LAMP2, bright_lamp2);
  }//if ms
 break;
 
 case 80:
 break;
}//switch lsmps









}// loop
