/*!
 * Copyright (C) tkornuta, IBM Corporation 2015-2019
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*!
 * \file MazeOfDigitsDLRERPOMPD.hpp
 * \brief 
 * \author tkornut
 * \date May 6, 2016
 */

#ifndef SRC_APPLICATION_MAZEOFDIGITSDLRERPOMPD_HPP_
#define SRC_APPLICATION_MAZEOFDIGITSDLRERPOMPD_HPP_

#include <vector>
#include <string>

#include <opengl/application/OpenGLEpisodicApplication.hpp>
#include <opengl/visualization/WindowCollectorChart.hpp>
#include <opengl/visualization/WindowMazeOfDigits.hpp>
using namespace mic::opengl::visualization;

#include <mlnn/BackpropagationNeuralNetwork.hpp>
// Using multi layer neural networks
using namespace mic::mlnn;
using namespace mic::types;

#include <types/MazeOfDigits.hpp>
#include <types/SpatialExperienceMemory.hpp>

namespace mic {
namespace application {


/*!
 * \brief Application of Partially Observable Deep Q-learning with Experience Reply to the maze of digits problem.
 * There is an assumption that the agent observes only part of the environment (POMPD).
 * \author tkornuta
 */
class MazeOfDigitsDLRERPOMPD: public mic::opengl::application::OpenGLEpisodicApplication {
public:
	/*!
	 * Default Constructor. Sets the application/node name, default values of variables etc.
	 * @param node_name_ Name of the application/node (in configuration file).
	 */
	MazeOfDigitsDLRERPOMPD(std::string node_name_ = "application");

	/*!
	 * Destructor.
	 */
	virtual ~MazeOfDigitsDLRERPOMPD();

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

	/// Window for displaying statistics.
	WindowCollectorChart<float>* w_chart;

	/// Data collector.
	mic::utils::DataCollectorPtr<std::string, float> collector_ptr;

	/// Window displaying the whole environment.
	WindowMazeOfDigits* wmd_environment;
	/// Window displaying the observation.
	WindowMazeOfDigits* wmd_observation;


	/// The maze of digits environment.
	mic::environments::MazeOfDigits env;

	/// Saccadic path - a sequence of consecutive agent positions.
	std::shared_ptr<std::vector <mic::types::Position2D> > saccadic_path;

	/// Size of the batch in experience replay - set to the size of maze (width*height).
	size_t batch_size;

	/*!
	 * Property: the "expected intermediate reward", i.e. reward received by performing each step (typically negative, but can be positive as all).
	 */
	mic::configuration::Property<float> step_reward;

	/*!
	 * Property: future discount (should be in range 0.0-1.0).
	 */
	mic::configuration::Property<float> discount_rate;

	/*!
	 * Property: neural network learning rate (should be in range 0.0-1.0).
	 */
	mic::configuration::Property<float> learning_rate;

	/*!
	 * Property: variable denoting epsilon in action selection (the probability "below" which a random action will be selected).
	 * if epsilon < 0 then if will be set to 1/episode, hence change dynamically depending on the episode number.
	 */
	mic::configuration::Property<double> epsilon;

	/*!
	 * Limit of steps for episode. Setting step_limit <= 0 means that the limit should not be considered.
	 */
	mic::configuration::Property<int> step_limit;

	/// Property: name of the file to which the statistics will be exported.
	mic::configuration::Property<std::string> statistics_filename;

	/// Property: name of the file to which the neural network will be serialized (or deserialized from).
	mic::configuration::Property<std::string> mlnn_filename;

	/// Property: flad denoting thether the nn should be saved to a file (after every episode end).
	mic::configuration::Property<bool> mlnn_save;

	/// Property: flad denoting thether the nn should be loaded from a file (at the initialization of the task).
	mic::configuration::Property<bool> mlnn_load;

	/// Multi-layer neural network used for approximation of the Qstate rewards.
	BackpropagationNeuralNetwork<float> neural_net;

	/*!
	 * Calculates the best value for the current state and predictions.
	 * @param player_position_ State (player position).
	 * @param predictions_ Vector of predictions to be analyzed.
	 * @return Value of the best possible action for given state.
	 */
	float computeBestValueForGivenStateAndPredictions(mic::types::Position2D player_position_, float* predictions_);

	/*!
	 * Returns the predicted rewards for given state.
	 * @param player_position_ State (player position).
	 * @return Pointer to the predicted rewards (network output matrix).
	 */
	mic::types::MatrixXfPtr getPredictedRewardsForGivenState(mic::types::Position2D player_position_);

	/*!
	 * Finds the best action for the current state.
	 * @param player_position_ State (player position).
	 * @return The best action found.
	 */
	mic::types::NESWAction selectBestActionForGivenState(mic::types::Position2D player_position_);

	/*!
	 * Steams the current network response - values of actions associates with consecutive agent poses.
	 * @return Ostream with description of the state-action table.
	 */
	std::string streamNetworkResponseTable();

	/*!
	 * Sum of all iterations made till now - used in statistics.
	 */
	long long sum_of_iterations;

	/*!
	 * Sum of optimal to episodic path lengths.
	 */
	double sum_of_opt_to_episodic_lenghts;

	/*!
	 * Table of past experiences.
	 */
	SpatialExperienceMemory experiences;
};

} /* namespace application */
} /* namespace mic */

#endif /* SRC_APPLICATION_MAZEOFDIGITSDLRERPOMPD_HPP_ */
