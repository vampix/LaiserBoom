#include <Arduino.h>
#include <LaserWar.h>
#include <LWShoot.h>
#include <LWCommand.h>

/*
Используем фототранзистор 940нм и подключаем с общим эмиттером.
Подключаем короткую ногу (коллектор) через резистор 1 КОм (или 10 КОм) к +5В
Эту же кототкую ногу (коллектор) подключаем к аналоговому пину, например, A0, напрямую
Длинную ногу (эмиттер) подключаем к GND
*/
LaserWar lw(A0); // Определяем ресивер на пине A0
LWShoot shoot;  // Класс для парсинга выстрелов
LWCommand cmd;  // Класс для парсинга команд

void setup(){
  Serial.begin(9600);
}

void loop() {
  unsigned long s = lw.read(); // Получаем команду через ресивер
  if (s){
    Serial.println(s, HEX); // Выводим полученные данные
    if (cmd.load(s)){
      Serial.println(cmd); // Если полученные данные являются командой, выводим ее
    } else {
      if (shoot.load(s)){
        Serial.println(shoot); // Если полученные данные являются выстрелом, выводим расшифровку выстрела
      }
    }
  }
}