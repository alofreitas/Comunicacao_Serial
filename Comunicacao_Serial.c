#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "led.pio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#define endereco 0x3C
#define MATRIZ 25
#define OUT_PIN 7

// Definição dos pinos da GPIO dos LEDs e botões, além do display
#define LED_GREEN 11
#define LED_BLUE 12
#define LED_RED 13
#define BOTAO_A 5
#define BOTAO_B 6
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_PORT i2c1

// Definição de variáveis globais
PIO pio; 
uint sm; 
ssd1306_t ssd; 

// Matriz
double led_matrix[10][MATRIZ] = {
        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 0
        {0, 0, 1, 0, 0,
         0, 1, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
        }, // 1
        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 0, 1, 0, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0,
        },  // 2
        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 3
        {0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 0, 0, 1, 0,
        }, // 4
        {0, 1, 1, 1, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 5
        {0, 1, 1, 1, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
        },  // 6
        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
         0, 0, 1, 0, 0,
        }, // 7
        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
        }, // 8
        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
        }  // 9
    };

// Sequência de LEDs
int matriz[MATRIZ] = { 0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 10, 11, 12, 13, 14, 19, 18, 17, 16, 15, 20, 21, 22, 23, 24};

// Melhorar a intensidade
uint32_t cores_rgb(double blue)
{
  unsigned char B;
  B = blue * 200; 
  return (B << 8);
}

// Converte os valores da matriz de LEDs para a matriz de LEDs da PIO
void display_num(int n) {
    for (int i = 0; i < MATRIZ; i++) {
        uint32_t valor_led = cores_rgb(led_matrix[n][matriz[MATRIZ - 1 - i]]);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

static volatile uint num = 0;
static volatile uint32_t last_time = 0;

// Função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time()); // Obtém o tempo atual

    // Inverte a cor
    bool cor = true;
    cor = !cor; 

    // Debounce: evita múltiplas leituras em um curto intervalo de tempo
    if (tempo_atual - last_time > 200) {
        switch(gpio){
            case BOTAO_A:
            //Mudar o estado do led verde ao pressionar o botão A
            gpio_put(LED_GREEN, !gpio_get(LED_GREEN));
    
            // Exibe uma mensagem no display
            ssd1306_fill(&ssd, !cor); 
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); 
            ssd1306_draw_string(&ssd, "BOTAO A", 20, 30); 
            ssd1306_draw_string(&ssd, "ACIONADO", 15, 48);     
            ssd1306_send_data(&ssd); 
            
            // Exibe o número no serial monitor
            printf("Botão A pressionado, estado do LED alterado\n");

            break;

            case BOTAO_B:
            //Mudar o estado do led azul ao pressionar o botão B
            gpio_put(LED_BLUE, !gpio_get(LED_BLUE));
            printf("Botão B pressionado, estado do LED alterado\n");

            // Exibe uma mensagem no display
            ssd1306_fill(&ssd, !cor); 
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
            ssd1306_draw_string(&ssd, "BOTAO B", 20, 30);  
            ssd1306_draw_string(&ssd, "ACIONADO", 15, 48);     
            ssd1306_send_data(&ssd);

            break;

        }   

    // Registra o tempo do último evento
    last_time = tempo_atual;
}

        
}

int main()
{
    // Inicializa o stdio
    stdio_init_all();

    // Inicializa os LEDs e os botões
    gpio_init(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_pull_up(BOTAO_B);
    gpio_init(LED_RED);
    gpio_init(LED_GREEN);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    // Inicializa a PIO
    pio = pio0;
    uint offset = pio_add_program(pio, &led_program); 
    uint sm = pio_claim_unused_sm(pio, true);
    led_program_init(pio, sm, offset, OUT_PIN);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL); 
 
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Configura a interrupção
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);


while (true) {

    int n;
    char c;

    scanf("%c", &c);

    // Verifica se o caractere é um número
    if (c >= '0' && c <= '9') {
        n = c - '0'; // Converte o caractere para um número
        display_num(n);
    }

        // Limpa o display
        ssd1306_fill(&ssd, false);

        // Exibe o caractere capturado na posição (0, 0)
        char str[2] = {c, '\0'};
        ssd1306_draw_string(&ssd, str, 0, 0);

        // Atualiza o display
        ssd1306_send_data(&ssd);

        printf("Caractere: %c\n", c);
    }

    sleep_ms(1000);  // Aguarda um pouco antes da próxima leitura
    
}


