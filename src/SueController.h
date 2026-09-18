#pragma once

#include "Controller.h"
#include "FSM.h"

#include <memory>
#include <chrono>

// ============================================================
// SUE STATE MACHINE
// ============================================================

class SueStateMachine;


// ============================================================
// ESTADOS
// ============================================================

class SueWaitState : public FSMState {
public:
    SueWaitState(
        std::shared_ptr<Character> character
    );

    void onEnter(const GameState& game) override;
    Move onUpdate(const GameState& game) override;
};


class SueScatterState : public FSMState {
public:
    SueScatterState(
        std::shared_ptr<Character> character
    );

    void onEnter(const GameState& game) override;
    Move onUpdate(const GameState& game) override;
};


class SueChaseState : public FSMState {
public:
    SueChaseState(
        std::shared_ptr<Character> character
    );

    void onEnter(const GameState& game) override;
    Move onUpdate(const GameState& game) override;
};


class SueFleeState : public FSMState {
public:
    SueFleeState(
        std::shared_ptr<Character> character
    );

    void onEnter(const GameState& game) override;
    Move onUpdate(const GameState& game) override;
};


// ============================================================
// TRANSICIONES
// ============================================================

class SueDotCountTransition : public FSMTransition {
public:
    SueDotCountTransition(
        std::shared_ptr<FSMState> next,
        int dotsRequired
    );

    bool isValid(const GameState& game) override;

    std::shared_ptr<FSMState> getNextState() override;

private:
    std::shared_ptr<FSMState> nextState;

    int dotsRequired;

    int initialPills;

    bool initialized;
};


// ------------------------------------------------------------

class SueTimeTransition : public FSMTransition {
public:
    SueTimeTransition(
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


// ------------------------------------------------------------

class SueVulnerableTransition : public FSMTransition {
public:
    SueVulnerableTransition(
        std::shared_ptr<FSMState> next
    );

    bool isValid(const GameState& game) override;

    std::shared_ptr<FSMState> getNextState() override;

private:
    std::shared_ptr<FSMState> nextState;
};


// ------------------------------------------------------------

class SueRecoverTransition : public FSMTransition {
public:
    SueRecoverTransition(
        std::shared_ptr<FSMState> next
    );

    bool isValid(const GameState& game) override;

    std::shared_ptr<FSMState> getNextState() override;

private:
    std::shared_ptr<FSMState> nextState;
};


// ============================================================
// FSM DE SUE
// ============================================================

class SueStateMachine : public FiniteStateMachine {
public:
    SueStateMachine(
        std::shared_ptr<Character> character
    );

    Move update(const GameState& game) override;

    ~SueStateMachine();
};


// ============================================================
// CONTROLLER
// ============================================================

class SueController : public Controller {
public:
    SueController(
        std::shared_ptr<Character> character
    );

    virtual ~SueController();

    Move getMove(
        const GameState& game
    ) override;

private:
    std::shared_ptr<SueStateMachine> fsm;
};