#include "SueController.h"

#include <vector>

// ============================================================
// MOVIMIENTOS
// ============================================================

static std::vector<Move> getMoves(
    const GameState& game,
    const std::shared_ptr<Character>& character
)
{
    if (character->getDirection() == PASS)
    {
        return game.getMaze().getPossibleMoves(
            character->getPos()
        );
    }

    return game.getMaze().getGhostLegalMoves(
        character->getPos(),
        character->getDirection()
    );
}

// ============================================================
// IR HACIA UN OBJETIVO
// ============================================================

static Move moveToTarget(
    const GameState& game,
    const std::shared_ptr<Character>& character,
    std::pair<int, int> target
)
{
    auto moves = getMoves(game, character);

    Move bestMove = PASS;
    float bestDistance = 1000000000.0f;

    for (Move move : moves)
    {
        if (move == PASS)
            continue;

        int node =
            game.getMaze().getNeighbour(
                character->getPos(),
                move
            );

        if (node == -1)
            continue;

        auto pos =
            game.getMaze().getNodePos(node);

        float distance =
            euclid2(target, pos);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestMove = move;
        }
    }

    return bestMove;
}

// ============================================================
// ALEJARSE
// ============================================================

static Move moveAway(
    const GameState& game,
    const std::shared_ptr<Character>& character,
    std::pair<int, int> target
)
{
    auto moves = getMoves(game, character);

    Move bestMove = PASS;
    float bestDistance = -1.0f;

    for (Move move : moves)
    {
        if (move == PASS)
            continue;

        int node =
            game.getMaze().getNeighbour(
                character->getPos(),
                move
            );

        if (node == -1)
            continue;

        auto pos =
            game.getMaze().getNodePos(node);

        float distance =
            euclid2(target, pos);

        if (distance > bestDistance)
        {
            bestDistance = distance;
            bestMove = move;
        }
    }

    return bestMove;
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

Move SueWaitState::onUpdate(
    const GameState&
)
{
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

Move SueScatterState::onUpdate(
    const GameState& game
)
{
    // Esquina inferior izquierda
    std::pair<int, int> target(5, 104);

    return moveToTarget(
        game,
        character,
        target
    );
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

Move SueChaseState::onUpdate(
    const GameState& game
)
{
    auto pacman =
        game.getMaze().getNodePos(
            game.getPacmanPos()
        );

    return moveToTarget(
        game,
        character,
        pacman
    );
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

Move SueFleeState::onUpdate(
    const GameState& game
)
{
    auto pacman =
        game.getMaze().getNodePos(
            game.getPacmanPos()
        );

    return moveAway(
        game,
        character,
        pacman
    );
}

// ============================================================
// TRANSICION POR DOTS
// ============================================================

SueDotCountTransition::SueDotCountTransition(
    std::shared_ptr<FSMState> next,
    int dots
)
    : nextState(next),
      dotsRequired(dots),
      initialPills(-1)
{
}

bool SueDotCountTransition::isValid(
    const GameState& game
)
{
    int current =
        static_cast<int>(
            game.getMaze()
                .getPillPositions()
                .size()
        );

    if (initialPills == -1)
    {
        initialPills = current;
        return false;
    }

    int eaten =
        initialPills - current;

    return eaten >= dotsRequired;
}

std::shared_ptr<FSMState>
SueDotCountTransition::getNextState()
{
    return nextState;
}

// ============================================================
// TRANSICION POR TIEMPO
// ============================================================

SueTimeTransition::SueTimeTransition(
    std::shared_ptr<FSMState> next,
    float seconds
)
    : nextState(next),
      seconds(seconds),
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

    float elapsed =
        std::chrono::duration<float>(
            std::chrono::steady_clock::now()
            - startTime
        ).count();

    return elapsed >= seconds;
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

SueVulnerableTransition::SueVulnerableTransition(
    std::shared_ptr<FSMState> next
)
    : nextState(next)
{
}

bool SueVulnerableTransition::isValid(
    const GameState& game
)
{
    // Sue es el fantasma 3
    return game.isGhostEdible(3);
}

std::shared_ptr<FSMState>
SueVulnerableTransition::getNextState()
{
    return nextState;
}

// ============================================================
// RECUPERAR
// ============================================================

SueRecoverTransition::SueRecoverTransition(
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
// FSM DE SUE
// ============================================================

SueStateMachine::SueStateMachine(
    std::shared_ptr<Character> character
)
    : FiniteStateMachine(character)
{
    auto wait =
        std::make_shared<SueWaitState>(character);

    auto scatter =
        std::make_shared<SueScatterState>(character);

    auto chase =
        std::make_shared<SueChaseState>(character);

    auto fleeScatter =
        std::make_shared<SueFleeState>(character);

    auto fleeChase =
        std::make_shared<SueFleeState>(character);

    initialState = wait;
    activeState = wait;

    states.push_back(wait);
    states.push_back(scatter);
    states.push_back(chase);
    states.push_back(fleeScatter);
    states.push_back(fleeChase);

    // WAIT -> SCATTER
    wait->addTransition(
        std::make_shared<SueDotCountTransition>(
            scatter,
            80
        )
    );

    // SCATTER -> FLEE
    scatter->addTransition(
        std::make_shared<SueVulnerableTransition>(
            fleeScatter
        )
    );

    // SCATTER -> CHASE
    scatter->addTransition(
        std::make_shared<SueTimeTransition>(
            chase,
            7.0f
        )
    );

    // CHASE -> FLEE
    chase->addTransition(
        std::make_shared<SueVulnerableTransition>(
            fleeChase
        )
    );

    // CHASE -> SCATTER
    chase->addTransition(
        std::make_shared<SueTimeTransition>(
            scatter,
            20.0f
        )
    );

    // FLEE -> SCATTER
    fleeScatter->addTransition(
        std::make_shared<SueRecoverTransition>(
            scatter
        )
    );

    // FLEE -> CHASE
    fleeChase->addTransition(
        std::make_shared<SueRecoverTransition>(
            chase
        )
    );
}

// ============================================================
// UPDATE
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

SueStateMachine::~SueStateMachine()
{
}