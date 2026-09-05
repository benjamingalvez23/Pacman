/*
 * FSMController.cpp
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#include "FSMController.h"
#include <iostream>

FSMController::FSMController(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<ExampleStateMachine>(character)) {
}

FSMController::~FSMController() {
	// TODO Auto-generated destructor stub
}

Move 
FSMController::getMove(const GameState& game){
	return fsm->update(game);
}


///////////////////////////////////PillTransition///////////////////////////////
PillTransition::PillTransition(std::shared_ptr<FSMState> next):last(0),_next(next){
 
}

bool PillTransition::isValid(const GameState& gs){
	int quedan=gs.getMaze().getPillPositions().size();
	if(last!=quedan && quedan%20==0){
		last =quedan;
		return true;
	}
	if(last == -1){
		last = quedan;
        return false;
	}
	return false;
}
std::shared_ptr<FSMState> PillTransition::getNextState(){
	return _next;
}



///////////////////////////////ChaseState///////////////////////////////////////
ChaseState::ChaseState(std::shared_ptr<Character> _character): FSMState(_character) {}

void ChaseState::onEnter(const GameState&) {
    auto ghost = std::dynamic_pointer_cast<Ghost>(character);
    if (ghost) {
        ghost->revert();
    }
}

Move ChaseState::onUpdate(const GameState& game) {
    std::vector<Move> moves;
    const auto pacmanCoord = game.getMaze().getNodePos(game.getPacmanPos());
    const auto myPos = character->getPos();

    if (character->getDirection() == PASS) {
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }

    if (moves.empty()) return PASS;

    float min = euclid2(
        game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[0])),
        pacmanCoord
    );
    int minI = 0;
    for (unsigned int i = 1; i < moves.size(); i++) {
        auto dist = euclid2(
            game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])),
            pacmanCoord
        );
        if (dist < min) {
            min = dist;
            minI = i;
        }
    }
    return moves[minI];
}

ChaseState::~ChaseState() {}

ScatterState::ScatterState(std::shared_ptr<Character> _character): FSMState(_character) {}

Move ScatterState::onUpdate(const GameState& game) {
    std::vector<Move> moves;
    // Objetivo: Esquina superior/nodo 0 para el fantasma
    const auto scatterTarget = game.getMaze().getNodePos(0); 
    const auto myPos = character->getPos();

    if (character->getDirection() == PASS) {
        moves = game.getMaze().getPossibleMoves(myPos);
    } else {
        moves = game.getMaze().getGhostLegalMoves(myPos, character->getDirection());
    }

    if (moves.empty()) return PASS;

    float minDist = euclid2(
        game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[0])),
        scatterTarget
    );
    int minI = 0;

    for (unsigned int i = 1; i < moves.size(); i++) {
        auto dist = euclid2(
            game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos, moves[i])),
            scatterTarget
        );
        if (dist < minDist) {
            minDist = dist;
            minI = i;
        }
    }
    return moves[minI];
}

ScatterState::~ScatterState() {}

NonfrightenedStateMachine::NonfrightenedStateMachine(std::shared_ptr<Character> _character):
    FiniteStateMachine(_character) {
    
    auto chase = std::make_shared<ChaseState>(character);
    auto scatter = std::make_shared<ScatterState>(character);

    // Transición recíproca entre Chase y Scatter según el consumo de píldoras
    chase->addTransition(std::make_shared<PillTransition>(scatter));
    scatter->addTransition(std::make_shared<PillTransition>(chase));

    initialState = chase;
    activeState = initialState;

    states.push_back(chase);
    states.push_back(scatter);
}

Move NonfrightenedStateMachine::update(const GameState& game) {
    auto t = activeState->getActiveTransition(game);
    if (t != nullptr) {
        activeState->onExit(game);
        t->onTransition(game);
        activeState = t->getNextState();
        activeState->onEnter(game);
    }
    return activeState->onUpdate(game);
}

NonfrightenedStateMachine::~NonfrightenedStateMachine() {}

NonfrightenedState::NonfrightenedState(std::shared_ptr<Character> _character):
    FSMState(_character),
    fsm(std::make_shared<NonfrightenedStateMachine>(_character)) {
}

Move NonfrightenedState::onUpdate(const GameState& game) {
    return fsm->update(game);
}

void NonfrightenedState::onEnter(const GameState& game) {}

void NonfrightenedState::onExit(const GameState& game) {}

NonfrightenedState::~NonfrightenedState() {}

/////////////////////////////////////BlinkyStateMachine/////////////////////////////
ExampleStateMachine::ExampleStateMachine(std::shared_ptr<Character> _character):FiniteStateMachine(_character){
	initialState = std::make_shared<NonfrightenedState>(character);
	activeState=initialState;
	states.push_back(initialState);
}

Move ExampleStateMachine::update(const GameState& gs) {
    auto t = activeState->getActiveTransition(gs);
    if (t != nullptr) {
        activeState->onExit(gs);
        t->onTransition(gs);
        activeState = t->getNextState();
        activeState->onEnter(gs);
    }
    return activeState->onUpdate(gs);
}

ExampleStateMachine::~ExampleStateMachine() {}