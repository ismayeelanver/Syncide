import "raylib";

const SCREEN_WIDTH  = 800;
const SCREEN_HEIGHT = 600;
const PLAYER_SIZE   = 50;
const BLOCK_SIZE    = 50;
const BLOCK_SPEED   = 5;

// Game State Structures
Player :: struct {
    x: s32;
    y: s32;
    size: s32;
};

Block :: struct {
    x: s32;
    y: s32;
    size: s32;
    speed: s32;
};

Main :: () {
    // Initialize the game window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Avoid the Falling Blocks");
    SetTargetFPS(60);

    // Initialize the player and block
    player := Player{
        x = SCREEN_WIDTH / 2 - PLAYER_SIZE / 2,
        y = SCREEN_HEIGHT - PLAYER_SIZE - 20,
        size = PLAYER_SIZE,
    };

    block := Block{
        x = GetRandomValue(0, SCREEN_WIDTH - BLOCK_SIZE),
        y = -BLOCK_SIZE,
        size = BLOCK_SIZE,
        speed = BLOCK_SPEED,
    };

    game_over := false;

    while !WindowShouldClose() {
        if game_over {
            // Display "Game Over" screen
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Game Over!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 40, RED);
            DrawText("Press R to Restart", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 30, 20, DARKGRAY);
            EndDrawing();

            if IsKeyPressed(KEY_R) {
                // Restart game
                game_over = false;
                block.y = -BLOCK_SIZE;
                block.x = GetRandomValue(0, SCREEN_WIDTH - BLOCK_SIZE);
            }
            continue;
        }

        // Player Movement
        if IsKeyDown(KEY_LEFT) and player.x > 0 {
            player.x -= 10;
        }
        if IsKeyDown(KEY_RIGHT) and player.x < SCREEN_WIDTH - player.size {
            player.x += 10;
        }

        // Update Block Position
        block.y += block.speed;

        if block.y > SCREEN_HEIGHT {
            // Reset block to the top
            block.y = -BLOCK_SIZE;
            block.x = GetRandomValue(0, SCREEN_WIDTH - BLOCK_SIZE);
        }

        // Collision Detection
        if player.x < block.x + block.size and
           player.x + player.size > block.x and
           player.y < block.y + block.size and
           player.y + player.size > block.y {
            game_over = true;
        }

        // Render the game
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw Player
        DrawRectangle(player.x, player.y, player.size, player.size, GREEN);

        // Draw Block
        DrawRectangle(block.x, block.y, block.size, block.size, RED);

        // Display instructions and score
        DrawText("Use Arrow Keys to Move", 10, 10, 20, DARKGRAY);
        if game_over {
            DrawText("Game Over!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 40, RED);
        }

        EndDrawing();
    }

    CloseWindow(); // Close the game window
}
