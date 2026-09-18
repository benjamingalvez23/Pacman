#pragma once
 
#include "Controller.h"
#include "BehaviorTree.h"
#include <memory>
 
class PinkyInfo {
public:
    std::shared_ptr<Character> in_character;
    const GameState* in_gamestate;
    Move out_move = PASS;
 
    static PinkyInfo* getInfo() {
        if (info == nullptr) info = new PinkyInfo();
        return info;
    }
 
private:
    static PinkyInfo* info;
};
 
// ==================== BEHAVIOR TREE BEHAVIORS ====================
 
class PinkyOptimalRange : public Behavior {
public:
    virtual Status update() override;
};
 
class PinkyChase : public Behavior {
public:
    virtual Status update() override;
};
 
class PinkyScatter : public Behavior {
public:
    PinkyScatter();
    virtual Status update() override;
 
private:
    std::pair<int, int> target;
};
 
class PinkyFrightened : public Behavior {
public:
    virtual Status update() override;
};
 
class PinkyPowerPillHunt : public Behavior {
public:
    virtual Status update() override;
};
 
// ==================== CONDITIONS ====================
 
class PinkyEdibleCondition : public Behavior {
public:
    virtual Status update() override;
};
 
class PinkyLowPillsCondition : public Behavior {
public:
    virtual Status update() override;
};
 
// ==================== PINKY CONTROLLER ====================
 
class PinkyController : public Controller {
public:
    PinkyController(std::shared_ptr<Character> character);
    virtual ~PinkyController();
    virtual Move getMove(const GameState& game) override;
 
private:
    std::shared_ptr<Selector> root;
};