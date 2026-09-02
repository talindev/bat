#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

volatile int stop_animation = 0; // controls thread

int palette[12][3] = {
    {0, 0, 0},       // 0  = transparent
    {15, 10, 15},    // 1  = black (border, eyes)
    {103, 67, 112},  // 2  = medium purple (body)
    {87, 58, 94},    // 3  = dark purple (shadow)
    {194, 130, 186}, // 4  = light purple (wing)
    {250, 192, 243}, // 5  = lightest purple (detail)
    {242, 240, 206}, // 6  = off white (teeth)
    {250, 36, 68},   // 7  = vibrant red (mouth open)
    {159, 32, 53},   // 8  = dark red (mouth background)
    {254, 254, 254}, // 9  = lightest white (strong echo)
    {212, 210, 212}, // 10 = medium white-gray (echo)
    {181, 179, 181}  // 11 = dark white (or gray) (weak echo)
};

#define BAT_WIDTH 37
#define BAT_HEIGHT 16
#define NUM_FRAMES 8

// 3d matrix with frames
int frames[NUM_FRAMES][BAT_HEIGHT][BAT_WIDTH] = {}; // secret (for now)

void draw_frame(int frame[BAT_HEIGHT][BAT_WIDTH]) {
    for (int y = 0; y < BAT_HEIGHT; y += 2) {
        for (int x = 0; x < BAT_WIDTH; x++) {
            int color_top = frame[y][x];
            int color_bottom = (y + 1 < BAT_HEIGHT) ? frame[y + 1][x] : 0;

            if (color_top == 0 && color_bottom == 0) {
                printf("\x1b[0m ");
            } else if (color_top != 0 && color_bottom == 0) {
                printf("\x1b[38;2;%d;%d;%dm\x1b[49m▀",
                    palette[color_top][0], palette[color_top][1], palette[color_top][2]);
            } else if (color_top == 0 && color_bottom != 0) {
                printf("\x1b[38;2;%d;%d;%dm\x1b[49m▄",
                    palette[color_bottom][0], palette[color_bottom][1], palette[color_bottom][2]);
            } else {
                printf("\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm▀",
                    palette[color_top][0], palette[color_top][1], palette[color_top][2],
                    palette[color_bottom][0], palette[color_bottom][1], palette[color_bottom][2]);
            }
        }
        printf("\x1b[0m\n");
    }
}

void *bat_animator(void *arg) {
    printf("\x1b[?25l");
    int current_frame = 0;

    int terminal_lines = BAT_HEIGHT / 2;

    while (!stop_animation) {
        draw_frame(frames[current_frame]);
        fflush(stdout);

        for (int i = 0; i < 5; i++) {
            if (stop_animation) break;
            usleep(25000);
        }

        if (!stop_animation) {
            printf("\x1b[%dA", terminal_lines);
        }

        current_frame = (current_frame + 1) % NUM_FRAMES;
    }

    for (int i = 0; i < terminal_lines; i++) {
        printf("\x1b[2K\n");
    }
    printf("\x1b[%dA", terminal_lines);

    printf("\x1b[?25h");
    return NULL;
}

int main(void) {
    printf("\x1b[2J\x1b[H");

    printf("Iniciando request...\n");
    printf("O morcego com eco agora vai animar todos os 8 frames (simulando 5 segs)...\n\n");

    pthread_t anim_thread;
    stop_animation = 0;

    if (pthread_create(&anim_thread, NULL, bat_animator, NULL) != 0) {
        perror("Erro ao criar a thread de animacao");
        return 1;
    }

    sleep(5); // Simulando o tempo de conexão/download
    stop_animation = 1;
    pthread_join(anim_thread, NULL);

    printf("\x1b[32m[+] Conexao concluida com sucesso!\x1b[0m\n");
    printf("Dados recebidos: { \"status\": \"ok\", \"bytes\": 1024 }\n");

    return 0;
}
