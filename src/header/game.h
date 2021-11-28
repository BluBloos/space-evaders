#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include <vector>

class Game {
    
    public:
        Game(); // Constructor
        ~Game(); // Destructor
    
        // Function for game to implement and is to be called for each frame
        // to be rendered.    
        void GameUpdateAndRender();

        std::vector<Entity *> GetGrounds(); // Will return all the ground entities
        std::vector<Entity *> GetCharacters(); // Will return all the ground entities
        float GetLastFrameTime();

    private:
        std::vector<Entity *> characters;
        std::vector<Entity *> grounds;
};

#endif



