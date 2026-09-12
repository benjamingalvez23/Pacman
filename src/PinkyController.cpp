#include "PinkyController.h"

#include <vector>
#include <limits>



PinkyInfo* PinkyInfo::info = nullptr;



PinkyController::PinkyController(
    std::shared_ptr<Character> character
)
    : Controller(character),
      root(std::make_shared<Selector>())
{
    auto filter = std::make_shared<Filter>();
	filter->addCondition(std::make_shared<Powerpill>());
	filter->addAction(std::make_shared<PinkyFrihtend>()); 		
	root->addChild(filter);

    auto time = std::make_shared<Filter>();
    time->addCondition(std::make_shared<TimeOut>());
	time->addAction(std::make_shared<PinkyScatter>()); 		
	root->addChild(time);
    
    root->addChild(std::make_shared<PinkyChase>());
}

PinkyController::~PinkyController()
{
}


Move PinkyController::getMove(
    const GameState& game
)
{

    PinkyInfo::getInfo()->in_character =
        character;


    PinkyInfo::getInfo()->in_gamestate =
        &game;



    root->tick();


    return PinkyInfo::getInfo()->out_move;
}



Status PinkyChase::update()
{
    auto character = PinkyInfo::getInfo()->in_character;
    auto gs = PinkyInfo::getInfo()->in_gamestate;

    auto pacmanPos = gs->getPacmanPos();

    Move bestMove = PASS;

    float bestDifference = std::numeric_limits<float>::max();

    std::vector<Move> moves;

    if(character->getDirection() == PASS) {
        moves = gs->getMaze().getPossibleMoves(character->getPos());
    } 
    else {
        moves = gs->getMaze().getGhostLegalMoves(
            character->getPos(),
            character->getDirection()
        );
    }

    for(auto move : moves)
    {
        if(move == PASS) {
            break;
        }

        auto nextPos = gs->getMaze().getNeighbour(
            character->getPos(),
            move
        );

        auto nextNode = gs->getMaze().getNodePos(nextPos);
        auto pacmanNode = gs->getMaze().getNodePos(pacmanPos);

        float dist = euclid2(pacmanNode, nextNode);

        float difference = std::abs(dist - 4.0f);

        if(difference < bestDifference)
        {
            bestDifference = difference;
            bestMove = move;
        }
    }

    PinkyInfo::getInfo()->out_move = bestMove;

    return BH_SUCCESS;

}


Status PinkyScatter::update(){
    target=std::make_pair(0,0);
    if(target.first == -1){
		target = PinkyInfo::getInfo()->in_gamestate->getMaze().getPowerPillPositions()[0];
	}
    auto character = PinkyInfo::getInfo()->in_character;
    auto gs = PinkyInfo::getInfo()->in_gamestate;
   

    Move minMove=PASS;
    std::vector<Move> mover;
    if(character->getDirection()==PASS) {
		mover=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		mover = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
    float min=100000000;
	for(auto move:mover) {
		if(move==PASS) {
			break;
		}
		float dist = euclid2(target,gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(),move)));
		if(dist<min) {
			min=dist;
			minMove=move;
		}
	}
	PinkyInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
    
}

Status PinkyFrihtend::update(){
	//std::cerr << " Frightened \n" ;
	auto character = PinkyInfo::getInfo()->in_character;
	auto gs = PinkyInfo::getInfo()->in_gamestate;
	std::vector<Move> mover;
	if(character->getDirection()==PASS) {
		mover=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		mover = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move m = mover[rand()%mover.size()];
	PinkyInfo::getInfo()->out_move = m;
	return BH_SUCCESS; 
}