#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <memory>

class InkyInfo {
private:
    static InkyInfo* info;
    InkyInfo() {}

public:
    static InkyInfo* getInfo() {
        if (info == nullptr) {
            info = new InkyInfo();
        }
        return info;
    }

    const GameState* in_gamestate;
    Move out_move;
    std::shared_ptr<Character> in_character;
};

class InkyController : public Controller {
private:
    std::shared_ptr<Composite> root;

public:
    InkyController(std::shared_ptr<Character> character);
    virtual ~InkyController();
    virtual Move getMove(const GameState& game) override;
};

class InkyIntercept : public Behavior {
public:
    virtual Status update() override;
};

class InkyChase : public Behavior {
public:
    virtual Status update() override;
};

class InkyFrightened : public Behavior {
public:
    virtual Status update() override;
};

class InkyConditionEdible : public Behavior {
public:
    virtual Status update() override;
};

class InkyConditionNotEdible : public Behavior {
public:
    virtual Status update() override;
};