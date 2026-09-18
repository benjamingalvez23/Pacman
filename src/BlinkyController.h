
#pragma once

#include "Controller.h"
#include "FSM.h"

#include <memory>
#include <chrono>

// ============================================================
// BLINKY STATE MACHINE
// ============================================================

class BlinkyStateMachine;


// ============================================================
// ESTADOS
// ============================================================

class BlinkyScatterState : public FSMState {
public:
    BlinkyScatterState(std::shared_ptr<Character> character);

    void onEnter(const GameState& game) override;
    Move onUpdate(const GameState& game) override;
};


class BlinkyChaseState : public FSMState {
public:
    BlinkyChaseState(std::shared_ptr<Character> character);

    void onEnter(const GameState& game) override;
    Move onUpdate(const GameState& game) override;
};


class BlinkyElroyState : public FSMState {
public:
    BlinkyElroyState(
        std::shared_ptr<Character> character,
        float speedMultiplier
    );

    void onEnter(const GameState& game) override;
    void onExit(const GameState& game) override;
    Move onUpdate(const GameState& game) override;

private:
    float speedMultiplier;
};


class BlinkyFleeState : public FSMState {
public:
    BlinkyFleeState(std::shared_ptr<Character> character);

    void onEnter(const GameState& game) override;
    Move onUpdate(const GameState& game) override;
};


// ============================================================
// TRANSICIONES
// ============================================================

class BlinkyTimeTransition : public FSMTransition {
public:
    BlinkyTimeTransition(
        std::shared_ptr<FSMState> next,
        float seconds
    );

    bool isValid(const GameState& game) override;
    std::shared_ptr<FSMState> getNextState() override;
    void onTransition(const GameState& game) override;

private:
    std::shared_ptr<FSMState> nextState;

    float duration;

    bool started;

    std::chrono::steady_clock::time_point startTime;
};


class BlinkyPillTransition : public FSMTransition {
public:
    BlinkyPillTransition(
        std::shared_ptr<FSMState> next,
        int threshold
    );

    bool isValid(const GameState& game) override;
    std::shared_ptr<FSMState> getNextState() override;

private:
    std::shared_ptr<FSMState> nextState;

    int threshold;

    bool activated;

    int previousPills;
};


class BlinkyVulnerableTransition : public FSMTransition {
public:
    BlinkyVulnerableTransition(
        std::shared_ptr<FSMState> next
    );

    bool isValid(const GameState& game) override;
    std::shared_ptr<FSMState> getNextState() override;

private:
    std::shared_ptr<FSMState> nextState;
};


class BlinkyRecoverTransition : public FSMTransition {
public:
    BlinkyRecoverTransition(
        std::shared_ptr<FSMState> next
    );

    bool isValid(const GameState& game) override;
    std::shared_ptr<FSMState> getNextState() override;

private:
    std::shared_ptr<FSMState> nextState;
};


// ============================================================
// FSM DE BLINKY
// ============================================================

class BlinkyStateMachine : public FiniteStateMachine {
public:
    BlinkyStateMachine(
        std::shared_ptr<Character> character
    );

    Move update(const GameState& game) override;

    ~BlinkyStateMachine();
};


// ============================================================
// CONTROLLER
// ============================================================

class BlinkyController : public Controller {
public:
    BlinkyController(
        std::shared_ptr<Character> character
    );

    virtual ~BlinkyController();

    Move getMove(
        const GameState& game
    ) override;

private:
    std::shared_ptr<BlinkyStateMachine> fsm;
};
