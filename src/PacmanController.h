#pragma once
 
#include "Controller.h"
#include "BehaviorTree.h"
#include <memory>
 
class PacmanInfo {
private:
    static PacmanInfo* info;
    PacmanInfo() {}
 
public:
    static PacmanInfo* getInfo() {
        if (info == nullptr) {
            info = new PacmanInfo();
        }
        return info;
    }
 
    const GameState* in_gamestate;
    std::shared_ptr<Character> in_character;
    Move out_move = PASS;
};
 
class PacmanBTController : public Controller {
private:
    std::shared_ptr<Selector> root;
 
public:
    PacmanBTController(std::shared_ptr<Character> character);
    virtual ~PacmanBTController();
    virtual Move getMove(const GameState& game) override;
};
 
// ==================== CONDICIONES ====================
 
// Éxito si hay al menos un fantasma comestible en el mapa.
class PacmanGhostEdibleCondition : public Behavior {
public:
    virtual Status update() override;
};
 
// Éxito si hay un fantasma NO comestible demasiado cerca de Pac-Man.
class PacmanDangerCondition : public Behavior {
private:
    float dangerRadius; // en unidades de distancia al cuadrado
public:
    PacmanDangerCondition(float radius = 36.0f); // radio ~6 casillas
    virtual Status update() override;
};
 
// ==================== ACCIONES ====================
 
// Persigue al fantasma comestible más cercano.
class PacmanChaseEdibleGhost : public Behavior {
public:
    virtual Status update() override;
};
 
// Huye del fantasma peligroso más cercano (maximiza distancia mínima).
class PacmanFlee : public Behavior {
public:
    virtual Status update() override;
};
 
// Comportamiento por defecto: ir a comer la pill más cercana
// (normal o power pill, la que esté más próxima).
class PacmanSeekPill : public Behavior {
public:
    virtual Status update() override;
};
