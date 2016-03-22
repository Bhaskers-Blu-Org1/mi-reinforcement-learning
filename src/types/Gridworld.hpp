/*!
 * \file Gridworld.hpp
 * \brief 
 * \author tkornut
 * \date Mar 21, 2016
 */

#ifndef SRC_TYPES_GRIDWORLD_HPP_
#define SRC_TYPES_GRIDWORLD_HPP_

#include <types/Position2D.hpp>
#include <types/TensorTypes.hpp>
#include <logger/Log.hpp>

namespace mic {
namespace types {

/*!
 * \brief Gridworld channels
 * \author tkornuta
 */
enum class GridworldChannels : std::size_t
{
	Goal = 0, ///< Channel storing the goal(s)
	Pit = 1, ///< Channel storing the pits(s)
	Wall = 2, ///< Channel storing the walls(s)
	Player = 3 ///< Channel storing the player pose
};


/*!
 * \brief Class responsible for generation and presentation of gridworld environments.
 * \author tkornuta
 */
class Gridworld {
public:
	/*!
	 * Constructor. Creates and empty gridworld (set size to 0x0).
	 */
	Gridworld();

	/*!
	 * Destructor. Empty for now.
	 */
	virtual ~Gridworld();

	/*!
	 * 	Method initializes the exemplary grid.
	 *
	 * [[' ',' ',' ',' '],
	 *  ['S',-10,' ',' '],
	 *  [' ','','#',' '],
	 *  [' ',' ',' ',10]]
	 */
	void initExemplaryGrid();

	/*!
	 * Initializes the classic cliff gridworld.
	 *
	 * [[' ',' ',' ',' ',' '],
	 *  ['S',' ',' ',' ',10],
	 *  [-100,-100, -100, -100, -100]]
	 */
	void initClassicCliffGrid();

	/*!
	 * Initializes the classic discount gridworld.
	 *
	 * [[' ',' ',' ',' ',' '],
	 *  [' ','#',' ',' ',' '],
	 *  [' ','#', 1,'#', 10],
	 *   ['S',' ',' ',' ',' '],
	 *   [-10,-10, -10, -10, -10]]
	 */
	void initDiscountGrid();

	/*!
	 * Initializes the classic discount gridworld.
	 *
	 * [[ '#',-100, -100, -100, -100, -100, '#'],
	 *  [   1, 'S',  ' ',  ' ',  ' ',  ' ',  10],
	 *  [ '#',-100, -100, -100, -100, -100, '#']]
	 */
	void initBridgeGrid();

	/*!
	 * Initializes the classic Book gridworld - example from Sutton&Barto book on RL.
	 *
	 * [[' ',' ',' ',+1],
	 *  [' ','#',' ',-1],
	 *  ['S',' ',' ',' ']]
	 */
	void initBookGrid();

	/*!
	 * Initializes the classic maze gridworld.
	 *
	 * [[' ',' ',' ',+1],
	 *  ['#','#',' ','#'],
	 *  [' ','#',' ',' '],
	 *  [' ','#','#',' '],
	 *  ['S',' ',' ',' ']]
	 */
	void initMazeGrid();

	/*!
	 * Generates a random grid of size (width x height).
	 */
	void initRandomGrid(size_t width_, size_t height_);

	/*!
	 * Returns the (flattened, i.e. 2D) grid of characters.
	 * @return Flattened grid of chars.
	 */
	mic::types::Tensor<char> flattenGrid();

	/*!
	 * Steams the current state of the gridworld.
	 * @return Ostream with description of the gridworld.
	 */
	std::string streamGrid();

	/*!
	 * Calculates the player position.
	 * @return Player position.
	 */
	mic::types::Position2D getPlayerPosition();

	/*!
	 * Move player to the position.
	 * @param pos_ Position to be checked.
	 * @param pos_ The position to be set.
	 */
	void movePlayerToPosition(mic::types::Position2D pos_);


	/*!
	 * Returns the reward associated with the given state.
	 * @param pos_ Position (state).
	 * @return Reward for being in given state (r).
	 */
	float getStateReward(mic::types::Position2D pos_);

	/*!
	 * Checks if position is allowed, i.e. within the gridworld boundaries and there is no wall at that place.
	 * @param pos_ Position to be checked.
	 * @return True if the position is allowed, false otherwise.
	 */
	bool isStateAllowed(mic::types::Position2D pos_);

	/*!
	 * Checks if position is terminal, i.e. player is standing in a pit or reached the goal. Returns reward associated with given state.
	 * @param pos_ Position (state) to be checked.
	 * @return The reward associated with "final" action (might be positive or negative), equal to zero means that the position is not final.
	 */
	bool isStateTerminal(mic::types::Position2D pos_);

	/*!
	 * Checks whether performing given action starting in given state is allowed.
	 * @param pos_ Starting state (position).
	 * @param ac_ Action to be performed.
	 * @return True if action is allowed, false otherwise.
	 */
	bool isActionAllowed(mic::types::Position2D pos_, mic::types::Action2DInterface ac_);

	/*!
	 * Returns current width of the gridworld.
	 * @return Width.
	 */
	size_t getWidth() { return width; }

	/*!
	 * Returns current height of the gridworld.
	 * @return Height.
	 */
	size_t getHeight() { return height; }

protected:

	/// Width of gridworld.
	size_t width;

	/// Height of gridworld.
	size_t height;

	/// Property: height of gridworld.
	mic::types::Position2D initial_position;

	/// Tensor storing the 3D Gridworld (x + y + 4 "depth" channels representing: 0 - goals, 1 - pits, 2 - walls, 3 - player).
	mic::types::TensorXf gridworld;

};

} /* namespace types */
} /* namespace mic */

#endif /* SRC_TYPES_GRIDWORLD_HPP_ */
