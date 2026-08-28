#ifndef PACMANDTCONTROLLER_H_
#define PACMANDTCONTROLLER_H_

#include "Controller.h"
#include <memory>

class PacmanDTController: public Controller {
public:
    PacmanDTController(std::shared_ptr<Character> character);
    virtual ~PacmanDTController();

    Move getMove(const GameState& gs) override;
};

#endif 