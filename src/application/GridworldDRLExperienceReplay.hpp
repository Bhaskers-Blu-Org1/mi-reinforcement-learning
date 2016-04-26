/*!
 * \file GridworldDRLExperienceReplay.hpp
 * \brief 
 * \author tkornut
 * \date Apr 26, 2016
 */

#ifndef SRC_APPLICATION_GRIDWORLDDRLEXPERIENCEREPLAY_HPP_
#define SRC_APPLICATION_GRIDWORLDDRLEXPERIENCEREPLAY_HPP_

#include <vector>
#include <string>

#include <opengl/application/OpenGLEpisodicApplication.hpp>
#include <opengl/visualization/WindowFloatCollectorChart.hpp>
using namespace mic::opengl::visualization;

#include <mlnn/MultiLayerNeuralNetwork.hpp>
// Using multi layer neural networks
using namespace mic::mlnn;
using namespace mic::types;

#include <types/Gridworld.hpp>

namespace mic {
namespace application {


/*!
 * \brief Class responsible for solving the gridworld problem with Q-learning, neural network used for approximation of the rewards and experience replay using for (batch) training of the neural network.
 * \author tkornuta
 */
class GridworldDRLExperienceReplay: public mic::opengl::application::OpenGLEpisodicApplication {
public:
	/*!
	 * Default Constructor. Sets the application/node name, default values of variables, initializes classifier etc.
	 * @param node_name_ Name of the application/node (in configuration file).
	 */
	GridworldDRLExperienceReplay(std::string node_name_ = "application");

	/*!
	 * Destructor.
	 */
	virtual ~GridworldDRLExperienceReplay();

protected:

	/*!
	 * Method initializes GLUT and OpenGL windows.
	 * @param argc Number of application parameters.
	 * @param argv Array of application parameters.
	 */
	virtual void initialize(int argc, char* argv[]);

	/*!
	 * Initializes all variables that are property-dependent.
	 */
	virtual void initializePropertyDependentVariables();

	/*!
	 * Performs single step of computations.
	 */
	virtual bool performSingleStep();

	/*!
	 * Method called at the beginning of new episode (goal: to reset the statistics etc.) - abstract, to be overridden.
	 */
	virtual void startNewEpisode();

	/*!
	 * Method called when given episode ends (goal: export collected statistics to file etc.) - abstract, to be overridden.
	 */
	virtual void finishCurrentEpisode();


private:

	/// Window for displaying ???.
	WindowFloatCollectorChart* w_chart;

	/// Data collector.
	mic::data_io::DataCollectorPtr<std::string, float> collector_ptr;

	/// The gridworld object - current state.
	mic::types::Gridworld state;

	/// Property: type of gridworld. Please refer to Gridworld::generateGridworld() method for more details on types of gridworlds.
	mic::configuration::Property<short> gridworld_type;

	/// Property: width of gridworld.
	mic::configuration::Property<size_t> width;

	/// Property: height of gridworld.
	mic::configuration::Property<size_t> height;

	/*!
	 * Property: the "expected intermediate reward", i.e. reward received by performing each step (typically negative, but can be positive as all).
	 */
	mic::configuration::Property<float> step_reward;

	/*!
	 * Property: future discount (should be in range 0.0-1.0).
	 */
	mic::configuration::Property<float> discount_rate;

	/*!
	 * Property: learning rate (should be in range 0.0-1.0).
	 */
	mic::configuration::Property<float> learning_rate;

	/*!
	 * Property: move noise, determining gow often action results in unintended direction.
	 */
	mic::configuration::Property<float> move_noise;

	/*!
	 * Property: variable denoting epsilon in action selection (the probability "below" which a random action will be selected).
	 * if epsilon < 0 then if will be set to 1/episode, hence change dynamically depending on the episode number.
	 */
	mic::configuration::Property<double> epsilon;

	/// Property: name of the file to which the statistics will be exported.
	mic::configuration::Property<std::string> statistics_filename;

	/// Multi-layer neural network used for approximation of the Qstate rewards.
	MultiLayerNeuralNetwork neural_net;

	/*!
	 * Performs "deterministic" move. It is assumed that the move is truncated by the gridworld boundaries (no circular world assumption).
	 * @param ac_ The action to be performed.
	 * @return True if move was performed, false if it was not possible.
	 */
	bool move (mic::types::Action2DInterface ac_);

	/*!
	 * Calculates the best value for the current state - by finding the action having the maximal expected value.
	 * @return Value for given state.
	 */
	float computeBestValueForCurrentState();

	/*!
	 * Returns the predicted rewards for given state.
	 * @return Pointerr to predicted rewards (network output matrix).
	 */
	mic::types::MatrixXfPtr getPredictedRewardsForCurrentState();

	/*!
	 * Finds the best action for the current state.
	 * @return The best action found.
	 */
	mic::types::NESWAction selectBestActionForCurrentState();

	/*!
	 * Steams the current network response - values of actions associates with consecutive agent poses.
	 * @return Ostream with description of the state-action table.
	 */
	std::string streamNetworkResponseTable();

	/*!
	 * Used in statistics.
	 */
	long long sum_of_iterations;

	/*!
	 * A desired motion trajectory.
	 */
	std::vector<mic::types::NESWAction> trajectory;
	size_t step_number;

	/*!
	 * Player position at time (t-1).
	 */
	mic::types::Position2D player_pos_t_minus_prim;
};

} /* namespace application */
} /* namespace mic */


#endif /* SRC_APPLICATION_GRIDWORLDDRLEXPERIENCEREPLAY_HPP_ */
