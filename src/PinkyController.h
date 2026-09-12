#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include "BTGhostController.h"

#include <memory>


class PinkyInfo
{
private:

    static PinkyInfo* info;

    PinkyInfo()
    {
    }

public:

    static PinkyInfo* getInfo()
    {
        if (info == nullptr)
        {
            info = new PinkyInfo();
        }

        return info;
    }

    const GameState* in_gamestate;

    Move out_move;

    std::shared_ptr<Character> in_character;
};


class PinkyController : public Controller
{
private:

    std::shared_ptr<Composite> root;

public:

    PinkyController(
        std::shared_ptr<Character> character
    );

    virtual ~PinkyController();

    virtual Move getMove(
        const GameState& game
    ) override;
};


// ============================================================
// Comportamiento de Pinky
// ============================================================

class PinkyChase : public Behavior
{
public:

    virtual Status update() override;
};
class PinkyScatter : public Behavior
{
    private:
        std::pair<int,int> target;

    public:
        PinkyScatter(){}
        virtual Status update() override;
       
};
class PinkyFrihtend: public Behavior{
    private:
    std::mt19937 e;
    std::uniform_int_distribution<int> uniform_dist;
    public:
        PinkyFrihtend (){}
        virtual Status update() override;
        
};
