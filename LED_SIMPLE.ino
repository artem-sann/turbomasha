/*
	Вращающийся дисплей на адресной ленте WS2812
	Добавление своих изображений:
		- Формат bmp
		- Разрешение 200х200px
	Настройки программы (из видео):
		- 36 сегментов
		- Размер сегмента не важен
		- 10 градусов
		- 1 градус в отсчёте
		- Глубина цвета 16 бит
	Настройки здесь:
		- Разрешение 10 градусов
		- Скорость мотора 160 (это 160/255 * 12 Вольт)
		- Лента 144 диода/метр, полметра
		- Кнопка 2 не задействована
*/

#define NUM_LEDS 72     // количество светодиодов
#define BRIGHTNESS 255  // яркость
#define RES 10          // разрешение (каждые n градусов)
#define OFFSET 0      // сдвиг по углу (0-360)
#define NUM_FRAMES 8    // количество фреймов анимации
#define FRAME_RATE 60   // количество перерисовок между сменой кадра

#define PIN 4           // пин ленты
#define INTER_PIN 3 //Датчик холла 3 или 2 пин

#include "Adafruit_NeoPixel.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);

boolean motor_state = false;
volatile uint32_t timer;
volatile uint32_t period, new_period, period_f;
volatile boolean timer_isr, hall_isr, new_loop;




void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  attachInterrupt(digitalPinToInterrupt(INTER_PIN), hall, RISING);
  strip.show();
}

void hall() {   // тут у нас тахометр
/*
2100об/мин -> 2100об/60с -> 35об/с -> 1об/0.028с
*/
  if (micros() - timer > 15000) {     // фильтруем ложные // 0.030sec
    new_period = micros() - timer;    // вот он вот он период оборота
    timer = micros();
    hall_isr = true;
    Serial.print("hal!\n");
  }
}



byte counter1 = 0, counter2 = 1,counter3 = 1;
void animation_simple() {
  strip.clear();
  for(int i=0; i<NUM_LEDS;i++){
    if(counter1==0)
    strip.setPixelColor(i, strip.Color(i*3,0,0));
    if(counter1==1)strip.setPixelColor(i, strip.Color(0,i*3,0));
    if(counter1==2)strip.setPixelColor(i, strip.Color(0,0,i*3));
  }
  counter1=(counter1+1)%3;
  /*strip.setPixelColor(counter1, strip.Color(255,0,0));
  strip.setPixelColor(counter2,  strip.Color(0,255,0));
  strip.setPixelColor(counter3,  strip.Color(0,0,255));
  counter1=((counter1+3)%(NUM_LEDS-20))+20;
  counter2=((counter2+3)%(NUM_LEDS-20))+20;
  counter3=((counter3+3)%(NUM_LEDS-20))+20;*/
  strip.show();
  Serial.print("OOMG!\n");
}
////////////////////////////////////////

#define HUE_START 3     // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define HUE_GAP 18      // коэффициент цвета огня (чем больше - тем дальше заброс по цвету)
#define SMOOTH_K 1   // коэффициент плавности огня
#define MIN_BRIGHT 80   // мин. яркость огня
#define MAX_BRIGHT 255  // макс. яркость огня
#define MIN_SAT 245     // мин. насыщенность
#define MAX_SAT 255     // макс. насыщенность

// для разработчиков
#define ZONE_AMOUNT NUM_LEDS   // количество зон
byte zoneValues[ZONE_AMOUNT];
byte zoneRndValues[ZONE_AMOUNT];

uint32_t  getFireColor(int val) {
  // чем больше val, тем сильнее сдвигается цвет, падает насыщеность и растёт яркость
  return strip.ColorHSV(
           HUE_START + map(val, 20, 60, 0, HUE_GAP),                    // H
           constrain(map(val, 20, 60, MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(val, 20, 60, MIN_BRIGHT, MAX_BRIGHT), 0, 255)  // V
         );
}

void fireTick() {
  int thisPos = 0, lastPos = 0;

  for( int i=0; i<ZONE_AMOUNT;i++) {
      zoneRndValues[i] = random(0, 10);
  }

  for( int i=0; i<ZONE_AMOUNT;i++) {
    zoneValues[i] = (float)zoneValues[i] * (1 - SMOOTH_K) + (float)zoneRndValues[i] * 10 * SMOOTH_K;
    //zoneValues[i] = (zoneRndValues[i] * 10 - zoneValues[i]) * SMOOTH_K;
    strip.setPixelColor(i, getFireColor(zoneValues[i]));
  }
  strip.show();
}

// возвращает цвет огня для одного пикселя
///////////////////////////////////


void loop() {
  if (hall_isr) {
    hall_isr = false;
    animation_simple();
    //fireTick();
  }
}


