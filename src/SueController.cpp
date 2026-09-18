#include "SueController.h"

#include <iostream>
#include <vector>

// ============================================================
// FUNCION AUXILIAR
// ============================================================

static std::vector<Move> getSueMoves(
    const GameState& game,
    const std::shared_ptr<Character>& character
)
{
    const auto myPos =
        character->getPos();

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
// CONTROLLER
// ============================================================

SueController::SueController(
    std::shared_ptr<Character> character
)
    : Controller(character),
      fsm(std::make_shared<SueStateMachine>(character))
{
}

SueController::~SueController()
{
}

Move SueController::getMove(
    const GameState& game
)
{
    return fsm->update(game);
}


// ============================================================
// WAIT
// ============================================================

SueWaitState::SueWaitState(
    std::shared_ptr<Character> character
)
    : FSMState(character)
{
}

void SueWaitState::onEnter(
    const GameState&
)
{
    std::cout
        << "Sue FSM -> WAIT"
        << std::endl;
}

Move SueWaitState::onUpdate(
    const GameState&
)
{
    // Sue permanece dentro de la casa.
    return PASS;
}


// ============================================================
// SCATTER
// ============================================================

SueScatterState::SueScatterState(
    std::shared_ptr<Character> character
)
    : FSMState(character)
{
}

void SueScatterState::onEnter(
    const GameState&
)
{
    std::cout
        << "Sue FSM -> SCATTER"
        << std::endl;

    auto ghost =
        std::dynamic_pointer_cast<Ghost>(character);

    if (ghost)
    {
        ghost->revert();
    }
}

Move SueScatterState::onUpdate(
    const GameState& game
)
{
    std::vector<Move> moves =
        getSueMoves(game, character);

    if (moves.empty())
    {
        return PASS;
    }

    // Esquina de Clyde/Sue.
    const std::pair<int, int> target =
        std::make_pair(5, 104);

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
// CHASE
// ============================================================

SueChaseState::SueChaseState(
    std::shared_ptr<Character> character
)
    : FSMState(character)
{
}

void SueChaseState::onEnter(
    const GameState&
)
{
    std::cout
        << "Sue FSM -> CHASE"
        << std::endl;

    auto ghost =
        std::dynamic_pointer_cast<Ghost>(character);

    if (ghost)
    {
        ghost->revert();
    }
}

Move SueChaseState::onUpdate(
    const GameState& game
)
{
    std::vector<Move> moves =
        getSueMoves(game, character);

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
// FLEE
// ============================================================

SueFleeState::SueFleeState(
    std::shared_ptr<Character> character
)
    : FSMState(character)
{
}

void SueFleeState::onEnter(
    const GameState&
)
{
    std::cout
        << "Sue FSM -> FLEE"
        << std::endl;
}

Move SueFleeState::onUpdate(
    const GameState& game
)
{
    std::vector<Move> moves =
        getSueMoves(game, character);

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

    // Alejarse lo máximo posible
    // de Pac-Man.
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
// DOT COUNT
// ============================================================

SueDotCountTransition::SueDotCountTransition(
    std::shared_ptr<FSMState> next,
    int dots
)
    : nextState(next),
      dotsRequired(dots),
      initialPills(-1),
      initialized(false)
{
}

bool SueDotCountTransition::isValid(
    const GameState& game
)
{
    const auto& pills =
        game.getMaze().getPillPositions();

    const int remainingPills =
        static_cast<int>(pills.size());

    if (!initialized)
    {
        initialPills = remainingPills;
        initialized = true;
        return false;
    }

    const int dotsEaten =
        initialPills - remainingPills;

    return dotsEaten >= dotsRequired;
}

std::shared_ptr<FSMState>
SueDotCountTransition::getNextState()
{
    return nextState;
}


// ============================================================
// TIME
// ============================================================

SueTimeTransition::SueTimeTransition(
    std::shared_ptr<FSMState> next,
    float seconds
)
    : nextState(next),
      duration(seconds),
      started(false)
{
}

bool SueTimeTransition::isValid(
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
SueTimeTransition::getNextState()
{
    return nextState;
}

void SueTimeTransition::onTransition(
    const GameState&
)
{
    started = false;
}


// ============================================================
// VULNERABLE
// ============================================================

SueVulnerableTransition::
SueVulnerableTransition(
    std::shared_ptr<FSMState> next
)
    : nextState(next)
{
}

bool SueVulnerableTransition::isValid(
    const GameState& game
)
{
    // Sue/Clyde = Ghost 3.
    return game.isGhostEdible(3);
}

std::shared_ptr<FSMState>
SueVulnerableTransition::getNextState()
{
    return nextState;
}


// ============================================================
// RECOVER
// ============================================================

SueRecoverTransition::
SueRecoverTransition(
    std::shared_ptr<FSMState> next
)
    : nextState(next)
{
}

bool SueRecoverTransition::isValid(
    const GameState& game
)
{
    return !game.isGhostEdible(3);
}

std::shared_ptr<FSMState>
SueRecoverTransition::getNextState()
{
    return nextState;
}


// ============================================================
// SUE STATE MACHINE
// ============================================================

SueStateMachine::SueStateMachine(
    std::shared_ptr<Character> character
)
    : FiniteStateMachine(character)
{
    // --------------------------------------------------------
    // CREAR ESTADOS
    // --------------------------------------------------------

    auto wait =
        std::make_shared<SueWaitState>(
            character
        );

    auto scatter =
        std::make_shared<SueScatterState>(
            character
        );

    auto chase =
        std::make_shared<SueChaseState>(
            character
        );

    auto fleeFromScatter =
        std::make_shared<SueFleeState>(
            character
        );

    auto fleeFromChase =
        std::make_shared<SueFleeState>(
            character
        );


    // --------------------------------------------------------
    // ESTADO INICIAL
    // --------------------------------------------------------

    initialState = wait;
    activeState = initialState;


    // --------------------------------------------------------
    // REGISTRAR ESTADOS
    // --------------------------------------------------------

    states.push_back(wait);
    states.push_back(scatter);
    states.push_back(chase);
    states.push_back(fleeFromScatter);
    states.push_back(fleeFromChase);


    // ========================================================
    // WAIT -> SCATTER
    // ========================================================

    wait->addTransition(
        std::make_shared<SueDotCountTransition>(
            scatter,
            80
        )
    );


    // ========================================================
    // SCATTER -> FLEE
    // ========================================================

    // Vulnerabilidad primero para tener prioridad.
    scatter->addTransition(
        std::make_shared<SueVulnerableTransition>(
            fleeFromScatter
        )
    );


    // ========================================================
    // SCATTER -> CHASE
    // ========================================================

    scatter->addTransition(
        std::make_shared<SueTimeTransition>(
            chase,
            7.0f
        )
    );


    // ========================================================
    // CHASE -> FLEE
    // ========================================================

    chase->addTransition(
        std::make_shared<SueVulnerableTransition>(
            fleeFromChase
        )
    );


    // ========================================================
    // CHASE -> SCATTER
    // ========================================================

    chase->addTransition(
        std::make_shared<SueTimeTransition>(
            scatter,
            20.0f
        )
    );


    // ========================================================
    // FLEE DESDE SCATTER -> SCATTER
    // ========================================================

    fleeFromScatter->addTransition(
        std::make_shared<SueRecoverTransition>(
            scatter
        )
    );


    // ========================================================
    // FLEE DESDE CHASE -> CHASE
    // ========================================================

    fleeFromChase->addTransition(
        std::make_shared<SueRecoverTransition>(
            chase
        )
    );
}


// ============================================================
// FSM UPDATE
// ============================================================

Move SueStateMachine::update(
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

SueStateMachine::~SueStateMachine()
{
}
