#include "PacmanDTController.h"
#include <iostream>

PacmanDTController::PacmanDTController(std::shared_ptr<Character> character):
	Controller(character){
}

PacmanDTController::~PacmanDTController() {
	// TODO Auto-generated destructor stub
}

Move
PacmanDTController::getMove(const GameState& game){
    int pacmanNode = game.getPacmanPos();
    
    auto pacmanCoords = game.getMaze().getNodePos(pacmanNode);
    
   
    std::vector<Move> moves = game.getMaze().getPossibleMoves(pacmanNode);
    if(moves.empty()) {
        return PASS;
    }

   
    bool algunFantasmaComestible = false;
    for(int i = 0; i < 4; i++) {
    
        if(game.isGhostEdible(i)) {
            algunFantasmaComestible = true;
            break;
        }
    }

    if(algunFantasmaComestible) {
       
        int minDist = 10000000;
        Move minMove = moves[0]; 
        
        for(Move m : moves) {
            int vecino = game.getMaze().getNeighbour(pacmanNode, m);

            auto vecinoCoords = game.getMaze().getNodePos(vecino);
            
            
            for(int i = 0; i < 4; i++) {
                if(game.isGhostEdible(i)) {
                    int ghostNode = game.getGhostsPos(i);
                    
                    auto ghostCoords = game.getMaze().getNodePos(ghostNode);
                    
                    int dx = vecinoCoords.first - ghostCoords.first;
                    int dy = vecinoCoords.second - ghostCoords.second;
                    int sqDist = dx * dx + dy * dy;
                    
                    if(sqDist < minDist) {
                        minDist = sqDist;
                        minMove = m;
                    }
                }
            }
        }
        return minMove;
        
    } else {
        
        int maxDist = -1;
        Move maxMove = moves[0]; 
        
        for(Move m : moves) {
            int vecino = game.getMaze().getNeighbour(pacmanNode, m);

            auto vecinoCoords = game.getMaze().getNodePos(vecino);
            int minDistanceToAnyGhost = 10000000;
            
            
            for(int i = 0; i < 4; i++) {
                if(!game.isGhostEdible(i)) {
                    int ghostNode = game.getGhostsPos(i);
                    
                    auto ghostCoords = game.getMaze().getNodePos(ghostNode);
                    
                    int dx = vecinoCoords.first - ghostCoords.first;
                    int dy = vecinoCoords.second - ghostCoords.second;
                    int sqDist = dx * dx + dy * dy;
                    
                    if(sqDist < minDistanceToAnyGhost) {
                        minDistanceToAnyGhost = sqDist;
                    }
                }
            }

            if(minDistanceToAnyGhost > maxDist) {
                maxDist = minDistanceToAnyGhost;
                maxMove = m;
            }
        }
        return maxMove;
    }
};
