#include <stdio.h>        // Biblioteca padrão de entrada e saída
#include "hardware/adc.h" // Biblioteca para manipulação do ADC no RP2040
#include "hardware/pwm.h" // Biblioteca para controle de PWM no RP2040
#include "pico/stdlib.h"  // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/i2c.h" // Biblioteca para comunicação I2C no RP2040
#include "ssd1306.h"      // Biblioteca para controle do display OLED SSD1306

// PINOS I2C
#define I2C_PORT i2c1 // Define o barramento I2C a ser utilizado
#define PINO_SCL 14
#define PINO_SDA 15
const int LED_B = 12;                    // Pino para controle do LED azul via PWM
const int LED_R = 13;                    // Pino para controle do LED vermelho via PWM
const int LED_G = 11;                    // Pino para controle do LED verde via PWM
const int SW = 22;           // Pino de leitura do botão do joystick

uint pos_y = 14; // Posição inicial do cursor no eixo Y

// Definição dos pinos usados para o joystick e LEDs
const int eixoX = 26;          // Pino de leitura do eixo X do joystick (conectado ao ADC)
const int eixoY = 27;          // Pino de leitura do eixo Y do joystick (conectado ao ADC)
const int ADC_CHANNEL_0 = 0; // Canal ADC para o eixo X do joystick
const int ADC_CHANNEL_1 = 1; // Canal ADC para o eixo Y do joystick

ssd1306_t display; // inicializa o objeto do display OLED


// variaveis para os leds
/*
const float DIVIDER_PWM = 16.0;          // Divisor fracional do clock para o PWM
const uint16_t PERIOD = 4096;            // Período do PWM (valor máximo do contador)
uint16_t led_b_level, led_r_level = 100; // Inicialização dos níveis de PWM para os LEDs
uint slice_led_b, slice_led_r;           // Variáveis para armazenar os slices de PWM correspondentes aos LEDs
*/

void inicializacao(){
    stdio_init_all();

    // Inicializa o ADC e os pinos de entrada analógica
    adc_init();         // Inicializa o módulo ADC
    adc_gpio_init(eixoX); // Configura o pino 26 (eixo X) para entrada ADC
    adc_gpio_init(eixoY); // Configura o pino 27 (eixo Y) para entrada ADC

    // inicialização do display OLED
    i2c_init(I2C_PORT, 600 * 1000); // Inicializa o barramento I2C com velocidade de 400kHz
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C); // Configura o pino SCL
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C); // Configura o pino SDA
    gpio_pull_up(PINO_SCL); // Ativa o pull-up no pino SCL
    gpio_pull_up(PINO_SDA); // Ativa o pull-up no pino SDA
    display.external_vcc = false; // O display não possui tensão de alimentação externa
    ssd1306_init(&display, 128, 64, 0x3c, I2C_PORT); // Inicializa o display OLED
    ssd1306_clear(&display); // Limpa o display


  // Inicializa o pino do botão do joystick
    gpio_init(SW);             // Inicializa o pino do botão
    gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
    gpio_pull_up(SW);          // Ativa o pull-up no pino do botão para evitar flutuações

    // leds
    gpio_init(LED_B); // Inicializa o pino do LED azul
    gpio_init(LED_R); // Inicializa o pino do LED vermelho
    gpio_init(LED_G); // Inicializa o pino do LED verde
    gpio_set_function(LED_B, GPIO_OUT); // Configura o pino do LED azul como saida
    gpio_set_function(LED_R, GPIO_OUT); // Configura o pino do LED vermelho como saida
    gpio_set_function(LED_G, GPIO_OUT); // Configura o pino do LED verde como saida

    gpio_put(LED_B, 0); // inicializa desligado
    gpio_put(LED_R, 0); //  inicializa desligado
    gpio_put(LED_G, 0); //  inicializa desligado

}
void print_texto(char *msg, uint pos_x, uint pos_y, uint scale){ //mensagem, posicao x, posicao y, escala
    ssd1306_draw_string(&display, pos_x, pos_y, scale, msg);
    ssd1306_show(&display);
}

void print_retangulo(int x1, int x2, int y1, int y2){
    ssd1306_draw_empty_square(&display, x1, x2, y1, y2);
    ssd1306_show(&display);
}

void print_menu(uint posy){
    ssd1306_clear(&display); // Limpa o display
    print_texto("MENU", 52, 2, 1); // Imprime a palavra "menu" no display
    print_retangulo(2, posy, 120, 12); // Imprime um retangulo no display
    print_texto("Joystick led", 10, 18, 1);
    print_texto("buzzer", 10, 30, 1);
    print_texto("PWM led", 10, 42, 1);


}


int main(){
    printf("Iniciando...\n");
    inicializacao();
    char *text = "";
    uint countDown = 0;
    uint countUp = 2;
    uint pos_y_anterior = 19; 
    uint menu = 1; // posicao das opcoes do menu - sera incrementado ou decrementado conforme o movimento do joystick

    print_menu(pos_y);

    


    gpio_put(LED_R, 0); 
    /******************************************************* */
    // trecho aproveitado do codigo do Joystick.c
    while (true) {

    adc_select_input(0);
    // Lê o valor do ADC para o eixo Y
    uint adc_y_raw = adc_read();
    const uint bar_width = 40;
    const uint adc_max = (1 << 12) - 1;
    uint bar_y_pos = adc_y_raw * bar_width / adc_max; // bar_y_pos determinara se o Joystick foi pressionado para cima ou para baixo


    if(bar_y_pos < 14 && countDown < 2){
        pos_y += 12;
        countDown += 1;
        countUp -= 1;
        menu ++; // incrementa a posicao do menu
    }
    else{
        if(bar_y_pos > 24 && countUp < 2){
        pos_y -= 12;
        countDown -= 1;
        countUp += 1;
        menu --; // incrementa a posicao do menu
    }
}
    sleep_ms(100);
    if(pos_y != pos_y_anterior){
        print_menu(pos_y);
       // pos_y_anterior = pos_y;
    }

    if(gpio_get(SW) == 0){
      if(menu == 1){
            gpio_put(LED_R, 1);
            sleep_ms(500);
            gpio_put(LED_R, 0);
        }
        else if(menu == 2){
            gpio_put(LED_G, 1);
            sleep_ms(500);
            gpio_put(LED_G, 0);
        }
        else if(menu == 3){
            gpio_put(LED_B, 1);
            sleep_ms(500);
            gpio_put(LED_B, 0);
        }
        else{
            gpio_put(LED_R, 0);
            gpio_put(LED_G, 0);
            gpio_put(LED_B, 0);
        }
    }
    sleep_ms(100);
    pos_y_anterior = pos_y; // atualiza a posicao do cursor para a origem

    }
}
