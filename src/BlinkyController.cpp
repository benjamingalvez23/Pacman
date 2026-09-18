
#include "BlinkyController.h"

#include <iostream>
#include <vector>
#include <cstdlib>

// ============================================================
// CONTROLLER
// ============================================================

BlinkyController::BlinkyController(
    std::shared_ptr<Character> character
)
    : Controller(character),
      fsm(std::make_shared<BlinkyStateMachine>(character))
{
}

BlinkyController::~BlinkyController()
{
}

Move BlinkyController::getMove(
    const GameState& game
)
{
    return fsm->update(game);
}


// ============================================================
// FUNCION AUXILIAR
// ============================================================

static std::vector<Move> getBlinkyMoves(
    const GameState& game,
    const std::shared_ptr<Character>& character
)
{
    const auto myPos = character->getPos();

    if (character->getDirection() == PASS)
    {
        return game.getMaze().getPossibleMoves(myPos);
    }

    return game.getMaze().getGhostLegalMoves(
        myPos,
        character->getDirection()
    );
}


// ============================================================
// BLINKY SCATTER
// ============================================================

BlinkyScatterState::BlinkyScatterState(
    std::shared_ptr<Character> character
)
    : FSMState(character)
{
}

void BlinkyScatterState::onEnter(
    const GameState&
)
{
    std::cout
        << "Blinky FSM -> SCATTER"
        << std::endl;

    auto ghost =
        std::dynamic_pointer_cast<Ghost>(character);

    if (ghost)
    {
        ghost->revert();
    }
}

Move BlinkyScatterState::onUpdate(
    const GameState& game
)
{
    std::vector<Move> moves =
        getBlinkyMoves(game, character);

    if (moves.empty())
    {
        return PASS;
    }

    // Esquina de Blinky:
    // esquina superior derecha del laberinto.
    //
    // Esta es la misma coordenada utilizada
    // por la implementación anterior:
    // (104, 1)
    const std::pair<int, int> target =
        std::make_pair(104, 1);

    const auto myPos =
        character->getPos();

    float bestDistance =
        euclid2(
            game.getMaze().getNodePos(
                game.getMaze().getNeighbour(
                    myPos,
                    moves[0]
                )
            ),
            target
        );

    std::size_t bestIndex = 0;

    for (std::size_t i = 1;
         i < moves.size();
         ++i)
    {
        float distance =
            euclid2(
                game.getMaze().getNodePos(
                    game.getMaze().getNeighbour(
                        myPos,
                        moves[i]
                    )
                ),
                target
            );

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return moves[bestIndex];
}


// ============================================================
// BLINKY CHASE
// ============================================================

BlinkyChaseState::BlinkyChaseState(
    std::shared_ptr<Character> character
)
    : FSMState(character)
{
}

void BlinkyChaseState::onEnter(
    const GameState&
)
{
    std::cout
        << "Blinky FSM -> CHASE"
        << std::endl;

    auto ghost =
        std::dynamic_pointer_cast<Ghost>(character);

    if (ghost)
    {
        ghost->revert();
    }
}

Move BlinkyChaseState::onUpdate(
    const GameState& game
)
{
    std::vector<Move> moves =
        getBlinkyMoves(game, character);

    if (moves.empty())
    {
        return PASS;
    }

    const auto myPos =
        character->getPos();

    const auto pacmanPos =
        game.getMaze().getNodePos(
            game.getPacmanPos()
        );

    // Blinky intenta minimizar la distancia
    // a Pac-Man.
    float bestDistance =
        euclid2(
            game.getMaze().getNodePos(
                game.getMaze().getNeighbour(
                    myPos,
                    moves[0]
                )
            ),
            pacmanPos
        );

    std::size_t bestIndex = 0;

    for (std::size_t i = 1;
         i < moves.size();
         ++i)
    {
        float distance =
            euclid2(
                game.getMaze().getNodePos(
                    game.getMaze().getNeighbour(
                        myPos,
                        moves[i]
                    )
                ),
                pacmanPos
            );

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return moves[bestIndex];
}


// ============================================================
// BLINKY ELROY
// ============================================================

BlinkyElroyState::BlinkyElroyState(
    std::shared_ptr<Character> character,
    float multiplier
)
    : FSMState(character),
      speedMultiplier(multiplier)
{
}

void BlinkyElroyState::onEnter(
    const GameState&
)
{
    std::cout
        << "Blinky FSM -> ELROY x"
        << speedMultiplier
        << std::endl;

    auto ghost =
        std::dynamic_pointer_cast<Ghost>(character);

    if (ghost)
    {
        ghost->revert();
    }
}

void BlinkyElroyState::onExit(
    const GameState&
)
{
    auto ghost =
        std::dynamic_pointer_cast<Ghost>(character);

    if (ghost)
    {
        ghost->revert();
    }
}

Move BlinkyElroyState::onUpdate(
    const GameState& game
)
{
    // Elroy mantiene el comportamiento agresivo
    // de persecución de Blinky.
    //
    // El multiplicador se conserva como parte del
    // estado FSM. La clase Character/Ghost del
    // proyecto no expone en el código conocido una
    // función pública para cambiar la velocidad.

    std::vector<Move> moves =
        getBlinkyMoves(game, character);

    if (moves.empty())
    {
        return PASS;
    }

    const auto myPos =
        character->getPos();

    const auto pacmanPos =
        game.getMaze().getNodePos(
            game.getPacmanPos()
        );

    float bestDistance =
        euclid2(
            game.getMaze().getNodePos(
                game.getMaze().getNeighbour(
                    myPos,
                    moves[0]
                )
            ),
            pacmanPos
        );

    std::size_t bestIndex = 0;

    for (std::size_t i = 1;
         i < moves.size();
         ++i)
    {
        float distance =
            euclid2(
                game.getMaze().getNodePos(
                    game.getMaze().getNeighbour(
                        myPos,
                        moves[i]
                    )
                ),
                pacmanPos
            );

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return moves[bestIndex];
}


// ============================================================
// BLINKY FLEE
// ============================================================

BlinkyFleeState::BlinkyFleeState(
    std::shared_ptr<Character> character
)
    : FSMState(character)
{
}

void BlinkyFleeState::onEnter(
    const GameState&
)
{
    std::cout
        << "Blinky FSM -> FLEE"
        << std::endl;
}

Move BlinkyFleeState::onUpdate(
    const GameState& game
)
{
    std::vector<Move> moves =
        getBlinkyMoves(game, character);

    if (moves.empty())
    {
        return PASS;
    }

    const auto myPos =
        character->getPos();

    const auto pacmanPos =
        game.getMaze().getNodePos(
            game.getPacmanPos()
        );

    // Flee busca maximizar la distancia
    // respecto de Pac-Man.
    float bestDistance =
        euclid2(
            game.getMaze().getNodePos(
                game.getMaze().getNeighbour(
                    myPos,
                    moves[0]
                )
            ),
            pacmanPos
        );

    std::size_t bestIndex = 0;

    for (std::size_t i = 1;
         i < moves.size();
         ++i)
    {
        float distance =
            euclid2(
                game.getMaze().getNodePos(
                    game.getMaze().getNeighbour(
                        myPos,
                        moves[i]
                    )
                ),
                pacmanPos
            );

        if (distance > bestDistance)
        {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return moves[bestIndex];
}


// ============================================================
// TIME TRANSITION
// ============================================================

BlinkyTimeTransition::BlinkyTimeTransition(
    std::shared_ptr<FSMState> next,
    float seconds
)
    : nextState(next),
      duration(seconds),
      started(false)
{
}

bool BlinkyTimeTransition::isValid(
    const GameState&
)
{
    if (!started)
    {
        startTime =
            std::chrono::steady_clock::now();

        started = true;

        return false;
    }

    const auto now =
        std::chrono::steady_clock::now();

    const std::chrono::duration<float> elapsed =
        now - startTime;

    return elapsed.count() >= duration;
}

std::shared_ptr<FSMState>
BlinkyTimeTransition::getNextState()
{
    return nextState;
}

void BlinkyTimeTransition::onTransition(
    const GameState&
)
{
    started = false;
}


// ============================================================
// PILL TRANSITION
// ============================================================

BlinkyPillTransition::BlinkyPillTransition(
    std::shared_ptr<FSMState> next,
    int thresholdValue
)
    : nextState(next),
      threshold(thresholdValue),
      activated(false),
      previousPills(-1)
{
}

bool BlinkyPillTransition::isValid(
    const GameState& game
)
{
    const auto& pills =
        game.getMaze().getPillPositions();

    const int remainingPills =
        static_cast<int>(pills.size());

    // Primera lectura.
    if (previousPills < 0)
    {
        previousPills = remainingPills;
    }

    // Entra en Elroy cuando el número de
    // píldoras llega al umbral.
    if (!activated &&
        remainingPills <= threshold &&
        previousPills > threshold)
    {
        activated = true;
        previousPills = remainingPills;

        return true;
    }

    previousPills = remainingPills;

    return false;
}

std::shared_ptr<FSMState>
BlinkyPillTransition::getNextState()
{
    return nextState;
}


// ============================================================
// VULNERABLE TRANSITION
// ============================================================

BlinkyVulnerableTransition::
BlinkyVulnerableTransition(
    std::shared_ptr<FSMState> next
)
    : nextState(next)
{
}

bool BlinkyVulnerableTransition::isValid(
    const GameState& game
)
{
    // Blinky = ghost 0.
    return game.isGhostEdible(0);
}

std::shared_ptr<FSMState>
BlinkyVulnerableTransition::getNextState()
{
    return nextState;
}


// ============================================================
// RECOVER TRANSITION
// ============================================================

BlinkyRecoverTransition::
BlinkyRecoverTransition(
    std::shared_ptr<FSMState> next
)
    : nextState(next)
{
}

bool BlinkyRecoverTransition::isValid(
    const GameState& game
)
{
    // Cuando deja de ser edible.
    return !game.isGhostEdible(0);
}

std::shared_ptr<FSMState>
BlinkyRecoverTransition::getNextState()
{
    return nextState;
}


// ============================================================
// BLINKY STATE MACHINE
// ============================================================

BlinkyStateMachine::BlinkyStateMachine(
    std::shared_ptr<Character> character
)
    : FiniteStateMachine(character)
{
    // --------------------------------------------------------
    // CREAR ESTADOS
    // --------------------------------------------------------

    auto scatter =
        std::make_shared<BlinkyScatterState>(
            character
        );

    auto chase =
        std::make_shared<BlinkyChaseState>(
            character
        );

    auto elroy =
        std::make_shared<BlinkyElroyState>(
            character,
            1.25f
        );

    auto fleeFromScatter =
        std::make_shared<BlinkyFleeState>(
            character
        );

    auto fleeFromChase =
        std::make_shared<BlinkyFleeState>(
            character
        );

    auto fleeFromElroy =
        std::make_shared<BlinkyFleeState>(
            character
        );


    // --------------------------------------------------------
    // ESTADO INICIAL
    // --------------------------------------------------------

    initialState = scatter;
    activeState = initialState;


    // --------------------------------------------------------
    // GUARDAR ESTADOS
    // --------------------------------------------------------

    states.push_back(scatter);
    states.push_back(chase);
    states.push_back(elroy);
    states.push_back(fleeFromScatter);
    states.push_back(fleeFromChase);
    states.push_back(fleeFromElroy);


    // ========================================================
    // SCATTER
    // ========================================================

    // Primero vulnerabilidad para darle prioridad.
    scatter->addTransition(
        std::make_shared<BlinkyVulnerableTransition>(
            fleeFromScatter
        )
    );

    // Después de 7 segundos -> Chase.
    scatter->addTransition(
        std::make_shared<BlinkyTimeTransition>(
            chase,
            7.0f
        )
    );


    // ========================================================
    // CHASE
    // ========================================================

    // Vulnerabilidad tiene prioridad.
    chase->addTransition(
        std::make_shared<BlinkyVulnerableTransition>(
            fleeFromChase
        )
    );

    // Cuando quedan 20 pills -> Elroy.
    chase->addTransition(
        std::make_shared<BlinkyPillTransition>(
            elroy,
            20
        )
    );

    // Después de 20 segundos -> Scatter.
    chase->addTransition(
        std::make_shared<BlinkyTimeTransition>(
            scatter,
            20.0f
        )
    );


    // ========================================================
    // ELROY
    // ========================================================

    elroy->addTransition(
        std::make_shared<BlinkyVulnerableTransition>(
            fleeFromElroy
        )
    );


    // ========================================================
    // FLEE DESDE SCATTER
    // ========================================================

    fleeFromScatter->addTransition(
        std::make_shared<BlinkyRecoverTransition>(
            scatter
        )
    );


    // ========================================================
    // FLEE DESDE CHASE
    // ========================================================

    fleeFromChase->addTransition(
        std::make_shared<BlinkyRecoverTransition>(
            chase
        )
    );


    // ========================================================
    // FLEE DESDE ELROY
    // ========================================================

    fleeFromElroy->addTransition(
        std::make_shared<BlinkyRecoverTransition>(
            elroy
        )
    );
}


// ============================================================
// UPDATE FSM
// ============================================================

Move BlinkyStateMachine::update(
    const GameState& game
)
{
    auto transition =
        activeState->getActiveTransition(game);

    if (transition != nullptr)
    {
        activeState->onExit(game);

        transition->onTransition(game);

        activeState =
            transition->getNextState();

        activeState->onEnter(game);
    }

    return activeState->onUpdate(game);
}


// ============================================================
// DESTRUCTOR
// ============================================================

BlinkyStateMachine::~BlinkyStateMachine()
{
}