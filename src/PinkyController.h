#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include "BTGhostController.h" // <--- ESTO FALTABA PARA POWERPILL Y TIMEOUT
#include <memory>
#include <random>

class PinkyInfo {
private:
    static PinkyInfo* info;
    PinkyInfo() {}

public:
    static PinkyInfo* getInfo() {
        if (info == nullptr) {
            info = new PinkyInfo();
        }
        return info;
    }

    const GameState* in_gamestate;
    Move out_move;
    std::shared_ptr<Character> in_character;
};

class PinkyController : public Controller {
private:
    std::shared_ptr<Composite> root;

public:
    PinkyController(std::shared_ptr<Character> character);
    virtual ~PinkyController();
    virtual Move getMove(const GameState& game) override;
};

class PinkyChase : public Behavior {
public:
    virtual Status update() override;
};

class PinkyScatter : public Behavior {
private:
    std::pair<int, int> target;

public:
    PinkyScatter() : target(-1, -1) {}
    virtual Status update() override;
};

class PinkyFrihtend : public Behavior {
private:
    std::mt19937 e;
    std::uniform_int_distribution<int> uniform_dist;

public:
    PinkyFrihtend() : e(rand()), uniform_dist(0, 3) {}
    virtual Status update() override;
};