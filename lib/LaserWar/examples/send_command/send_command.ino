#include <Arduino.h>
#include <LaserWar.h>
#include <LWCommand.h>

// Используем ИК-светодиод на 940nm и цифровой пин
#define LED_PIN 3

LaserWar lw(LED_PIN);
// Создаем команду, которую нужно отправить
// LWCommand cmd(LwSetting::RadiationDamage, 15); // Можно создать команду сразу
LWCommand cmd;

void setup() {
  Serial.begin(9600);
  Serial.println("Ready");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Если мы используем LwSetting::AdminCommand, то вторым параметром должны быть данные для нее из LwAdminSetting
  cmd.setCommand(LwSetting::AdminCommand, LwAdminSetting::BombDeactivated);

  // Если мы используем цветовые команды, второй параметр - цвет
  cmd.setCommand(LwSetting::ChangeColor, LWColor::Blue);
  cmd.setCommand(LwSetting::KillColor, LWColor::Blue);
  cmd.setCommand(LwSetting::RespawnColor, LWColor::Blue);
  cmd.setCommand(LwSetting::FullAmmoColor, LWColor::Blue);
  cmd.setCommand(LwSetting::PauseColor, LWColor::Blue);

  // Если мы используем LwSetting::ApplyPreset, вторым параметром должен быть LwPreset
  cmd.setCommand(LwSetting::ApplyPreset, LwPreset::Assault);

  // Если мы используем другие команды, то вторым параметром должно быть число
  cmd.setCommand(LwSetting::RadiationDamage, 15); // Удар радиации, урон 15 единиц

  lw.send(LED_PIN, cmd); // Отправляем команду
  Serial.println(cmd); // Вывести на экран расшифровку команды

  delay(5000);
}
