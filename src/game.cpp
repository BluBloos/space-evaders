#include <iostream>
#include <vector>
#include "player.cpp"
#include "entity.cpp"
#include "ground.cpp"
#include "enemy.cpp"
#include "game.h"
#include "resource.h"
#include "star.cpp"
#include "oxygenTank.cpp"
#include <iostream>

#define PLAYER_SPAWN (Vector2){0.0f, 400.0f}
#define PLAYER_CHARACTER_INDEX 0

// Define the things that happen when the game is initialized.
Game::Game() {
    std::cout << "Game has been initialized\n\n";

    // Create game entities.
    this->characters.push_back(new Player(PLAYER_SPAWN, 0));
    Enemy *evil_enemy = new Enemy((Vector2){700.0f, 200.0f}, 0);
    evil_enemy->SetMovable(true, 0, -1, 100, 100);
    this->characters.push_back((Entity *)evil_enemy);
    this->grounds.push_back(new Ground((Vector2){-20.0f, 400.0f}, 0, 4000, 200)); // NOTE: We made this big to test camera movement :)

    // Add a moveable platform for player to jump onto!
    Ground *epic_ground = new Ground((Vector2){400.0f, 200.0f}, 0, 200, 100);
    epic_ground->SetMovable(true, -1, 0, 100, 100);
    //epic_ground->SetOscillation(); // set the platform to go back and forth in movement
    this->grounds.push_back((Entity *)epic_ground);

    this->onTitle = true;
    this->controlFlag = true; // control flag to swap between WASD and arrow keys
    this->settingsFlag = false; // settings flag to display settings

    this->camera = { 0 };
    this->camera.target = this->characters[0]->GetPos();
    camera.offset = (Vector2){ SCREENWIDTH/2.0f, SCREENHEIGHT/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    this->timeCount = 0;
    this->oxygenRemaining = 100;
    this->tanks.push_back(tank(400, 30));

    this->moonTexture = LoadTexture("../arts/moon.png");
}

std::vector<Entity *> Game::GetGrounds() {
    return this->grounds;
}

std::vector<Entity *> Game::GetCharacters() {
    return this->characters;
}

float Game::GetLastFrameTime() {
    return GetFrameTime();
}

// Define what will happen each frame of the game.
void Game::GameUpdateAndRender() {

    // oxygen timer
    if ((GetTime() - timeCount) > 1){
        timeCount = GetTime();
        oxygenRemaining--;
    }

	for (int i = 0; i < 200; i++) {
		this->stars[i].x -= 12 * (stars[i].z / 1);

		if (this->stars[i].x <= 0) {  // Check if the star has gone off screen
			this->stars[i].x += 2000;
			this->stars[i].y = GetRandomValue(0, 1000);
		}
	}

    BeginDrawing(); 
    ClearBackground(BLACK);

    for (int i = 0; i < 200; i++) {
		float x = this -> stars[i].x;
		float y = this -> stars[i].y;

		DrawRectangle(x, y, 8*stars[i].z, 8*stars[i].z, RAYWHITE);
	}

    if (onTitle){ // still on title screen
        showTitle();
        if (IsKeyPressed(KEY_ENTER)){ // if enter is pressed, start game
            onTitle = false;
            UnloadTexture(this->titleSprite); // unload sprite
        }
    }

    else { // no longer on title screen

        // Calling raylib function GetFrameTime to return the time in seconds for the last frame drawn.
        float deltaTime = GetFrameTime();


        if (IsKeyPressed(KEY_P)) { // if user hits escape
            this->setSettingsFlag(); // switch settings flag
        }

        if (gameOver){
            showGameOver();
            if (IsKeyPressed(KEY_ENTER)) { switchGameOver(); }
        }
        else {

            if (settingsFlag){
                showSettings();
            }

            else {

                this->updateCameraSmoothFollowInsideMap(deltaTime);
                BeginMode2D(this->camera);


                // Go through loop of all entities and call update.
                for (unsigned int i = 0; i < this->grounds.size(); i++) {
                    Entity *entity = this->grounds[i];
                    entity->update(this);
                }
                for (unsigned int i = 0; i < this->characters.size(); i++) {
                    Entity *entity = this->characters[i];
                    entity->update(this);
                }

                Vector2 coords = characters[0]->GetPos();
                for (unsigned int i = 0; i < this->tanks.size(); i++) {
                    tanks[i].showTank();
                    if ((coords.x >= (tanks[i].getX() - 25)) && (coords.x <= (tanks[i].getX() + 25)) && (coords.y >= (tanks[i].getY() - 15)) && (coords.y <= (tanks[i].getY() + 35))){
                        if (!tanks[i].getCollected()) {this->tankRefill(); }
                        // if player touches a tank, set tank to collected
                        tanks[i].isCollected();
                    }
                }

                DrawText(FormatText("Oxygen Remaining: %i", this->getO2()), camera.target.x + 103, camera.target.y - 295, 30, RAYWHITE);

                EndMode2D();
            }
        }
    }

    EndDrawing();
}

// NOTE(Noah): Camera speed must be exactly player speed to avoid buggy behaviour.
// Overall this type of camera moement feels nice!!!!
void Game::updateCameraSmoothFollowInsideMap(float delta){
    float minSpeed = 600.0f;
    float minEffectLength = 5.0f;
    float fractionSpeed = 0.8f;

    Vector2 playerPos = this->characters[0]->GetPos();
    //this->camera.offset = (Vector2){ SCREENWIDTH/2.0f - OFFSETCORRECTVALUE, SCREENHEIGHT/2.0f };
    Vector2 diff = Vector2Subtract(playerPos, this->camera.target);

    float fringeLenX = 200.0f; // 50 pixels of the sides of screen.
    float fringeLenY = 100.0f;
    bool playerOnFringeX = abs(this->camera.target.x - playerPos.x) > SCREENWIDTH/2.0f - fringeLenX;
    bool playerOnFringeY = abs(this->camera.target.y - playerPos.y) > SCREENHEIGHT/2.0f - fringeLenY;

    // adjust the diff accordingly, to only move in specific dir.
    if (playerOnFringeX && !playerOnFringeY) {
        diff.y = 0; 
    } else if (playerOnFringeY && !playerOnFringeX) {
        diff.x = 0;
    } else if (!playerOnFringeY && !playerOnFringeX) {
        diff.x = 0; diff.y = 0;
    }

    float length = Vector2Length(diff);

    if (length > minEffectLength) {
        float speed = fmaxf(fractionSpeed*length, minSpeed);
        this->camera.target = Vector2Add(this->camera.target, Vector2Scale(diff, speed*delta/length));
    }
}

// Define the things that will happen when the game is closed.
Game::~Game() {

    // Clean up entities.
    for (unsigned int i = 0; i < this->characters.size(); i++) {
        Entity *entity = this->characters[i];
        delete entity;
    }

    for (unsigned int i = 0; i < this->grounds.size(); i++) {
        Entity *entity = this->grounds[i];
        delete entity;
    }

    // unload the moon texture.
    UnloadTexture(this->moonTexture);

    std::cout << "Game has been closed\n";
}

void Game::showGameOver() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), RED);
    DrawText("Game Over", GetScreenWidth()*0.225, GetScreenHeight()*0.3, 100, RAYWHITE);
    DrawText("Press Enter to Respawn", GetScreenWidth()*0.225, GetScreenHeight()*0.6, 40, RAYWHITE);
}

void Game::switchGameOver() { 
    
    if (this->gameOver == false) {
        // we are going from not gameover to a gameover state.
        // thus, we reset the position of the player!
        this->characters[PLAYER_CHARACTER_INDEX]->SetPos(PLAYER_SPAWN);
    }
    this->gameOver = !this->gameOver;

}

bool Game::getGameOver() { return this->gameOver; }
void Game::showTitle() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    DrawText("Space Evaders", GetScreenWidth()*0.1, GetScreenHeight()*0.3, 95, RAYWHITE);
    DrawText("Press Enter to Start", GetScreenWidth()*0.2, GetScreenHeight()*0.8, 50, RAYWHITE);
    // DrawTexture(this->titleSprite, GetScreenWidth()*0.25, GetScreenHeight()*0.7, RAYWHITE);
}
void Game::showSettings() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    DrawText("Controls", GetScreenWidth()*0.325, GetScreenHeight()*0.1, 75, RAYWHITE);
    DrawText("Press Enter to Switch Movement", GetScreenWidth()*0.1, GetScreenHeight()*0.4, 40, RAYWHITE);

    if (IsKeyPressed(KEY_ENTER)){ setControlFlag(); }

    if (getControlFlag()) {
        DrawText("Movement: WASD", GetScreenWidth()*0.1, GetScreenHeight()*0.5, 40, RAYWHITE);
    }

    else {
        DrawText("Movement: Arrow Keys", GetScreenWidth()*0.1, GetScreenHeight()*0.5, 40, RAYWHITE);
    }

    DrawText("Jump: Space or W/Up Arrow", GetScreenWidth()*0.1, GetScreenHeight()*0.6, 40, RAYWHITE);
    DrawText("Gravity Flip: F", GetScreenWidth()*0.1, GetScreenHeight()*0.7, 40, RAYWHITE);
    DrawText("Black Hole: G", GetScreenWidth()*0.1, GetScreenHeight()*0.8, 40, RAYWHITE);
}

void Game::setSettingsFlag() {
    this->settingsFlag = !this->settingsFlag; // switch flag
}

void Game::setControlFlag() {
    this->controlFlag = !this->controlFlag; // switch control flag
}

bool Game::getControlFlag() { return this->controlFlag; }

int Game::getO2() { return oxygenRemaining; }
void Game::tankRefill() {
    this->oxygenRemaining = this->oxygenRemaining + 50;
}