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
 * \file MNISTDLREPPOMDP.cpp
 * \brief 
 * \author tkornut
 * \date Jun 8, 2016
 */

#include <application/MNISTDigitDLRERPOMDP.hpp>

#include <limits>
#include <utils/RandomGenerator.hpp>


namespace mic {
namespace application {

/*!
 * \brief Registers the application.
 * \author tkornuta
 */
void RegisterApplication (void) {
	REGISTER_APPLICATION(mic::application::MNISTDigitDLRERPOMDP);
}


MNISTDigitDLRERPOMDP::MNISTDigitDLRERPOMDP(std::string node_name_) : OpenGLEpisodicApplication(node_name_),
		saccadic_path(new std::vector <mic::types::Position2D>()),
		step_reward("step_reward", 0.0),
		discount_rate("discount_rate", 0.9),
		learning_rate("learning_rate", 0.005),
		epsilon("epsilon", 0.1),
		step_limit("step_limit",0),
		statistics_filename("statistics_filename","mnist_digit_drl_er_statistics.csv"),
		mlnn_filename("mlnn_filename", "mnist_digit_drl_er_mlnn.txt"),
		mlnn_save("mlnn_save", false),
		mlnn_load("mlnn_load", false),
		experiences(10000,1)
	{
	// Register properties - so their values can be overridden (read from the configuration file).
	registerProperty(step_reward);
	registerProperty(discount_rate);
	registerProperty(learning_rate);
	registerProperty(epsilon);
	registerProperty(step_limit);
	registerProperty(statistics_filename);
	registerProperty(mlnn_filename);
	registerProperty(mlnn_save);
	registerProperty(mlnn_load);

	LOG(LINFO) << "Properties registered";
}


MNISTDigitDLRERPOMDP::~MNISTDigitDLRERPOMDP() {
	delete(w_chart);
	delete(wmd_environment);
	delete(wmd_observation);	
}


void MNISTDigitDLRERPOMDP::initialize(int argc, char* argv[]) {
	// Initialize GLUT! :]
	VGL_MANAGER->initializeGLUT(argc, argv);

	collector_ptr = std::make_shared < mic::utils::DataCollector<std::string, float> >( );
	// Add containers to collector.
	collector_ptr->createContainer("path_length_episode",  mic::types::color_rgba(0, 255, 0, 180));
	collector_ptr->createContainer("path_length_average", mic::types::color_rgba(255, 255, 0, 180));
	collector_ptr->createContainer("path_length_optimal", mic::types::color_rgba(255, 255, 255, 180));
	collector_ptr->createContainer("path_length_diff", mic::types::color_rgba(255, 0, 0, 180));

	sum_of_iterations = 0;

	// Create the visualization windows - must be created in the same, main thread :]
	w_chart = new WindowCollectorChart<float>("MNISTDigitDLRERPOMDP", 256, 512, 0, 0);
	w_chart->setDataCollectorPtr(collector_ptr);

}

void MNISTDigitDLRERPOMDP::initializePropertyDependentVariables() {
	// Create windows for the visualization of the whole environment and a single observation.
	wmd_environment = new WindowMNISTDigit("Environment", env.getEnvironmentHeight()*20,env.getEnvironmentWidth()*20, 0, 316);
	wmd_observation = new WindowMNISTDigit("Observation", env.getObservationHeight()*20,env.getObservationWidth()*20, env.getEnvironmentWidth()*20, 316);


	// Hardcode batchsize - for fastening the display!
	batch_size = env.getObservationWidth() * env.getObservationHeight();

	// Try to load neural network from file.
	if ((mlnn_load) && (neural_net.load(mlnn_filename))) {
		// Do nothing ;)
	} else {
		// Create a simple neural network.
		// gridworld wxhx4 -> 100 -> 4 -> regression!.
		neural_net.pushLayer(new Linear<float>((size_t) env.getObservationSize(), 250));
		neural_net.pushLayer(new ReLU<float>(250));
		neural_net.pushLayer(new Linear<float>(250, 100));
		neural_net.pushLayer(new ReLU<float>(100));
		neural_net.pushLayer(new Linear<float>(100, 4));

		// Set batch size.
		neural_net.resizeBatch(batch_size);
		// Change optimization function from default GradientDescent to Adam.
		neural_net.setOptimization<mic::neural_nets::optimization::Adam<float> >();
		// Set loss function -> regression!
		neural_net.setLoss <mic::neural_nets::loss::SquaredErrorLoss<float> >();

		LOG(LINFO) << "Generated new neural network";
	}//: else

	// Set batch size in experience replay memory.
	experiences.setBatchSize(batch_size);

	// Set displayed matrix pointers.
	wmd_environment->setDigitPointer(env.getEnvironment());
	wmd_environment->setPathPointer(saccadic_path);
	wmd_observation->setDigitPointer(env.getObservation());

}


void MNISTDigitDLRERPOMDP::startNewEpisode() {
	LOG(LSTATUS) << "Starting new episode " << episode;

	// Generate the gridworld (and move player to initial position).
	env.initializeEnvironment();
	saccadic_path->clear();
	// Add first, initial position to  to saccadic path.
	saccadic_path->push_back(env.getAgentPosition());

	/*LOG(LNOTICE) << "Network responses: \n" <<  streamNetworkResponseTable();
	LOG(LNOTICE) << "Observation: \n"  << env.observationToString();
	LOG(LNOTICE) << "Environment: \n" << env.environmentToString();*/
	// Do not forget to get the current observation!
	env.getObservation();
}


void MNISTDigitDLRERPOMDP::finishCurrentEpisode() {
	LOG(LTRACE) << "End current episode";

	sum_of_iterations += iteration -1; // -1 is the fix related to moving the terminal condition to the front of step!

	// Add variables to container.
	collector_ptr->addDataToContainer("path_length_episode",(iteration -1));
	collector_ptr->addDataToContainer("path_length_average",(float)sum_of_iterations/episode);
	collector_ptr->addDataToContainer("path_length_optimal", (float)env.optimalPathLength());
	collector_ptr->addDataToContainer("path_length_diff", (float)(iteration -1 - env.optimalPathLength()));


	// Export reward "convergence" diagram.
	collector_ptr->exportDataToCsv(statistics_filename);

	// Save nn to file.
	if (mlnn_save && (episode %10))
		neural_net.save(mlnn_filename);
}


std::string MNISTDigitDLRERPOMDP::streamNetworkResponseTable() {
	LOG(LTRACE) << "streamNetworkResponseTable()";
	std::string rewards_table;
	std::string actions_table;

	// Remember the current state i.e. player position.
	mic::types::Position2D current_player_pos_t = env.getAgentPosition();

	// Create new matrices for batches of inputs and targets.
	MatrixXfPtr inputs_batch(new MatrixXf(env.getObservationSize(), batch_size));

	// Assume that the batch_size = grid_env.getWidth() * grid_env.getHeight()
	assert(env.getObservationWidth()*env.getObservationHeight() == batch_size);


	size_t dx = (env.getObservationWidth()-1)/2;
	size_t dy = (env.getObservationHeight()-1)/2;
	mic::types::Position2D p = env.getAgentPosition();

	// Copy data.
	for (long oy=0, ey=(p.y-dy); oy<(long)env.getObservationHeight(); oy++, ey++){
		for (long ox=0, ex=(p.x-dx); ox<(long)env.getObservationWidth(); ox++, ex++) {

			// Move the player to given state - disregarding whether it was successful or not, answers for walls/positions outside of the gridworld do not interes us anyway...
			if (!env.moveAgentToPosition(Position2D(ex,ey)))
				LOG(LDEBUG) << "Failed!"; //... but still we can live with that... ;)
			// Encode the current state.
			mic::types::MatrixXfPtr encoded_state = env.encodeObservation();
			// Add to batch.
			inputs_batch->col(oy*env.getObservationWidth()+ox) = encoded_state->col(0);
		}//: for x
	}//: for y

	// Get rewards for the whole batch.
	neural_net.forward(inputs_batch);
	// Get predictions for all those states - there is no need to create a copy.
	MatrixXfPtr predicted_batch = neural_net.getPredictions();


	rewards_table += "Action values:\n";
	actions_table += "Best actions:\n";
	// Generate all possible states and all possible rewards.
	for (long oy=0, ey=(p.y-dy); oy<(long)env.getObservationHeight(); oy++, ey++){
		rewards_table += "| ";
		actions_table += "| ";
		for (long ox=0, ex=(p.x-dx); ox<(long)env.getObservationWidth(); ox++, ex++) {
			float bestqval = -std::numeric_limits<float>::infinity();
			size_t best_action = -1;
			for (size_t a=0; a<4; a++) {
				float qval = (*predicted_batch)(a, oy*env.getObservationWidth()+ox);

				rewards_table += std::to_string(qval);
				if (a==3)
					rewards_table += " | ";
				else
					rewards_table += " , ";

				// Remember the best value.
				if (env.isStateAllowed(ex,ey) && (!env.isStateTerminal(ex,ey)) && env.isActionAllowed(ex,ey,a) && (qval > bestqval)){
					bestqval = qval;
					best_action = a;
				}//: if

			}//: for a(ctions)
			switch(best_action){
				case 0 : actions_table += "N | "; break;
				case 1 : actions_table += "E | "; break;
				case 2 : actions_table += "S | "; break;
				case 3 : actions_table += "W | "; break;
				default: actions_table += "- | ";
			}//: switch

		}//: for x
		rewards_table += "\n";
		actions_table += "\n";
	}//: for y

	// Move player to previous position.
	env.moveAgentToPosition(current_player_pos_t);

	return rewards_table + actions_table;
}



float MNISTDigitDLRERPOMDP::computeBestValueForGivenStateAndPredictions(mic::types::Position2D player_position_, float* predictions_){
	LOG(LTRACE) << "computeBestValueForGivenState()";
	float best_qvalue = -std::numeric_limits<float>::infinity();

	// Create a list of possible actions.
	std::vector<mic::types::NESWAction> actions;
	actions.push_back(A_NORTH);
	actions.push_back(A_EAST);
	actions.push_back(A_SOUTH);
	actions.push_back(A_WEST);

	for(mic::types::NESWAction action : actions) {
		// .. and find the value of the best allowed action.
		if(env.isActionAllowed(player_position_, action)) {
			float qvalue = predictions_[(size_t)action.getType()];
			if (qvalue > best_qvalue)
				best_qvalue = qvalue;
		}//if is allowed
	}//: for

	return best_qvalue;
}


mic::types::MatrixXfPtr MNISTDigitDLRERPOMDP::getPredictedRewardsForGivenState(mic::types::Position2D player_position_) {
	LOG(LTRACE) << "getPredictedRewardsForGivenState()";
	// Remember the current state i.e. player position.
	mic::types::Position2D current_player_pos_t = env.getAgentPosition();

	// Move the player to given state.
	env.moveAgentToPosition(player_position_);

	// Encode the current state.
	mic::types::MatrixXfPtr encoded_state = env.encodeObservation();

	// Create NEW matrix for the inputs batch.
	MatrixXfPtr inputs_batch(new MatrixXf(env.getObservationSize(), batch_size));
	inputs_batch->setZero();

	// Set the first input - only this one interests us.
	inputs_batch->col(0) = encoded_state->col(0);

	//LOG(LERROR) << "Getting predictions for input batch:\n" <<inputs_batch->transpose();

	// Pass the data and get predictions.
	neural_net.forward(inputs_batch);

	MatrixXfPtr predictions_batch = neural_net.getPredictions();

	//LOG(LERROR) << "Resulting predictions batch:\n" << predictions_batch->transpose();

	// Get the first prediction only.
	MatrixXfPtr predictions_sample(new MatrixXf(4, 1));
	predictions_sample->col(0) = predictions_batch->col(0);

	//LOG(LERROR) << "Returned predictions sample:\n" << predictions_sample->transpose();

	// Move player to previous position.
	env.moveAgentToPosition(current_player_pos_t);

	// Return the predictions.
	return predictions_sample;
}

mic::types::NESWAction MNISTDigitDLRERPOMDP::selectBestActionForGivenState(mic::types::Position2D player_position_){
	LOG(LTRACE) << "selectBestAction";

	// Greedy methods - returns the index of element with greatest value.
	mic::types::NESWAction best_action = A_RANDOM;
    float best_qvalue = -std::numeric_limits<float>::infinity();

	// Create a list of possible actions.
	std::vector<mic::types::NESWAction> actions;
	actions.push_back(A_NORTH);
	actions.push_back(A_EAST);
	actions.push_back(A_SOUTH);
	actions.push_back(A_WEST);

	// Check the results of actions one by one... (there is no need to create a separate copy of predictions)
	MatrixXfPtr predictions_sample = getPredictedRewardsForGivenState(player_position_);
	//LOG(LERROR) << "Selecting action from predictions:\n" << predictions_sample->transpose();
	float* pred = predictions_sample->data();

	for(size_t a=0; a<4; a++) {
		// Find the best action allowed.
		if(env.isActionAllowed(player_position_, mic::types::NESWAction((mic::types::NESW)a))) {
			float qvalue = pred[a];
			if (qvalue > best_qvalue){
				best_qvalue = qvalue;
				best_action.setAction((mic::types::NESW)a);
			}
		}//if is allowed
	}//: for

	return best_action;
}

bool MNISTDigitDLRERPOMDP::performSingleStep() {
	LOG(LSTATUS) << "Episode "<< episode << ": step " << iteration << "";

	// Get player pos at time t.
	mic::types::Position2D player_pos_t= env.getAgentPosition();
	LOG(LINFO) << "Agent position at state t: " << player_pos_t;

	// Check whether state t is terminal - finish the episode.
	if(env.isStateTerminal(player_pos_t))
		return false;

	// TMP!
	double 	nn_weight_decay = 0;

	// Select the action.
	mic::types::NESWAction action;
	//action = A_NORTH;
	double eps = (double)epsilon;
	if ((double)epsilon < 0)
		eps = 1.0/(1.0+sqrt(episode));
	if (eps < 0.1)
		eps = 0.1;
	LOG(LDEBUG) << "eps = " << eps;
	bool random = false;

	// Epsilon-greedy action selection.
	if (RAN_GEN->uniRandReal() > eps){
		// Select best action.
		action = selectBestActionForGivenState(player_pos_t);
	} else {
		// Random action.
		action = A_RANDOM;
		random = true;
	}//: if

	// Execute action - do not monitor the success.
	env.moveAgent(action);

	// Get new state s(t+1).
	mic::types::Position2D player_pos_t_prim = env.getAgentPosition();
	LOG(LINFO) << "Agent position at t+1: " << player_pos_t_prim << " after performing the action = " << action << ((random) ? " [Random]" : "");

	// Add this position to  to saccadic path.
	saccadic_path->push_back(player_pos_t_prim);

	// Collect the experience.
	SpatialExperiencePtr exp(new SpatialExperience(player_pos_t, action, player_pos_t_prim));
	// Create an empty matrix for rewards - this will be recalculated each time the experience will be replayed anyway.
	MatrixXfPtr rewards (new MatrixXf(4 , batch_size));
	// Add experience to experience table.
	experiences.add(exp, rewards);


	// Deep Q learning - train network with random sample from the experience memory.
	if (experiences.size() >= 2*batch_size) {
		// Create new matrices for batches of inputs and targets.
		MatrixXfPtr inputs_t_batch(new MatrixXf(env.getObservationSize(), batch_size));
		MatrixXfPtr inputs_t_prim_batch(new MatrixXf(env.getObservationSize(), batch_size));
		MatrixXfPtr targets_t_batch(new MatrixXf(4, batch_size));

		// Get the random batch.
		SpatialExperienceBatch geb = experiences.getRandomBatch();

		// Debug purposes.
		geb.setNextSampleIndex(0);
		for (size_t i=0; i<batch_size; i++) {
			SpatialExperienceSample ges = geb.getNextSample();
			SpatialExperiencePtr ge_ptr = ges.data();
			LOG(LDEBUG) << "Training sample : " << ge_ptr->s_t << " -> " << ge_ptr->a_t << " -> " << ge_ptr->s_t_prim;
		}//: for

		// Iterate through samples and create inputs_t_batch.
		for (size_t i=0; i<batch_size; i++) {
			SpatialExperienceSample ges = geb.getNextSample();
			SpatialExperiencePtr ge_ptr = ges.data();

			// Replay the experience.
			// "Simulate" moving player to position from state/time (t).
			env.moveAgentToPosition(ge_ptr->s_t);
			// Encode the state at time (t).
			mic::types::MatrixXfPtr encoded_state_t = env.encodeObservation();
			//float* state = encoded_state_t->data();

			// Copy the encoded state to inputs batch.
			inputs_t_batch->col(i) = encoded_state_t->col(0);
		}// for samples.

		// Get network responses.
		neural_net.forward(inputs_t_batch);
		// Get predictions for all those states...
		MatrixXfPtr predictions_t_batch = neural_net.getPredictions();
		// ... and copy them to reward pointer - a container which we will modify.
		(*targets_t_batch) = (*predictions_t_batch);

		// Iterate through samples and create inputs_t_prim_batch.
		geb.setNextSampleIndex(0);
		for (size_t i=0; i<batch_size; i++) {
			SpatialExperienceSample ges = geb.getNextSample();
			SpatialExperiencePtr ge_ptr = ges.data();

			// Replay the experience.
			// "Simulate" moving player to position from state/time (t+1).
			env.moveAgentToPosition(ge_ptr->s_t_prim);
			// Encode the state at time (t+1).
			mic::types::MatrixXfPtr encoded_state_t = env.encodeObservation();
			//float* state = encoded_state_t->data();

			// Copy the encoded state to inputs batch.
			inputs_t_prim_batch->col(i) = encoded_state_t->col(0);
		}// for samples.

		// Get network responses.
		neural_net.forward(inputs_t_prim_batch);
		// Get predictions for all those states...
		MatrixXfPtr predictions_t_prim_batch = neural_net.getPredictions();

		// Calculate the rewards, one by one.
		// Iterate through samples and create inputs_t_prim_batch.
		geb.setNextSampleIndex(0);
		for (size_t i=0; i<batch_size; i++) {
			SpatialExperienceSample ges = geb.getNextSample();
			SpatialExperiencePtr ge_ptr = ges.data();

			if (ge_ptr->s_t == ge_ptr->s_t_prim) {
				// The move was not possible! Learn that as well.
				(*targets_t_batch)((size_t)ge_ptr->a_t.getType(), i) = 3*step_reward;
			} else if(env.isStateTerminal(ge_ptr->s_t_prim)) {
				// The position at (t+1) state appears to be terminal - learn the reward.
				(*targets_t_batch)((size_t)ge_ptr->a_t.getType(), i) = env.getStateReward(ge_ptr->s_t_prim);
			} else {
				MatrixXfPtr preds_t_prim (new MatrixXf(4, 1));
				preds_t_prim->col(0) = predictions_t_prim_batch->col(i);
				// Get best value for the NEXT state - position from (t+1) state.
				float max_q_st_prim_at_prim = computeBestValueForGivenStateAndPredictions(ge_ptr->s_t_prim, preds_t_prim->data());
				// If next state best value is finite.
				// Update running average for given action - Deep Q learning!
				if (std::isfinite(max_q_st_prim_at_prim))
					(*targets_t_batch)((size_t)ge_ptr->a_t.getType(), i) = step_reward + discount_rate*max_q_st_prim_at_prim;
			}//: else

		}//: for

		LOG(LDEBUG) <<"Inputs batch:\n" << inputs_t_batch->transpose();
		LOG(LDEBUG) <<"Targets batch:\n" << targets_t_batch->transpose();

		// Perform the Deep-Q-learning.
		//LOG(LDEBUG) << "Network responses before training:" << std::endl << streamNetworkResponseTable();

		// Train network with rewards.
		float loss = neural_net.train (inputs_t_batch, targets_t_batch, learning_rate, nn_weight_decay);
		LOG(LDEBUG) << "Training loss:" << loss;

		//LOG(LDEBUG) << "Network responses after training:" << std::endl << streamNetworkResponseTable();

		// Finish the replay: move the player to REAL, CURRENT POSITION.
		env.moveAgentToPosition(player_pos_t_prim);
	}//: if enough experiences
	else
		LOG(LWARNING) << "Not enough samples in the experience replay memory!";

	LOG(LNOTICE) << "Network responses: \n" << streamNetworkResponseTable();
	LOG(LNOTICE) << "Observation: \n"  << env.observationToString();
	LOG(LNOTICE) << "Environment: \n"  << env.environmentToString();
	// Do not forget to get the current observation!
	env.getObservation();

	// Check whether we reached maximum number of iterations.
	if ((step_limit>0) && (iteration > (size_t)step_limit))
		return false;

	return true;
}

} /* namespace application */
} /* namespace mic */
