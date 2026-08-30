#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// drawing a bat and animating it on my terminal

volatile int stop_animation; // global variable to signal animation stop

// colors: {R, G, B}
// 0 = transparent
// 1 = black/almost black (border & eyes)
// 2 = dark purple (belly shadow)
// 3 = medium purple (face/top of wings)
// 4 = pink (internal wing part)
int palette[5][3] = {
    {0, 0, 0}, // 0
    {15,  10, 15}, // 1
    {75, 30, 85}, // 2
    {145, 80, 155}, // 3
    {215, 120, 165}, // 4
};

#define BAT_WIDTH 19
#define BAT_HEIGHT 10

// wings up
int frame1[BAT_HEIGHT][BAT_WIDTH] = {
    {0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0},
    {1,4,4,4,1,0,1,3,3,3,1,0,1,4,4,4,1},
    {1,4,4,4,3,1,3,3,3,3,3,1,3,4,4,4,1},
    {0,1,3,4,4,3,3,1,3,1,3,3,4,4,3,1,0},
    {0,0,1,3,3,3,3,3,3,3,3,3,3,3,1,0,0},
    {0,0,0,1,1,1,2,2,2,2,2,1,1,1,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// wings down
int frame2[BAT_HEIGHT][BAT_WIDTH] = {
    {0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,3,3,3,1,0,0,0,0,0,0},
    {0,0,1,1,1,1,3,3,3,3,3,1,1,1,1,0,0},
    {0,1,4,4,4,3,3,1,3,1,3,3,4,4,4,1,0},
    {1,4,4,4,3,3,3,3,3,3,3,3,3,4,4,4,1},
    {1,3,3,1,1,1,2,2,2,2,2,1,1,1,3,3,1},
    {0,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

void draw_frame(int frame[BAT_HEIGHT][BAT_WIDTH]) {
    for (int y = 0; y < BAT_HEIGHT; y += 2) {
        for (int x = 0; x < BAT_WIDTH; x++) {
            int color_top = frame[y][x];
            int color_bottom = (y + 1 < BAT_HEIGHT) ? frame[y + 1][x] : 0;

            if (color_top == 0 && color_bottom == 0) {
                printf("\x1b[0m ");
            } else if (color_top != 0 && color_bottom == 0) {
                printf("\x1b[38;2;%d;%d;%dm", palette[color_top][0], palette[color_top][1], palette[color_top][2]);
                printf("\x1b[49m");
                printf("▀");
            } else if (color_top == 0 && color_bottom != 0) {
                printf("\x1b[38;2;%d;%d;%dm", palette[color_bottom][0], palette[color_bottom][1], palette[color_bottom][2]);
                printf("\x1b[49m");
                printf("▄");
            } else {
                printf("\x1b[38;2;%d;%d;%dm", palette[color_top][0], palette[color_top][1], palette[color_top][2]);
                printf("\x1b[48;2;%d;%d;%dm", palette[color_bottom][0], palette[color_bottom][1], palette[color_bottom][2]);
                printf("▀");
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
        if (current_frame == 0) {
            draw_frame(frame1);
        } else {
            draw_frame(frame2);
        }
        fflush(stdout);

        for (int i = 0; i < 5; i++) {
            if (stop_animation) break;
            usleep(50000);
        }

        if (!stop_animation) {
            printf("\x1b[%dA", terminal_lines);
        }

        current_frame = !current_frame;
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

    printf("initiating request...\n");
    printf("you should now see the animated bat while we wait (simulating 5 seconds)...\n");

    pthread_t anim_thread;
    stop_animation = 0;

    if (pthread_create(&anim_thread, NULL, bat_animator, NULL) != 0) {
        perror("error while creating animation's thread");
        return 1;
    }

    sleep(5);
    stop_animation = 1;
    pthread_join(anim_thread, NULL);

    printf("\x1b[32m[+] connection completed successfully!\x1b[0m\n");
    printf("data received: { \"status\": \"ok\", \"bytes\": 1024 }\n");

    return 0;
}
