#include "PinkyController.h"

#include <iostream>
#include <cmath>
#include <limits>
 
extern bool nogui;
 
PinkyInfo* PinkyInfo::info = nullptr;
 
// ==================== PINKY CONTROLLER ====================
 
PinkyController::PinkyController(std::shared_ptr<Character> character) :
    Controller(character), root(std::make_shared<Selector>()) {
    
    // Árbol de comportamiento para Pinky
    // 1. Si está asustado, huir aleatoriamente
    auto frightenedFilter = std::make_shared<Filter>();
    frightenedFilter->addCondition(std::make_shared<PinkyEdibleCondition>());
    frightenedFilter->addAction(std::make_shared<PinkyFrightened>());
    root->addChild(frightenedFilter);
    
    // 2. Si hay pocas pills, cazar power pills
    auto powerPillFilter = std::make_shared<Filter>();
    powerPillFilter->addCondition(std::make_shared<PinkyLowPillsCondition>());
    powerPillFilter->addAction(std::make_shared<PinkyPowerPillHunt>());
    root->addChild(powerPillFilter);
    
    // 3. Mantener rango óptimo con Pac-Man
    root->addChild(std::make_shared<PinkyOptimalRange>());
}
 
PinkyController::~PinkyController() {
}
 
Move PinkyController::getMove(const GameState& game) {
    PinkyInfo::getInfo()->in_character = character;
    PinkyInfo::getInfo()->in_gamestate = &game;
    
    root->tick();
    
    return PinkyInfo::getInfo()->out_move;
}
 
// ==================== BEHAVIORS ====================
 
Status PinkyOptimalRange::update() {
    // Pinky intenta mantenerse a una distancia óptima de Pac-Man (~4 nodos)
    auto character = PinkyInfo::getInfo()->in_character;
    auto gs = PinkyInfo::getInfo()->in_gamestate;
    
    auto pacmanPos = gs->getPacmanPos();
    auto pacmanNodePos = gs->getMaze().getNodePos(pacmanPos);
    
    float minDifference = std::numeric_limits<float>::max();
    Move bestMove = PASS;
    
    const float OPTIMAL_DISTANCE = 4.0f; // Distancia óptima
    
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    
    for (auto move : moves) {
        if (move == PASS) break;
        
        int nextNode = gs->getMaze().getNeighbour(character->getPos(), move);
        auto nextNodePos = gs->getMaze().getNodePos(nextNode);
        float dist = euclid2(pacmanNodePos, nextNodePos);
        
        // Calcular diferencia respecto a distancia óptima
        float difference = std::abs(dist - OPTIMAL_DISTANCE);
        
        if (difference < minDifference) {
            minDifference = difference;
            bestMove = move;
        }
    }
    
    PinkyInfo::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}
 
Status PinkyChase::update() {
    // Persecución directa a Pac-Man
    auto character = PinkyInfo::getInfo()->in_character;
    auto gs = PinkyInfo::getInfo()->in_gamestate;
    
    auto pacmanPos = gs->getPacmanPos();
    auto pacmanNodePos = gs->getMaze().getNodePos(pacmanPos);
    
    float minDist = 1000000000;
    Move bestMove = PASS;
    
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    
    for (auto move : moves) {
        if (move == PASS) break;
        
        int nextNode = gs->getMaze().getNeighbour(character->getPos(), move);
        auto nextNodePos = gs->getMaze().getNodePos(nextNode);
        float dist = euclid2(pacmanNodePos, nextNodePos);
        
        if (dist < minDist) {
            minDist = dist;
            bestMove = move;
        }
    }
    
    PinkyInfo::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}
 
PinkyScatter::PinkyScatter() : target(-1, -1) {
}
 
Status PinkyScatter::update() {
    // Dispersión hacia esquina superior izquierda
    auto character = PinkyInfo::getInfo()->in_character;
    auto gs = PinkyInfo::getInfo()->in_gamestate;
    
    if (target.first == -1) {
        target = gs->getMaze().getNodePos(0);
    }
    
    float minDist = 1000000000;
    Move bestMove = PASS;
    
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    
    for (auto move : moves) {
        if (move == PASS) break;
        
        int nextNode = gs->getMaze().getNeighbour(character->getPos(), move);
        auto nextNodePos = gs->getMaze().getNodePos(nextNode);
        float dist = euclid2(target, nextNodePos);
        
        if (dist < minDist) {
            minDist = dist;
            bestMove = move;
        }
    }
    
    PinkyInfo::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}
 
Status PinkyFrightened::update() {
    // Movimiento aleatorio cuando está asustado
    auto character = PinkyInfo::getInfo()->in_character;
    auto gs = PinkyInfo::getInfo()->in_gamestate;
    
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    
    if (moves.empty()) {
        PinkyInfo::getInfo()->out_move = PASS;
        return BH_SUCCESS;
    }
    
    Move m = moves[rand() % moves.size()];
    PinkyInfo::getInfo()->out_move = m;
    return BH_SUCCESS;
}
 
Status PinkyPowerPillHunt::update() {
    // Inky busca y come power pills para ayudar a Pac-Man
    auto character = PinkyInfo::getInfo()->in_character;
    auto gs = PinkyInfo::getInfo()->in_gamestate;
    
    auto powerPillPositions = gs->getMaze().getPowerPillPositions();
    
    if (powerPillPositions.empty()) {
        return BH_FAILURE; // No hay power pills
    }
    
    // Buscar la power pill más cercana
    float minDist = 1000000000;
    std::pair<int, int> closestPill = powerPillPositions[0];
    
    for (auto pill : powerPillPositions) {
        float dist = euclid2(gs->getMaze().getNodePos(character->getPos()), pill);
        if (dist < minDist) {
            minDist = dist;
            closestPill = pill;
        }
    }
    
    float minMoveDist = 1000000000;
    Move bestMove = PASS;
    
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    
    for (auto move : moves) {
        if (move == PASS) break;
        
        int nextNode = gs->getMaze().getNeighbour(character->getPos(), move);
        auto nextNodePos = gs->getMaze().getNodePos(nextNode);
        float dist = euclid2(closestPill, nextNodePos);
        
        if (dist < minMoveDist) {
            minMoveDist = dist;
            bestMove = move;
        }
    }
    
    PinkyInfo::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}
 
// ==================== CONDITIONS ====================
 
Status PinkyEdibleCondition::update() {
    auto character = PinkyInfo::getInfo()->in_character;
    auto ghost = dynamic_cast<Ghost*>(character.get());
    
    if (ghost != nullptr && ghost->isEdible()) {
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}
 
Status PinkyLowPillsCondition::update() {
    auto gs = PinkyInfo::getInfo()->in_gamestate;
    
    // Si quedan menos de 50 pills, ir por power pills
    int totalPills = gs->getMaze().getPillPositions().size() + 
                     gs->getMaze().getPowerPillPositions().size();
    
    if (totalPills < 50) {
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}
 