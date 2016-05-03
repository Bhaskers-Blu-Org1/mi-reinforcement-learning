/*!
 * \file Gridworld.cpp
 * \brief 
 * \author tkornut
 * \date Mar 21, 2016
 */

#include <types/Gridworld.hpp>

namespace mic {
namespace environments {

Gridworld::Gridworld() : Environment(0, 0, (size_t)GridworldChannels::Count){
}

Gridworld::~Gridworld() {
	// TODO Auto-generated destructor stub
}

mic::environments::Gridworld & Gridworld::operator= (const mic::environments::Gridworld & gw_) {
	width = gw_.width;
	height = gw_.height;
	channels = gw_.channels;
	initial_position = gw_.initial_position;
	environment_grid = gw_.environment_grid;

	return *this;
}


// Initialize environment_grid.
void Gridworld::generateGridworld(int environment_grid_type_, size_t width_, size_t height_) {
	switch(environment_grid_type_) {
		case 0 : initExemplaryGrid(); break;
		case 1 : initClassicCliffGrid(); break;
		case 2 : initDiscountGrid(); break;
		case 3 : initBridgeGrid(); break;
		case 4 : initBookGrid(); break;
		case 5 : initMazeGrid(); break;
		case 6 : initExemplaryDQLGrid(); break;
		case 7 : initModifiedDQLGrid(); break;
		case 8 : initDebug2x2Grid(); break;
		case 9 : initDebug3x3Grid(); break;
		case -2: initHardRandomGrid(width_, height_); break;
		case -1:
		default: initSimpleRandomGrid(width_, height_);
	}//: switch
}


void Gridworld::initExemplaryGrid() {
	LOG(LINFO) << "Generating exemplary environment_grid";
	// [[' ',' ',' ',' '],
	//  ['S',-10,' ',' '],
	//  [' ','','#',' '],
	//  [' ',' ',' ',10]]

	// Overwrite dimensions.
	width = 4;
	height = 4;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,1);
	moveAgentToPosition(initial_position);

	// Place wall(s).
	environment_grid({2,2, (size_t)GridworldChannels::Walls}) = 1;

	// Place pit(s).
	environment_grid({1,1, (size_t)GridworldChannels::Pits}) = -10;

	// Place goal(s).
	environment_grid({3,3, (size_t)GridworldChannels::Goals}) = 10;
}


void Gridworld::initClassicCliffGrid() {
	LOG(LINFO) << "Generating classic cliff environment_grid";
	// [[' ',' ',' ',' ',' '],
	//  ['S',' ',' ',' ',10],
	//  [-100,-100, -100, -100, -100]]

	// Overwrite dimensions.
	width = 5;
	height = 3;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,1);
	moveAgentToPosition(initial_position);

	// Place pit(s).
	for(size_t x=0; x<width; x++)
		environment_grid({x,2, (size_t)GridworldChannels::Pits}) = -100;

	// Place goal(s).
	environment_grid({4,1, (size_t)GridworldChannels::Goals}) = 10;
}

void Gridworld::initDiscountGrid() {
	LOG(LINFO) << "Generating classic discount environment_grid";
	// [[' ',' ',' ',' ',' '],
	//  [' ','#',' ',' ',' '],
	//  [' ','#', 1,'#', 10],
	//  ['S',' ',' ',' ',' '],
	//  [-10,-10, -10, -10, -10]]

	// Overwrite dimensions.
	width = 5;
	height = 5;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,3);
	moveAgentToPosition(initial_position);

	// Place pits.
	for(size_t x=0; x<width; x++)
		environment_grid({x,4, (size_t)GridworldChannels::Pits}) = -10;

	// Place wall(s).
	environment_grid({1,1, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({1,2, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({3,2, (size_t)GridworldChannels::Walls}) = 1;

	// Place goal(s).
	environment_grid({2,2, (size_t)GridworldChannels::Goals}) = 1;
	environment_grid({4,2, (size_t)GridworldChannels::Goals}) = 10;
}


void Gridworld::initBridgeGrid() {
	LOG(LINFO) << "Generating classic bridge environment_grid";
	// [[ '#',-100, -100, -100, -100, -100, '#'],
	//  [   1, 'S',  ' ',  ' ',  ' ',  ' ',  10],
	//  [ '#',-100, -100, -100, -100, -100, '#']]

	// Overwrite dimensions.
	width = 7;
	height = 3;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(1,1);
	moveAgentToPosition(initial_position);

	// Place pits.
	for(size_t x=1; x<width-1; x++) {
		environment_grid({x,0, (size_t)GridworldChannels::Pits}) = -100;
		environment_grid({x,2, (size_t)GridworldChannels::Pits}) = -100;
	}//: for

	// Place wall(s).
	environment_grid({0,0, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({0,2, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({6,0, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({6,2, (size_t)GridworldChannels::Walls}) = 1;

	// Place goal(s).
	environment_grid({0,1, (size_t)GridworldChannels::Goals}) = 1;
	environment_grid({6,1, (size_t)GridworldChannels::Goals}) = 10;
}


void Gridworld::initBookGrid() {
	LOG(LINFO) << "Generating classic book environment_grid!!";
	// [[' ',' ',' ',+1],
	//  [' ','#',' ',-1],
	//  ['S',' ',' ',' ']]

	// Overwrite dimensions.
	width = 4;
	height = 3;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,2);
	moveAgentToPosition(initial_position);

	// Place wall(s).
	environment_grid({1,1, (size_t)GridworldChannels::Walls}) = 1;

	// Place pit(s).
	environment_grid({3,1, (size_t)GridworldChannels::Pits}) = -1;

	// Place goal(s).
	environment_grid({3,0, (size_t)GridworldChannels::Goals}) = 1;
}


void Gridworld::initMazeGrid() {
	LOG(LINFO) << "Generating classic maze environment_grid";
	// [[' ',' ',' ',+1],
	//  ['#','#',' ','#'],
	//  [' ','#',' ',' '],
	//  [' ','#','#',' '],
	//  ['S',' ',' ',' ']]

	// Overwrite dimensions.
	width = 4;
	height = 5;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,4);
	moveAgentToPosition(initial_position);

	// Place wall(s).
	environment_grid({0,1, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({1,1, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({1,2, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({1,3, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({2,3, (size_t)GridworldChannels::Walls}) = 1;
	environment_grid({3,1, (size_t)GridworldChannels::Walls}) = 1;

	// Place goal(s).
	environment_grid({3,0, (size_t)GridworldChannels::Goals}) = 1;
}


void Gridworld::initExemplaryDQLGrid() {
	LOG(LINFO) << "Generating environment_grid from Deep Q-Learning example";
	/*
	 * [[' ',' ',' ',' '],
	 *  [' ',' ',+10,' '],
	 *  [' ','#',-10,' '],
	 *  ['S',' ',' ',' ']]
	 */

	// Overwrite dimensions.
	width = 4;
	height = 4;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,3);
	moveAgentToPosition(initial_position);

	// Place wall(s).
	environment_grid({1,2, (size_t)GridworldChannels::Walls}) = 1;

	// Place pit(s).
	environment_grid({2,2, (size_t)GridworldChannels::Pits}) = -10;

	// Place goal(s).
	environment_grid({2,1, (size_t)GridworldChannels::Goals}) = 10;
}

void Gridworld::initModifiedDQLGrid() {
	LOG(LINFO) << "Generating a slightly modified grid from Deep Q-Learning example";
	/*
	 * [[' ',' ',' ',' '],
	 *  [' ','#',+10,' '],
	 *  [' ',' ',-10,' '],
	 *  ['S',' ',' ',' ']]
	 */

	// Overwrite dimensions.
	width = 4;
	height = 4;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,3);
	moveAgentToPosition(initial_position);

	// Place wall(s).
	environment_grid({1,1, (size_t)GridworldChannels::Walls}) = 1;

	// Place pit(s).
	environment_grid({2,2, (size_t)GridworldChannels::Pits}) = -10;

	// Place goal(s).
	environment_grid({2,1, (size_t)GridworldChannels::Goals}) = 10;
}


void Gridworld::initDebug2x2Grid() {
	LOG(LINFO) << "Generating the 2x2 debug grid";
	/*
	 * [['S',-10],
	 *  [+10,' ']]
	 */

	// Overwrite dimensions.
	width = 2;
	height = 2;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(0,0);
	moveAgentToPosition(initial_position);

	// Place pit(s).
	environment_grid({1,0, (size_t)GridworldChannels::Pits}) = -10;

	// Place goal(s).
	environment_grid({0,1, (size_t)GridworldChannels::Goals}) = 10;
}


/*!
 * 	Method initializes the 3x3 grid useful during the debugging.
 *
 */
void Gridworld::initDebug3x3Grid() {
	LOG(LINFO) << "Generating the 3x3 debug grid";
	/*
	 * [[' ',-10,' '],
	 *  [-10,'S',-10],
	 *  [' ',+10,' ']]
	 */

	// Overwrite the dimensions.
	width = 3;
	height = 3;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	initial_position.set(1,1);
	moveAgentToPosition(initial_position);

	// Place wall(s).
	environment_grid({1,2, (size_t)GridworldChannels::Walls}) = 1;

	// Place pit(s).
	environment_grid({0,1, (size_t)GridworldChannels::Pits}) = -10;
	environment_grid({1,0, (size_t)GridworldChannels::Pits}) = -10;
	environment_grid({2,1, (size_t)GridworldChannels::Pits}) = -10;

	// Place goal(s).
	environment_grid({1,2, (size_t)GridworldChannels::Goals}) = 10;

}


void Gridworld::initSimpleRandomGrid(size_t width_, size_t height_) {
	LOG(LINFO) << "Generating simple " << width_ << "x" << height_<< " random grid";
	// Overwrite the dimensions.
	width = width_;
	height = height_;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	mic::types::Position2D player(0, width-1, 0, height-1);
	initial_position = player;
	moveAgentToPosition(initial_position);

	// Initialize random device and generator.
	std::random_device rd;
	std::mt19937_64 rng_mt19937_64(rd());

	// Place wall.
	while (1){
		// Random position.
		mic::types::Position2D wall(0, width-1, 0, height-1);

		// Validate pose.
		if (environment_grid({(size_t)wall.x, (size_t)wall.y, (size_t)GridworldChannels::Agent}) != 0)
			continue;

		// Add wall...
		environment_grid({(size_t)wall.x, (size_t)wall.y, (size_t)GridworldChannels::Walls}) = 1;
		break;
	}

	// Place pit.
	while(1){
		// Random position.
		mic::types::Position2D pit(0, width-1, 0, height-1);

		// Validate pose.
		if (environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Agent}) != 0)
			continue;
		if (environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Walls}) != 0)
			continue;

		// Add pit...
		environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Pits}) = -10;

		break;
	}//: while


	// Place goal.
	while(1) {
		// Random position.
		mic::types::Position2D goal(0, width-1, 0, height-1);

		// Validate pose.
		if (environment_grid({(size_t)goal.x, (size_t)goal.y, (size_t)GridworldChannels::Agent}) != 0)
			continue;
		if (environment_grid({(size_t)goal.x, (size_t)goal.y, (size_t)GridworldChannels::Walls}) != 0)
			continue;
		if (environment_grid({(size_t)goal.x, (size_t)goal.y, (size_t)GridworldChannels::Pits}) != 0)
			continue;

		// ... but additionally check the goal surroundings - there must be at least one way out, and not going through the pit!
		bool reachable = false;
		for (size_t a=0; a<4; a++){
			mic::types::NESWAction action(a);
			mic::types::Position2D way_to_goal = goal + action;
			if ((isStateAllowed(way_to_goal)) &&
					(environment_grid({(size_t)way_to_goal.x, (size_t)way_to_goal.y, (size_t)GridworldChannels::Pits}) == 0)) {
				reachable = true;
				break;
			}//: if
		}//: for
		if (!reachable)
			continue;

		// Ok, add the goal.
		environment_grid({(size_t)goal.x, (size_t)goal.y, (size_t)GridworldChannels::Goals}) = 10;
		break;
	}//: while

}

bool Gridworld::isGridTraversible(long x_, long y_, mic::types::Matrix<bool> & visited_) {
	// If not allowed...
	if (!isStateAllowed(x_, y_))
		return false;
	// .. or is a pit...
	if (environment_grid({(size_t)x_, (size_t)y_, (size_t)GridworldChannels::Pits}) < 0)
		return false;
	// ... or wasa already visited.
	if (visited_(y_,x_))
		return false;
	// Ok found the goal!
	if (environment_grid({(size_t)x_, (size_t)y_, (size_t)GridworldChannels::Goals}) > 0)
		return true;
	// Ok, new state.
	visited_(y_,x_) = true;

	// Recursive check NESW.
	if (isGridTraversible(x_, y_-1, visited_))
		return true;
	if (isGridTraversible(x_+1, y_, visited_))
		return true;
	if (isGridTraversible(x_, y_+1, visited_))
		return true;
	if (isGridTraversible(x_-1, y_, visited_))
		return true;
	// Sorry, no luck in her.
	return false;
}



void Gridworld::initHardRandomGrid(size_t width_, size_t height_) {
	LOG(LINFO) << "Generating hard " << width_ << "x" << height_<< " random grid";
	// Overwrite the dimensions.
	width = width_;
	height = height_;

	// Set environment_grid size.
	environment_grid.resize({width, height, channels});
	environment_grid.zeros();

	// Place the player.
	mic::types::Position2D player(0, width-1, 0, height-1);
	initial_position = player;
	moveAgentToPosition(initial_position);

	// Place goal.
	while(1) {
		// Random position.
		mic::types::Position2D goal(0, width-1, 0, height-1);

		// Validate pose.
		if (environment_grid({(size_t)goal.x, (size_t)goal.y, (size_t)GridworldChannels::Agent}) != 0)
			continue;

		// Ok, add the goal.
		environment_grid({(size_t)goal.x, (size_t)goal.y, (size_t)GridworldChannels::Goals}) = 10;
		break;
	}//: while

	// Initialize random device and generator.
	std::random_device rd;
	std::mt19937_64 rng_mt19937_64(rd());


	// Initialize uniform integer distribution.
	size_t max_obstacles = sqrt(width*height) - 2;
	std::uniform_int_distribution<size_t> obstacle_dist(0, max_obstacles);

	// Calculate number of walls.
	size_t number_of_walls = obstacle_dist(rng_mt19937_64);

	// Matrix informing us thwther we already visited the state or not.
	mic::types::Matrix<bool> visited (height, width);

	// Place wall(s).
	for (size_t i=0; i<number_of_walls; i++) {
		while (1){
			// Random position.
			mic::types::Position2D wall(0, width-1, 0, height-1);

			// Validate pose.
			if (environment_grid({(size_t)wall.x, (size_t)wall.y, (size_t)GridworldChannels::Agent}) != 0)
				continue;
			if (environment_grid({(size_t)wall.x, (size_t)wall.y, (size_t)GridworldChannels::Goals}) != 0)
				continue;
			if (environment_grid({(size_t)wall.x, (size_t)wall.y, (size_t)GridworldChannels::Walls}) != 0)
				continue;

			// Add wall...
			environment_grid({(size_t)wall.x, (size_t)wall.y, (size_t)GridworldChannels::Walls}) = 1;

			// ... but additionally whether the path from agent to the goal is traversable!
			visited.setZero();
			if (!isGridTraversible(player.x, player.y, visited)) {
				// Sorry, we must remove this wall...
				environment_grid({(size_t)wall.x, (size_t)wall.y, (size_t)GridworldChannels::Walls}) = 0;
				// .. and try once again.
				continue;
			}//: if

			break;
		}//: while
	}//: for number of walls


	// Calculate number of pits.
	size_t number_of_pits = obstacle_dist(rng_mt19937_64);

	// Place pit(s).
	for (size_t i=0; i<number_of_pits; i++) {
		while(1){
			// Random position.
			mic::types::Position2D pit(0, width-1, 0, height-1);

			// Validate pose.
			if (environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Agent}) != 0)
				continue;
			if (environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Goals}) != 0)
				continue;
			if (environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Pits}) != 0)
				continue;
			if (environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Walls}) != 0)
				continue;

			// Add pit...
			environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Pits}) = -10;

			// ... but additionally whether the path from agent to the goal is traversable!
			visited.setZero();
			if (!isGridTraversible(player.x, player.y, visited)) {
				// Sorry, we must remove this pit...
				environment_grid({(size_t)pit.x, (size_t)pit.y, (size_t)GridworldChannels::Pits}) = 0;
				// .. and try once again.
				continue;
			}//: if

			break;
		}//: while
	}//: for number of walls


}


std::string Gridworld::toString() {
	std::string s;
	for (size_t y=0; y<height; y++){
		for (size_t x=0; x<width; x++) {
			// Check object occupancy.
			if (environment_grid({x,y, (size_t)GridworldChannels::Agent}) != 0) {
				// Display agent.
				s += " A ,";
			} else if (environment_grid({x,y, (size_t)GridworldChannels::Walls}) != 0) {
				// Display wall.
				s += " # ,";
			} else if (environment_grid({x,y, (size_t)GridworldChannels::Pits}) < 0) {
				// Display pit.
				s +=  " - ,";
			} else if (environment_grid({x,y, (size_t)GridworldChannels::Goals}) > 0) {
				// Display goal.
				s += " + ,";
			} else
				s += "   ,";
		}//: for x
		s += "\n";
	}//: for y

	return s;

}


mic::types::MatrixXfPtr Gridworld::encodeAgentGrid() {

	// DEBUG - copy only player pose data, avoid goals etc.
	mic::types::MatrixXfPtr encoded_grid (new mic::types::MatrixXf(height, width));
	encoded_grid->setZero();

	for (size_t y=0; y<height; y++){
		for (size_t x=0; x<width; x++) {
			// Check object occupancy.
			if (environment_grid({x,y, (size_t)GridworldChannels::Agent}) != 0) {
				// Set one.
				(*encoded_grid)(y,x) = 1;
				break;
			}
		}//: for x
	}//: for y
	encoded_grid->resize(height*width, 1);

	// Return the matrix pointer.
	return encoded_grid;
}


mic::types::MatrixXfPtr Gridworld::encode() {
	// Temporarily reshape the environment_grid.
	environment_grid.conservativeResize({1, width * height * channels});
	// Create a matrix pointer and copy data from grid into the matrix.
	mic::types::MatrixXfPtr encoded_grid (new mic::types::MatrixXf(environment_grid));
	// Back to the original shape.
	environment_grid.resize({width, height, channels});

	// Return the matrix pointer.
	return encoded_grid;
}





mic::types::Position2D Gridworld::getAgentPosition() {
	mic::types::Position2D position;
	for (size_t y=0; y<height; y++){
		for (size_t x=0; x<width; x++) {
			if (environment_grid({x,y, (size_t)GridworldChannels::Agent}) == 1) {
				position.x = x;
				position.y = y;
				return position;
			}// if
		}//: for x
	}//: for y
	// Remove warnings...
	return position;
}

void Gridworld::moveAgentToPosition(mic::types::Position2D pos_) {
	// Clear old.
	mic::types::Position2D old = getAgentPosition();
	environment_grid({(size_t)old.x, (size_t)old.y, (size_t)GridworldChannels::Agent}) = 0;
	// Set new.
	environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Agent}) = 1;
}


float Gridworld::getStateReward(mic::types::Position2D pos_) {
	// Check reward - goal or pit.
    if (environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Pits}) != 0)
        return environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Pits});
    else if (environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Goals}) != 0)
		return environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Goals});
	else
        return 0;
}


bool Gridworld::isStateAllowed(mic::types::Position2D pos_) {
	if ((pos_.x < 0) || (pos_.x >= width))
		return false;

	if ((pos_.y < 0) || (pos_.y >= height))
			return false;

	// Check walls!
	if (environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Walls}) != 0)
		return false;

	return true;
}


bool Gridworld::isStateTerminal(mic::types::Position2D pos_) {
	if ((pos_.x < 0) || (pos_.x >= width))
		return false;

	if ((pos_.y < 0) || (pos_.y >= height))
			return false;

	// Check reward - goal or pit.
    if (environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Pits}) != 0)
        return true;
    else if (environment_grid({(size_t)pos_.x, (size_t)pos_.y, (size_t)GridworldChannels::Goals}) != 0)
        return true;
    else

        return false;
}


} /* namespace environments */
} /* namespace mic */
