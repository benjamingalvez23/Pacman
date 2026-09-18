#include "InkyController.h"
#include <iostream>
#include <cmath>

InkyInfo* InkyInfo::info = nullptr;

InkyController::InkyController(std::shared_ptr<Character> character) :
    Controller(character), root(std::make_shared<Selector>()) {
    
    auto edibleFilter = std::make_shared<Filter>();
    edibleFilter->addCondition(std::make_shared<InkyConditionEdible>());
    edibleFilter->addAction(std::make_shared<InkyFrightened>());
    root->addChild(edibleFilter);
    
    auto notEdibleFilter = std::make_shared<Filter>();
    notEdibleFilter->addCondition(std::make_shared<InkyConditionNotEdible>());
    notEdibleFilter->addAction(std::make_shared<InkyIntercept>());
    root->addChild(notEdibleFilter);
    
    root->addChild(std::make_shared<InkyChase>());
}

InkyController::~InkyController() {}

Move InkyController::getMove(const GameState& game) {
    InkyInfo::getInfo()->in_character = character;
    InkyInfo::getInfo()->in_gamestate = &game;
    root->tick();
    return InkyInfo::getInfo()->out_move;
}

Status InkyIntercept::update() {
    auto character = InkyInfo::getInfo()->in_character;
    auto gs = InkyInfo::getInfo()->in_gamestate;
    
    auto pacmanPos = gs->getPacmanPos();
    auto pacmanNodePos = gs->getMaze().getNodePos(pacmanPos);
    
    auto predictedPos = pacmanNodePos;
    predictedPos.first += 40; 
    
    float minDist = 1000000000;
    Move bestMove = PASS;
    
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    
    for (auto move : moves) {
        if (move == PASS) continue;
        
        int nextNode = gs->getMaze().getNeighbour(character->getPos(), move);
        if (nextNode == -1) continue; // Protección de límites

        auto nextNodePos = gs->getMaze().getNodePos(nextNode);
        float dist = euclid2(predictedPos, nextNodePos);
        
        if (dist < minDist) {
            minDist = dist;
            bestMove = move;
        }
    }
    
    InkyInfo::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}

Status InkyChase::update() {
    auto character = InkyInfo::getInfo()->in_character;
    auto gs = InkyInfo::getInfo()->in_gamestate;
    
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
        if (move == PASS) continue;
        
        int nextNode = gs->getMaze().getNeighbour(character->getPos(), move);
        if (nextNode == -1) continue; // Protección de límites

        auto nextNodePos = gs->getMaze().getNodePos(nextNode);
        float dist = euclid2(pacmanNodePos, nextNodePos);
        
        if (dist < minDist) {
            minDist = dist;
            bestMove = move;
        }
    }
    
    InkyInfo::getInfo()->out_move = bestMove;
    return BH_SUCCESS;
}

Status InkyFrightened::update() {
    auto character = InkyInfo::getInfo()->in_character;
    auto gs = InkyInfo::getInfo()->in_gamestate;
    
    std::vector<Move> moves;
    if (character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } else {
        moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
    }
    
    if (moves.empty()) {
        InkyInfo::getInfo()->out_move = PASS;
        return BH_SUCCESS;
    }
    
    Move m = moves[rand() % moves.size()];
    InkyInfo::getInfo()->out_move = m;
    return BH_SUCCESS;
}

Status InkyConditionEdible::update() {
    auto character = InkyInfo::getInfo()->in_character;
    auto ghost = dynamic_cast<Ghost*>(character.get());
    
    if (ghost != nullptr && ghost->isEdible()) {
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}

Status InkyConditionNotEdible::update() {
    auto character = InkyInfo::getInfo()->in_character;
    auto ghost = dynamic_cast<Ghost*>(character.get());
    
    if (ghost != nullptr && !ghost->isEdible()) {
        return BH_SUCCESS;
    }
    return BH_FAILURE;
}