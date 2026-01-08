// Date Created: 2023-07-25
// Date Updated: 2025-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"
#include "./MiscFunctions/Shapes/inc/cube.h"
#include "./MiscFunctions/LinAlg/inc/linalg.h"
#include "./MiscFunctions/LinAlg/inc/quaternions.h"
#include "./MiscFunctions/LinAlg/inc/vect3d.h"
#include "./G8RTOS/G8RTOS_Scheduler.h"
#include "./G8RTOS/G8RTOS_IPC.h"
#include "./MultimodDrivers/multimod_ST7789.h"
#include "./MultimodDrivers/multimod_uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>



/************************************Includes***************************************/




/*************************************Defines********************************/


/*************************************Defines***************************************/



/*********************************Global Variables**********************************/
GameBoard_t g_gameBoard;
volatile bool buttonPressed = false;
volatile bool restart = false;
uint32_t ship_speed = 40;
volatile bool game_over = false;
int16_t score = 0;
uint16_t PIXEL_SHIP_SIZE = 30;
volatile uint32_t asteroid_spawn_counter = 0;
volatile uint32_t current_spawn_rate = 100;
volatile bool multi_shot_active = false;
volatile bool fire_input_active = false;
volatile bool player_restarted = true;
volatile bool game_level_chosen = true;
volatile uint8_t active_asteroid_limit = 2;
volatile bool level_selected = false;
volatile bool lazer = false;
volatile bool wasLevel = false;
/*********************************Global Variables**********************************/

Bullet_t g_bulletPool[MAX_BULLETS];
Asteroid_t g_AsteroidPool[MAX_ASTEROIDS];


/********************************Helper Functions***********************************/
void Draw_Level_Select_Menu(void) {

    ST7789_DrawStringStatic("Welcome to Space Invaders!", ST7789_BLUE, 20, 40);

    ST7789_DrawRectangle((3 * GRID_SIZE), (10 * GRID_SIZE), 3 * GRID_SIZE, 3 * GRID_SIZE, ~ST7789_GREEN);
    ST7789_DrawStringStatic("EASY", ~ST7789_GREEN, (3 * GRID_SIZE), (9 * GRID_SIZE));

    ST7789_DrawRectangle((8 * GRID_SIZE), (10 * GRID_SIZE), 3 * GRID_SIZE, 3 * GRID_SIZE, ~ST7789_YELLOW);
    ST7789_DrawStringStatic("MED", ~ST7789_YELLOW, (8 * GRID_SIZE), (9 * GRID_SIZE));

    ST7789_DrawRectangle((13 * GRID_SIZE), (10 * GRID_SIZE), 3 * GRID_SIZE, 3 * GRID_SIZE, ~ST7789_BLUE);
    ST7789_DrawStringStatic("HARD", ~ST7789_BLUE, (13 * GRID_SIZE), (9 * GRID_SIZE));


    // Instructions
    ST7789_DrawStringStatic("Move Ship (Joystick)", ~ST7789_WHITE, 40, 200);
    ST7789_DrawStringStatic("and Shoot (Switch 4)", ~ST7789_WHITE, 40, 220);
    ST7789_DrawStringStatic("to Select Level!", ~ST7789_WHITE, 40, 240);
}

void Init_Ship(Ship_t* ship) {

    ship->col = MAX_X_GRID / 2;


    ship->row = MAX_Y_GRID;

    ship->cannon_angle = 0;  // Straight up
    ship->alive = true;
    score = 0;
    player_restarted = true;

    UARTprintf("Ship initialized: Col=%d, Row=%d\n", ship->col, ship->row);
}

Bullet_t* Create_Bullet(int16_t row, int16_t col, int16_t v_row, int16_t v_col) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        // Find the first bullet that is NOT active
        if (!g_bulletPool[i].active) {
            g_bulletPool[i].row = row;
            g_bulletPool[i].col = col;
            g_bulletPool[i].v_row = v_row;
            g_bulletPool[i].v_col = v_col;
            g_bulletPool[i].active = true;
            return &g_bulletPool[i]; // Return pointer to this slot
        }
    }
    return NULL; // Pool is full (too many bullets on screen)
}

void Remove_Bullet(Bullet_t** head, Bullet_t* bullet) {
    if (bullet != NULL) {
        bullet->active = false;
    }
}



Asteroid_t* Create_Asteroid(int16_t row, int16_t col, int16_t v_row, int16_t v_col) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        // Find the first bullet that is NOT active
        if (!g_AsteroidPool[i].active) {
            g_AsteroidPool[i].row = row + 3;
            g_AsteroidPool[i].col = col;
            g_AsteroidPool[i].v_row = v_row;
            g_AsteroidPool[i].v_col = v_col;
            g_AsteroidPool[i].active = true;
            return &g_AsteroidPool[i]; // Return pointer to this slot
        }
    }
    return NULL;
}

void Remove_Asteroid(Asteroid_t** head, Asteroid_t* asteroid) {
    if (asteroid != NULL) {
        asteroid->active = false;
    }
}



//bool Check_Collision(int16_t row1, int16_t col1, int16_t row2, int16_t col2) {
//    if (row1 == row2 && col1 == col2) {
//        return true;
//    }
//    return false;
//}


bool Check_Box_Collision(int16_t x1, int16_t y1, int16_t w1, int16_t h1,
                         int16_t x2, int16_t y2, int16_t w2, int16_t h2) {
    // Check if rectangles overlap
    // No collision if one rectangle is completely to the left/right/above/below the other
    if (x1 + w1 - 15  <= x2 || x2 + w2 - 15 <= x1 ||
        y1 + h1 - 15 <= y2 || y2 + h2 - 15 <= y1) {
        return false;
    }
    return true;
}

bool Check_Exact_Collision(int16_t x1, int16_t y1, int16_t w1, int16_t h1,
                         int16_t x2, int16_t y2, int16_t w2, int16_t h2) {
    // Check if rectangles overlap
    // No collision if one rectangle is completely to the left/right/above/below the other
    if (x1 + w1 <= x2 || x2 + w2 <= x1 ||
        y1 + h1 <= y2 || y2 + h2 <= y1) {
        return false;
    }
    return true;
}

// Check collision between bullet and asteroid (grid-based)
bool Check_Bullet_Asteroid_Collision(Bullet_t* bullet, Asteroid_t* asteroid) {
    // Convert grid positions to screen positions
    int16_t bullet_x = (bullet->col * GRID_SIZE) + GRID_OFFSET;
    int16_t bullet_y = (bullet->row * GRID_SIZE) + GRID_OFFSET;

    int16_t asteroid_x = (asteroid->col * GRID_SIZE) + GRID_OFFSET;
    int16_t asteroid_y = (asteroid->row * GRID_SIZE) + GRID_OFFSET;

    return Check_Exact_Collision(bullet_x, bullet_y, BULLET_SIZE, BULLET_SIZE,
                               asteroid_x, asteroid_y, ASTEROID_SIZE, ASTEROID_SIZE);
}

// Check collision between ship and asteroid (grid-based)
bool Check_Ship_Asteroid_Collision(Ship_t* ship, Asteroid_t* asteroid) {
    // Convert grid positions to screen positions
    int16_t ship_x = (ship->col * GRID_SIZE) + GRID_OFFSET;
    int16_t ship_y = (ship->row * GRID_SIZE) + GRID_OFFSET;

    int16_t asteroid_x = (asteroid->col * GRID_SIZE) + GRID_OFFSET;
    int16_t asteroid_y = (asteroid->row * GRID_SIZE) + GRID_OFFSET;

    return Check_Box_Collision(ship_x, ship_y, PIXEL_SHIP_SIZE, PIXEL_SHIP_SIZE,
                               asteroid_x, asteroid_y, ASTEROID_SIZE, ASTEROID_SIZE);
}

bool Check_Ship_Mystery_Collision(Ship_t* ship, Mystery_t* mystery) {
    // Convert grid positions to screen position
    int16_t ship_x = (ship->col * GRID_SIZE) + GRID_OFFSET;
    int16_t ship_y = (ship->row * GRID_SIZE) + GRID_OFFSET;

    int16_t mystery_x = (mystery->col * GRID_SIZE) + GRID_OFFSET;
    int16_t mystery_y = (mystery->row * GRID_SIZE) + GRID_OFFSET;

    return Check_Exact_Collision(ship_x, ship_y, PIXEL_SHIP_SIZE, PIXEL_SHIP_SIZE,
                               mystery_x, mystery_y, MYSTERY_SIZE, MYSTERY_SIZE);
}

void Spawn_Mystery_Box(void) {
    // Only spawn if not already alive
    if (!g_gameBoard.mystery.alive) {

        // Random column within grid bounds
        int16_t random_col = rand() % MAX_X_GRID;
        int16_t random_row = rand() % MAX_Y_GRID;

        g_gameBoard.mystery.col = random_col;
        g_gameBoard.mystery.row = random_row;
        g_gameBoard.mystery.alive = true;
    }
}

void Init_Game(void) {
    g_gameBoard.level = 1;
    g_gameBoard.asteroids_head = NULL;
    g_gameBoard.bullets_head = NULL;
    g_gameBoard.state = GAME_LEVEL;
    g_gameBoard.ship.alive = true;

    g_gameBoard.mystery.alive = false; // Reset mystery box

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        g_AsteroidPool[i].active = false;
    }
    for (int i = 0; i < MAX_BULLETS; i++) {
        g_bulletPool[i].active = false;
    }

    // Reset Global Counters
    score = 0;
    asteroid_spawn_counter = 0;
    current_spawn_rate = SPAWN_RATE;
    multi_shot_active = false;
    fire_input_active = false;

    // Initialize Ship Coordinates
    Init_Ship(&g_gameBoard.ship);

}



/********************************Helper Functions***********************************/




/*************************************Threads***************************************/

void Idle_Thread(void) {
    while (1) {
        // Lowest priority thread - does nothing
    }
}

/********************************Background Threads*********************************/

void Player_Thread(void) {
    uint32_t result;
    int16_t x, y;
    float joystickX_norm, joystickY_norm;
    bool menu_drawn = false;

    // Initial draw flag

    while(1) {
        if (g_gameBoard.state != GAME_RUNNING && g_gameBoard.state != GAME_LEVEL) {
           sleep(100);
           continue;
       }

        if (g_gameBoard.state == GAME_LEVEL && !menu_drawn) {
            G8RTOS_WaitSemaphore(&sem_SPIA);
            ST7789_Fill(~ST7789_BLACK);
            Draw_Level_Select_Menu();
            menu_drawn = true;
            ST7789_DrawSprite(
                (g_gameBoard.ship.col * GRID_SIZE) + GRID_OFFSET,
                (g_gameBoard.ship.row * GRID_SIZE) + GRID_OFFSET,
                PIXEL_SHIP_SIZE,
                PIXEL_SHIP_SIZE,
                pixel_ship
            );
            G8RTOS_SignalSemaphore(&sem_SPIA);


        } else if (g_gameBoard.state != GAME_LEVEL) {
          menu_drawn = false; // Reset flag so menu redraws next time we enter state
        }


        if (g_gameBoard.state == GAME_RUNNING && player_restarted) {
            G8RTOS_WaitSemaphore(&sem_SPIA);
            // Draw using grid math
            ST7789_DrawSprite(
                (g_gameBoard.ship.col * GRID_SIZE) + GRID_OFFSET,
                (g_gameBoard.ship.row * GRID_SIZE) + GRID_OFFSET,
                PIXEL_SHIP_SIZE,
                PIXEL_SHIP_SIZE,
                pixel_ship
            );
            G8RTOS_SignalSemaphore(&sem_SPIA);

            char output[64];
            snprintf(output, sizeof(output), "Score: %d", score);
            ST7789_DrawRectangle(10, 20, 95, 15, ~ST7789_BLACK);
            ST7789_DrawStringStatic(output, ~ST7789_WHITE, 10, 20);

            player_restarted = false;
        }

        // 3. Read FIFO
        result = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
        x = (int16_t)(result >> 16);
        y = (int16_t)(result & 0xFFFF);

        // 4. Normalize Joystick (Floats)
        if (x <= 600) {
            joystickX_norm = -1.0f;  // LEFT
        } else if (x >= 3500) {
            joystickX_norm = 1.0f;   // RIGHT
        } else {
            joystickX_norm = 0.0f;   // CENTER
        }

        if (y <= 600) {
            joystickY_norm = -1.0f;  // UP (Screen Y decreases)
        } else if (y >= 3500) {
            joystickY_norm = 1.0f;   // DOWN (Screen Y increases)
        } else {
            joystickY_norm = 0.0f;   // CENTER
        }

        // 5. Calculate New Positions
        bool move_detected = false;

        // Use critical section to read current position
        G8RTOS_WaitSemaphore(&sem_gameState);
        int new_row = g_gameBoard.ship.row;
        int new_col = g_gameBoard.ship.col;
        int old_row = g_gameBoard.ship.row;
        int old_col = g_gameBoard.ship.col;
        G8RTOS_SignalSemaphore(&sem_gameState);

        // Apply movement
        if (joystickX_norm < 0) {
            new_col--;
            move_detected = true;
        } else if (joystickX_norm > 0) {
            new_col++;
            move_detected = true;
        }

        if (joystickY_norm < 0) {
            new_row--;
            move_detected = true;
        } else if (joystickY_norm > 0) {
            new_row++;
            move_detected = true;
        }

        // 6. Clamp to Grid Boundaries
        if (new_col < 0) new_col = 0;
        if (new_col > MAX_X_GRID) new_col = MAX_X_GRID;

        if (new_row < 0) new_row = 0;
        if (new_row > MAX_Y_GRID) new_row = MAX_Y_GRID;

        // 7. Update and Draw (If moved)
        if (move_detected) {

            G8RTOS_WaitSemaphore(&sem_gameState);

            // Double check bounds/logic inside semaphore or simply update
            g_gameBoard.ship.row = new_row;
            g_gameBoard.ship.col = new_col;

            G8RTOS_SignalSemaphore(&sem_gameState);

            // Draw Update
            if (old_col != new_col || old_row != new_row) {
                G8RTOS_WaitSemaphore(&sem_SPIA);

                // Erase Old Position
                ST7789_EraseSprite(
                        (old_col * GRID_SIZE) + GRID_OFFSET,
                        (old_row * GRID_SIZE) + GRID_OFFSET,
                        PIXEL_SHIP_SIZE,
                        PIXEL_SHIP_SIZE,
                        pixel_ship
                    );
                G8RTOS_SignalSemaphore(&sem_SPIA);

                if (g_gameBoard.state == GAME_LEVEL) {
                    G8RTOS_WaitSemaphore(&sem_SPIA);
                    Draw_Level_Select_Menu();
                    G8RTOS_SignalSemaphore(&sem_SPIA);
                }

                G8RTOS_WaitSemaphore(&sem_SPIA);
                // Draw New Position
                ST7789_DrawSprite(
                                (g_gameBoard.ship.col * GRID_SIZE) + GRID_OFFSET,
                                (g_gameBoard.ship.row * GRID_SIZE) + GRID_OFFSET,
                                PIXEL_SHIP_SIZE,
                                PIXEL_SHIP_SIZE,
                                pixel_ship
                            );

                G8RTOS_SignalSemaphore(&sem_SPIA);
            }
        }

        sleep(ship_speed);
    }
}


void Spawn_Asteroids_Thread(void) {
    while (1) {

        if (g_gameBoard.state != GAME_RUNNING) {
            sleep(100);
            continue; // Skip the rest of the loop
        }

        asteroid_spawn_counter++;

        int active_count = 0;
        G8RTOS_WaitSemaphore(&sem_gameState);

        for(int k=0; k<MAX_ASTEROIDS; k++) {
             if (g_AsteroidPool[k].active) active_count++;
        }

        if (active_count >= active_asteroid_limit) {
            G8RTOS_SignalSemaphore(&sem_gameState);
            sleep(current_spawn_rate);
            continue;
        }


        // 3. Generate Random Column
        int16_t random_col = rand() % MAX_X_GRID;

        if (asteroid_spawn_counter >= 50) {

            G8RTOS_WaitSemaphore(&sem_SPIA);
            ST7789_EraseSprite(
                             (g_gameBoard.mystery.col * GRID_SIZE) + GRID_OFFSET,
                             (g_gameBoard.mystery.row * GRID_SIZE) + GRID_OFFSET,
                             MYSTERY_SIZE,
                             MYSTERY_SIZE,
                             pixel_mystery
            );
            G8RTOS_SignalSemaphore(&sem_SPIA);
            Spawn_Mystery_Box();
            asteroid_spawn_counter = 0; // Reset counter
                } else {
                    // Standard Asteroid Spawning
                    int16_t random_col = rand() % MAX_X_GRID;
                    Create_Asteroid(0, random_col, 1, 0);
                }


        G8RTOS_SignalSemaphore(&sem_gameState);
        sleep(current_spawn_rate); // change speed later on
    }


}


void Read_Buttons_Thread(void) {
    uint8_t buttonState;

    G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);

    while (1) {

        // Read button state
        G8RTOS_WaitSemaphore(&sem_I2CA);
        buttonState = MultimodButtons_Get();
        G8RTOS_SignalSemaphore(&sem_I2CA);
        if ((buttonState & SW1) && g_gameBoard.state == GAME_LEVEL) {

            int16_t ship_c = g_gameBoard.ship.col;
            int16_t ship_r = g_gameBoard.ship.row;

            if (ship_r >= 9 && ship_r <= 13) {


                if (ship_c >= 1 && ship_c <= 4) {
                    active_asteroid_limit = 2;
                    level_selected = true;

                }

                else if (ship_c >= 7 && ship_c <= 9) {
                    active_asteroid_limit = 3;
                    level_selected = true;
                }

                else if (ship_c >= 11 && ship_c <= 13) {
                    active_asteroid_limit = 5; // Hard mode!
                    level_selected = true;
                }
            }
            if (level_selected) {
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_Fill(~ST7789_BLACK);
                G8RTOS_SignalSemaphore(&sem_SPIA);

                Init_Game();

                g_gameBoard.state = GAME_RUNNING;

                fire_input_active = false;

                player_restarted = true;

                sleep(500); // Debounce delay
            }


         }


        // Check if SW1 pressed (shoot)
        if ((buttonState & SW1) && (g_gameBoard.state == GAME_RUNNING)) {
            fire_input_active = true;
        } else {
            fire_input_active = false;
        }



        if (buttonState & SW2) {
            if (game_over) {


                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_Fill(~ST7789_BLACK);
                G8RTOS_SignalSemaphore(&sem_SPIA);

                Init_Game();

                game_over = false;
                restart = false;

            }

            else if (!restart) {

                g_gameBoard.state = GAME_PAUSED;
                wasLevel = true;
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_Fill(~ST7789_RED);
                G8RTOS_SignalSemaphore(&sem_SPIA);
                char output[64];
                snprintf(output, sizeof(output), "Paused");
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_DrawStringStatic(output, ~ST7789_WHITE, 100, 120);
                G8RTOS_SignalSemaphore(&sem_SPIA);
                restart = true;
            }

            else {
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_Fill(~ST7789_BLACK);
                player_restarted = true;
                G8RTOS_SignalSemaphore(&sem_SPIA);
                g_gameBoard.state = GAME_RUNNING;
                restart = false;
            }
        }

        if ((buttonState & SW3) && g_gameBoard.state == GAME_RUNNING) {
            g_gameBoard.state = GAME_LEVEL;
            player_restarted = true;
        }




        GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_4);
        sleep(50);
    }
}



void Create_Bullet_Thread(void) {
    int16_t bullet_row, bullet_col;

    while (1) {

        if (g_gameBoard.state != GAME_RUNNING) {
                    sleep(100);
                    continue; // Skip the rest of the loop
        }


        if (fire_input_active && g_gameBoard.ship.alive) {

            G8RTOS_WaitSemaphore(&sem_gameState);
            int16_t ship_r = g_gameBoard.ship.row;
            int16_t ship_c = g_gameBoard.ship.col;
            G8RTOS_SignalSemaphore(&sem_gameState);

            bullet_row = ship_r - 1;
            bullet_col = ship_c;

            if (bullet_row >= 0) {
                if (multi_shot_active) {
                     // Shoot 5 Bullets (Fan Pattern)
                     G8RTOS_WaitSemaphore(&sem_gameState);
                     Create_Bullet(bullet_row, bullet_col, -1,  0); // Center
                     Create_Bullet(bullet_row, bullet_col, -1, -1); // Left Diagonal
                     Create_Bullet(bullet_row, bullet_col, -1,  1); // Right Diagonal
                     Create_Bullet(bullet_row, bullet_col, -1, -2); // Hard Left
                     Create_Bullet(bullet_row, bullet_col, -1,  2); // Hard Right
                     G8RTOS_SignalSemaphore(&sem_gameState);
                }

                else if (lazer) {
                    G8RTOS_WaitSemaphore(&sem_gameState);
                    Create_Bullet(bullet_row, bullet_col, -1,  0);
                    Create_Bullet(bullet_row, bullet_col, -2, 0);
                    Create_Bullet(bullet_row, bullet_col, -3,  0);
                    Create_Bullet(bullet_row, bullet_col, -4, 0);
                    Create_Bullet(bullet_row, bullet_col, -5,  0);
                    Create_Bullet(bullet_row, bullet_col, -6,  0);
                    Create_Bullet(bullet_row, bullet_col, -7,  0);
                    Create_Bullet(bullet_row, bullet_col, -8,  0);
                    Create_Bullet(bullet_row, bullet_col, -9,  0);
                    Create_Bullet(bullet_row, bullet_col, -10,  0);




                    G8RTOS_SignalSemaphore(&sem_gameState);
                }
                     else {
                          // Normal Shot
                         G8RTOS_WaitSemaphore(&sem_gameState);
                         Create_Bullet(bullet_row, bullet_col, -1, 0);
                         G8RTOS_SignalSemaphore(&sem_gameState);
                     }
            }

            sleep(400);
        } else {
            sleep(10);
        }
    }
}

void Board_Thread(void) {
    int i;
    int j;




    while (1) {


        if (g_gameBoard.state != GAME_RUNNING) {
               sleep(100);
               continue;
            }

        bool ship_hit = false;
        G8RTOS_WaitSemaphore(&sem_gameState);

        for (j = 0; j < MAX_ASTEROIDS; j++) {
            if (g_AsteroidPool[j].active) {


                if (Check_Ship_Asteroid_Collision(&g_gameBoard.ship, &g_AsteroidPool[j])) {
                   ship_hit = true;
                   g_AsteroidPool[j].active = false;
                   g_gameBoard.ship.alive = false;
                   g_gameBoard.state = GAME_OVER;
                   G8RTOS_SignalSemaphore(&sem_gameOver);

                   break;
                 }


                // 1. Erase Old Asteroid
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_DrawRectangle(
                    (g_AsteroidPool[j].col * GRID_SIZE) + GRID_OFFSET,
                    (g_AsteroidPool[j].row * GRID_SIZE) + GRID_OFFSET,
                    ASTEROID_SIZE, ASTEROID_SIZE,
                    ~ST7789_BLACK);
                G8RTOS_SignalSemaphore(&sem_SPIA);

                // 2. Move Asteroid
                g_AsteroidPool[j].row += g_AsteroidPool[j].v_row;
                g_AsteroidPool[j].col += g_AsteroidPool[j].v_col;

                // 3. Check Boundary (Bottom of screen)
                if (g_AsteroidPool[j].row > MAX_Y_GRID) {
                    g_AsteroidPool[j].active = false;
                    continue; // Skip drawing
                }

                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_DrawSprite(
                                (g_AsteroidPool[j].col * GRID_SIZE) + GRID_OFFSET,
                                (g_AsteroidPool[j].row * GRID_SIZE) + GRID_OFFSET,
                                ASTEROID_SIZE,
                                ASTEROID_SIZE,
                                pixel_rock
                            );
                G8RTOS_SignalSemaphore(&sem_SPIA);



            }
        }

        if (ship_hit) {

        } else {



        if (g_gameBoard.mystery.alive) {

            G8RTOS_WaitSemaphore(&sem_SPIA);
            ST7789_DrawSprite(
                    (g_gameBoard.mystery.col * GRID_SIZE) + GRID_OFFSET,
                    (g_gameBoard.mystery.row * GRID_SIZE) + GRID_OFFSET,
                    MYSTERY_SIZE,
                    MYSTERY_SIZE,
                    pixel_mystery
                    );
            G8RTOS_SignalSemaphore(&sem_SPIA);

            if (Check_Ship_Mystery_Collision(&g_gameBoard.ship, &g_gameBoard.mystery)) {
                asteroid_spawn_counter = 0;

                ST7789_EraseSprite(
                                    (g_gameBoard.mystery.col * GRID_SIZE) + GRID_OFFSET,
                                    (g_gameBoard.mystery.row * GRID_SIZE) + GRID_OFFSET,
                                    MYSTERY_SIZE,
                                    MYSTERY_SIZE,
                                    pixel_mystery
                                    );
                g_gameBoard.mystery.alive = false;
                G8RTOS_SignalSemaphore(&sem_Mystery);
            }

        }





        // Iterate through the entire static pool

        for (i = 0; i < MAX_BULLETS; i++) {

            // Only process ACTIVE bullets
            if (g_bulletPool[i].active) {

                // 1. Erase OLD position
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_DrawRectangle(
                    (g_bulletPool[i].col * GRID_SIZE) + GRID_OFFSET,
                    (g_bulletPool[i].row * GRID_SIZE) + GRID_OFFSET,
                    BULLET_SIZE,
                    BULLET_SIZE,
                    ~ST7789_BLACK
                );
                G8RTOS_SignalSemaphore(&sem_SPIA);

                // 2. Update Position
                g_bulletPool[i].row += g_bulletPool[i].v_row;
                g_bulletPool[i].col += g_bulletPool[i].v_col;

                // 3. Check Boundaries
                if (g_bulletPool[i].row < 3
                    || g_bulletPool[i].col < 0 ||
                    g_bulletPool[i].col > MAX_X_GRID) {

                    g_bulletPool[i].active = false;
                    continue;
                }

                bool hit = false;
                for (j = 0; j < MAX_ASTEROIDS; j++) {
                    if (g_AsteroidPool[j].active) {
                        if (Check_Bullet_Asteroid_Collision(&g_bulletPool[i], &g_AsteroidPool[j])) {

                            // COLLISION DETECTED
                            hit = true;

                            // Deactivate both
                            g_bulletPool[i].active = false;
                            g_AsteroidPool[j].active = false;

                            // Erase the asteroid immediately (Bullet is already erased above)
                            G8RTOS_WaitSemaphore(&sem_SPIA);
                            ST7789_DrawRectangle(
                                (g_AsteroidPool[j].col * GRID_SIZE) + GRID_OFFSET,
                                (g_AsteroidPool[j].row * GRID_SIZE) + GRID_OFFSET,
                                ASTEROID_SIZE, ASTEROID_SIZE,
                                ~ST7789_BLACK);
                            G8RTOS_SignalSemaphore(&sem_SPIA);

                        }

                    }
                }

                if (hit)  {
                    score += 100;
                    G8RTOS_SignalSemaphore(&sem_gameState);
                    char output[64];

                    snprintf(output, sizeof(output), "Score: %d", score);
                    G8RTOS_WaitSemaphore(&sem_SPIA);
                    ST7789_DrawRectangle(10, 20, 95, 15, ~ST7789_BLACK);
                    ST7789_DrawStringStatic(output, ~ST7789_WHITE, 10, 20);
                    G8RTOS_SignalSemaphore(&sem_SPIA);
                    continue;
                }

                // 4. Draw NEW position
                if (lazer) {
                    G8RTOS_WaitSemaphore(&sem_SPIA);
                    ST7789_DrawSprite(
                                    (g_bulletPool[i].col * GRID_SIZE) + GRID_OFFSET,
                                    (g_bulletPool[i].row * GRID_SIZE) + GRID_OFFSET,
                                    BULLET_SIZE,
                                    BULLET_SIZE,
                                    lazer_pixel
                    );
                    G8RTOS_SignalSemaphore(&sem_SPIA);

                }
                else {
                    G8RTOS_WaitSemaphore(&sem_SPIA);
                    ST7789_DrawSprite(
                                    (g_bulletPool[i].col * GRID_SIZE) + GRID_OFFSET,
                                    (g_bulletPool[i].row * GRID_SIZE) + GRID_OFFSET,
                                    BULLET_SIZE,
                                    BULLET_SIZE,
                                    pixel_bullet
                                );
                    G8RTOS_SignalSemaphore(&sem_SPIA);

                }

            }
        }
     }

        G8RTOS_SignalSemaphore(&sem_gameState);
        sleep(current_spawn_rate);
    }
}


void Mystery_Box_Thread(void) {
    while (1) {
        G8RTOS_WaitSemaphore(&sem_Mystery);
        int powerup_type = rand() % 3;

        if (powerup_type == 0) {
            current_spawn_rate = 300;
            sleep(5000);

            current_spawn_rate = SPAWN_RATE;
        }

        else if (powerup_type == 1){
            multi_shot_active = true;

            sleep(5000);
            multi_shot_active = false;
        }

        else {
            lazer = true;

            sleep(5000);
            lazer = false;
        }

    }
}


void Game_Over_Thread(void) {
    while (1) {
        G8RTOS_WaitSemaphore(&sem_gameOver);

        G8RTOS_WaitSemaphore(&sem_SPIA);
        ST7789_Fill(~ST7789_BLUE);
        G8RTOS_SignalSemaphore(&sem_SPIA);
        game_over = true;
        char output[64];
        snprintf(output, sizeof(output), "Restart? Click Switch 3!");
        G8RTOS_WaitSemaphore(&sem_SPIA);
        ST7789_DrawStringStatic(output, ~ST7789_WHITE, 30, 120);
        G8RTOS_SignalSemaphore(&sem_SPIA);

        snprintf(output, sizeof(output), "High Score: %d", score);
        G8RTOS_WaitSemaphore(&sem_SPIA);
        ST7789_DrawStringStatic(output, ~ST7789_WHITE, 30, 150);
        G8RTOS_SignalSemaphore(&sem_SPIA);
    }
}

/********************************Background Threads*********************************/




/********************************Periodic Threads***********************************/



void Read_Joystick(void) {
    uint32_t xy = JOYSTICK_GetXY();
    G8RTOS_WriteFIFO(JOYSTICK_FIFO, xy);
}



/********************************Periodic Threads***********************************/


/*******************************Aperiodic Threads***********************************/

void Button_Handler(void) {
    // Clear interrupt
    GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_4);

    GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_4);
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}


//
//void GPIOE_Handler() {
//     GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_4);
//     G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
//}

void GPIOD_Handler() {
     GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
     G8RTOS_SignalSemaphore(&sem_Joystick_Debounce);

}

/*******************************Aperiodic Threads***********************************/



