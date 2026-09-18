#include "PacmanController.h"

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

    return game.getMaze().getPossibleMoves(
        character->getPos()
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

        int node = game.getMaze().getNeighbour(
            character->getPos(),
            move
        );

        if (node == -1)
            continue;

        auto pos = game.getMaze().getNodePos(node);

        float distance = euclid2(target, pos);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestMove = move;
        }
    }

    return bestMove;
}

// ============================================================
// ALEJARSE DE UN OBJETIVO
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

        int node = game.getMaze().getNeighbour(
            character->getPos(),
            move
        );

        if (node == -1)
            continue;

        auto pos = game.getMaze().getNodePos(node);

        float distance = euclid2(target, pos);

        if (distance > bestDistance)
        {
            bestDistance = distance;
            bestMove = move;
        }
    }

    return bestMove;
}

// ============================================================
// INFORMACION
// ============================================================

PacmanInfo* PacmanInfo::info = nullptr;

// ============================================================
// CONTROLLER
// ============================================================

PacmanBTController::PacmanBTController(
    std::shared_ptr<Character> character
)
    : Controller(character),
      root(std::make_shared<Selector>())
{
    // 1. Fantasma comestible -> perseguirlo
    auto chaseGhost = std::make_shared<Filter>();

    chaseGhost->addCondition(
        std::make_shared<PacmanGhostEdibleCondition>()
    );

    chaseGhost->addAction(
        std::make_shared<PacmanChaseEdibleGhost>()
    );

    root->addChild(chaseGhost);

    // 2. Fantasma peligroso cerca -> escapar
    auto flee = std::make_shared<Filter>();

    flee->addCondition(
        std::make_shared<PacmanDangerCondition>()
    );

    flee->addAction(
        std::make_shared<PacmanFlee>()
    );

    root->addChild(flee);

    // 3. Si no ocurre nada -> buscar pill
    root->addChild(
        std::make_shared<PacmanSeekPill>()
    );
}

PacmanBTController::~PacmanBTController()
{
}

Move PacmanBTController::getMove(
    const GameState& game
)
{
    PacmanInfo::getInfo()->in_character = character;
    PacmanInfo::getInfo()->in_gamestate = &game;

    root->tick();

    return PacmanInfo::getInfo()->out_move;
}

// ============================================================
// HAY UN FANTASMA COMESTIBLE
// ============================================================

Status PacmanGhostEdibleCondition::update()
{
    auto game =
        PacmanInfo::getInfo()->in_gamestate;

    for (int i = 0; i < 4; i++)
    {
        if (game->isGhostEdible(i))
            return BH_SUCCESS;
    }

    return BH_FAILURE;
}

// ============================================================
// HAY PELIGRO CERCA
// ============================================================

PacmanDangerCondition::PacmanDangerCondition(
    float radius
)
    : dangerRadius(radius)
{
}

Status PacmanDangerCondition::update()
{
    auto info = PacmanInfo::getInfo();

    auto game = info->in_gamestate;

    auto myPos =
        game->getMaze().getNodePos(
            info->in_character->getPos()
        );

    for (int i = 0; i < 4; i++)
    {
        if (game->isGhostEdible(i))
            continue;

        auto ghostPos =
            game->getMaze().getNodePos(
                game->getGhostsPos(i)
            );

        if (euclid2(myPos, ghostPos) <= dangerRadius)
            return BH_SUCCESS;
    }

    return BH_FAILURE;
}

// ============================================================
// PERSEGUIR FANTASMA COMESTIBLE
// ============================================================

Status PacmanChaseEdibleGhost::update()
{
    auto info = PacmanInfo::getInfo();

    auto game = info->in_gamestate;

    auto myPos =
        game->getMaze().getNodePos(
            info->in_character->getPos()
        );

    float bestDistance = 1000000000.0f;
    std::pair<int, int> target;

    bool found = false;

    for (int i = 0; i < 4; i++)
    {
        if (!game->isGhostEdible(i))
            continue;

        auto ghostPos =
            game->getMaze().getNodePos(
                game->getGhostsPos(i)
            );

        float distance =
            euclid2(myPos, ghostPos);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            target = ghostPos;
            found = true;
        }
    }

    if (!found)
    {
        info->out_move = PASS;
        return BH_SUCCESS;
    }

    info->out_move =
        moveToTarget(
            *game,
            info->in_character,
            target
        );

    return BH_SUCCESS;
}

// ============================================================
// HUIR
// ============================================================

Status PacmanFlee::update()
{
    auto info = PacmanInfo::getInfo();

    auto game = info->in_gamestate;

    auto myPos =
        game->getMaze().getNodePos(
            info->in_character->getPos()
        );

    float closestDistance = 1000000000.0f;

    std::pair<int, int> danger;
    bool found = false;

    for (int i = 0; i < 4; i++)
    {
        if (game->isGhostEdible(i))
            continue;

        auto ghostPos =
            game->getMaze().getNodePos(
                game->getGhostsPos(i)
            );

        float distance =
            euclid2(myPos, ghostPos);

        if (distance < closestDistance)
        {
            closestDistance = distance;
            danger = ghostPos;
            found = true;
        }
    }

    if (!found)
    {
        info->out_move = PASS;
        return BH_SUCCESS;
    }

    info->out_move =
        moveAway(
            *game,
            info->in_character,
            danger
        );

    return BH_SUCCESS;
}

// ============================================================
// BUSCAR PILL
// ============================================================

Status PacmanSeekPill::update()
{
    auto info = PacmanInfo::getInfo();

    auto game = info->in_gamestate;

    auto myPos =
        game->getMaze().getNodePos(
            info->in_character->getPos()
        );

    auto pills =
        game->getMaze().getPillPositions();

    auto powerPills =
        game->getMaze().getPowerPillPositions();

    // Agregar power pills a la misma lista
    pills.insert(
        pills.end(),
        powerPills.begin(),
        powerPills.end()
    );

    if (pills.empty())
    {
        auto moves =
            getMoves(
                *game,
                info->in_character
            );

        info->out_move =
            moves.empty()
            ? PASS
            : moves[0];

        return BH_SUCCESS;
    }

    float bestDistance = 1000000000.0f;

    std::pair<int, int> target = pills[0];

    for (auto pill : pills)
    {
        float distance =
            euclid2(myPos, pill);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            target = pill;
        }
    }

    info->out_move =
        moveToTarget(
            *game,
            info->in_character,
            target
        );

    return BH_SUCCESS;
}