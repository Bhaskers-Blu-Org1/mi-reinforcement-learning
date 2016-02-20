/*!
 * \file HistogramFilterMazeLocalization.cpp
 * \brief File contains definition of methods of histogram filter based maze localization application.
 * \author tkornut
 * \date Jan 27, 2016
 */

#include <application/ApplicationFactory.hpp>

//#include <random>
#include <data_utils/RandomGenerator.hpp>
#include <applications/HistogramFilterMazeLocalization.hpp>


namespace mic {
namespace application {

/*!
 * \brief Registers application.
 * \author tkornuta
 */
void RegisterApplication (void) {
	REGISTER_APPLICATION(mic::applications::HistogramFilterMazeLocalization);
}

} /* namespace application */

namespace applications {

HistogramFilterMazeLocalization::HistogramFilterMazeLocalization(std::string node_name_) : OpenGLApplication(node_name_),
		hidden_maze_number("hidden_maze", 0),
		hidden_x("hidden_x", 0),
		hidden_y("hidden_y", 0),
		action("action", -1),
		epsilon("epsilon", 0.0),
		hit_factor("hit_factor", 0.6),
		miss_factor("miss_factor", 0.2),
		exact_move_probability("exact_move_probability", 1.0),
		overshoot_move_probability("overshoot_move_probability", 0.0),
		undershoot_move_probability("undershoot_move_probability", 0.0)

	{
	// Register properties - so their values can be overridden (read from the configuration file).
	registerProperty(hidden_maze_number);
	registerProperty(hidden_x);
	registerProperty(hidden_y);
	registerProperty(action);
	registerProperty(epsilon);

	registerProperty(hit_factor);
	registerProperty(miss_factor);
	registerProperty(exact_move_probability);
	registerProperty(overshoot_move_probability);
	registerProperty(undershoot_move_probability);

	LOG(LINFO) << "Properties registered";
}


HistogramFilterMazeLocalization::~HistogramFilterMazeLocalization() {

}


void HistogramFilterMazeLocalization::initialize(int argc, char* argv[]) {
	LOG(LSTATUS) << "In here you should initialize Glut and create all OpenGL windows";

	// Initialize GLUT! :]
	VGL_MANAGER->initializeGLUT(argc, argv);

	// Create the visualization windows - must be created in the same, main thread :]
	w_current_maze_chart = new WindowChart("Current_maze", 256, 256, 0, 0);
	w_current_coordinate_x = new WindowChart("Current_x", 256, 256, 0, 326);
	w_current_coordinate_y = new WindowChart("Current_y", 256, 256, 326, 326);


}

void HistogramFilterMazeLocalization::initializePropertyDependentVariables() {

	// Import mazes.
	if ((!importer.importData()) || (importer.getData().size() == 0)){
		LOG(LERROR) << "The dataset must consists of at least one maze!";
		exit(0);
	}//: if

	hf = new mic::algorithms::MazeHistogramFilter(importer.getData(), hidden_maze_number, hidden_x, hidden_y);

	// Show mazes.
	LOG(LNOTICE) << "Loaded mazes";
	for (size_t m=0; m<importer.getData().size(); m++) {
		// Display results.
		LOG(LNOTICE) << "maze(" <<m<<"):\n" << (importer.getData()[m]);
	}//: for

	// Assign initial probabilities to all variables (uniform distribution).s
	hf->assignInitialProbabilities();

	// Create data containers - for visualization.
	createDataContainers();

	// Store the "zero" state.
	storeCurrentStateInDataContainers(true);

	LOG(LWARNING) << "Hidden position in maze " << hidden_maze_number << "= (" << hidden_y << "," << hidden_x << ")";

	// Get first observation.
	hf->sense(hit_factor, miss_factor);

	// Update aggregated probabilities.
	hf->updateAggregatedProbabilities();

	// Store the first state.
	storeCurrentStateInDataContainers(true);

}



void HistogramFilterMazeLocalization::createDataContainers() {
	// Random device used for generation of colors.
	std::random_device rd;
	std::mt19937_64 rng_mt19937_64(rd());
	// Initialize uniform index distribution - integers.
	std::uniform_int_distribution<> color_dist(50, 200);
	// Create a single container for each maze.
	for (size_t m=0; m<importer.getData().size(); m++) {
		std::string label = "P(m" + std::to_string(m) +")";
		int r= color_dist(rng_mt19937_64);
		int g= color_dist(rng_mt19937_64);
		int b= color_dist(rng_mt19937_64);
		//std::cout << label << " g=" << r<< " g=" << r<< " b=" << b;

		// Add container to chart.
		w_current_maze_chart->createDataContainer(label, mic::types::color_rgba(r, g, b, 180));

	}//: for


	// Create a single container for each x coordinate.
	for (size_t x=0; x<importer.maze_width; x++) {
		std::string label = "P(x" + std::to_string(x) +")";
		int r= color_dist(rng_mt19937_64);
		int g= color_dist(rng_mt19937_64);
		int b= color_dist(rng_mt19937_64);

		// Add container to chart.
		w_current_coordinate_x->createDataContainer(label, mic::types::color_rgba(r, g, b, 180));

	}//: for


	// Create a single container for each y coordinate.
	for (size_t y=0; y<importer.maze_height; y++) {
		std::string label = "P(y" + std::to_string(y) +")";
		int r= color_dist(rng_mt19937_64);
		int g= color_dist(rng_mt19937_64);
		int b= color_dist(rng_mt19937_64);

		// Add container to chart.
		w_current_coordinate_y->createDataContainer(label, mic::types::color_rgba(r, g, b, 180));
	}//: for

}

void HistogramFilterMazeLocalization::storeCurrentStateInDataContainers(bool synchronize_) {

	if (synchronize_)
	{ // Enter critical section - with the use of scoped lock from AppState!
		APP_DATA_SYNCHRONIZATION_SCOPED_LOCK();

		// Add data to chart windows.
		for (size_t m=0; m<importer.getData().size(); m++) {
			std::string label = "P(m" + std::to_string(m) +")";
			w_current_maze_chart->addDataToContainer(label, hf->maze_probabilities[m]);
		}//: for

		for (size_t x=0; x<importer.maze_width; x++) {
			std::string label = "P(x" + std::to_string(x) +")";
			w_current_coordinate_x->addDataToContainer(label, hf->maze_x_coordinate_probilities[x]);
		}//: for

		for (size_t y=0; y<importer.maze_height; y++) {
			std::string label = "P(y" + std::to_string(y) +")";
			w_current_coordinate_y->addDataToContainer(label, hf->maze_y_coordinate_probilities[y]);
		}//: for

	}//: end of critical section.
	else {
		// Add data to chart windows.
		for (size_t m=0; m<importer.getData().size(); m++) {
			std::string label = "P(m" + std::to_string(m) +")";
			w_current_maze_chart->addDataToContainer(label, hf->maze_probabilities[m]);
		}//: for

		for (size_t x=0; x<importer.maze_width; x++) {
			std::string label = "P(x" + std::to_string(x) +")";
			w_current_coordinate_x->addDataToContainer(label, hf->maze_x_coordinate_probilities[x]);
		}//: for

		for (size_t y=0; y<importer.maze_height; y++) {
			std::string label = "P(y" + std::to_string(y) +")";
			w_current_coordinate_y->addDataToContainer(label, hf->maze_y_coordinate_probilities[y]);
		}//: for

	}//: else
}



bool HistogramFilterMazeLocalization::performSingleStep() {
	LOG(LINFO) << "Performing a single step ";

	short tmp_action = action;

	// Check epsilon-greedy action selection.
	if ((double)epsilon > 0) {
		if (RAN_GEN->uniRandReal() < (double)epsilon)
				tmp_action = -3;
	}//: if

	// Determine action.
	mic::types::Action2DInterface act;
	switch(tmp_action){
	case (short)-3:
			act = A_RANDOM; break;
	case (short)-2:
			act = hf->sumOfMostUniquePatchesActionSelection(); break;
	case (short)-1:
			act = hf->mostUniquePatchActionSelection(); break;
	default:
		act = mic::types::NESWAction((mic::types::NESW_action_type_t) (short)tmp_action);
	}//: switch action

	// Perform move.
	hf->probabilisticMove(act, exact_move_probability, overshoot_move_probability, undershoot_move_probability);


	// Get current observation.
	hf->sense(hit_factor, miss_factor);

	// Update state.
	hf->updateAggregatedProbabilities();

	// Store collected data for visualization/export.
	storeCurrentStateInDataContainers(false);

	return true;
}






} /* namespace applications */
} /* namespace mic */
