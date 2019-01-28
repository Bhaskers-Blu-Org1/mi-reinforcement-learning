/*!
 * \file HistogramFilterMazeLocalization.hpp
 * \brief Declaration of a class being a histogram filter based maze localization application.
 * \author tkornut
 * \date Jan 27, 2016
 */

#ifndef SRC_APPLICATION_HISTOGRAMFILTERMAZELOCALIZATION_HPP_
#define SRC_APPLICATION_HISTOGRAMFILTERMAZELOCALIZATION_HPP_

#include <types/MatrixTypes.hpp>

#include <data_io/MazeMatrixImporter.hpp>

#include <algorithms/MazeHistogramFilter.hpp>

#include <opengl/application/OpenGLApplication.hpp>
#include <opengl/visualization/WindowChart.hpp>
#include <opengl/visualization/WindowFloatCollectorChart.hpp>

#include <types/Action2D.hpp>
using namespace mic::opengl::visualization;



namespace mic {
namespace application {

/*!
 * \brief Class implementing a histogram filter based solution of the maze-of-digits problem.
 * \author tkornuta
 */
class HistogramFilterMazeLocalization: public mic::opengl::application::OpenGLApplication {
public:
	/*!
	 * Default Constructor. Sets the application/node name, default values of variables, initializes classifier etc.
	 * @param node_name_ Name of the application/node (in configuration file).
	 */
	HistogramFilterMazeLocalization(std::string node_name_ = "application");

	/*!
	 * Destructor.
	 */
	virtual ~HistogramFilterMazeLocalization();

protected:
	/*!
	 * Initializes all variables that are property-dependent (input patches, SDRs etc.).
	 */
	virtual void initializePropertyDependentVariables();

	/*!
	 * Method initializes GLUT and OpenGL windows.
	 * @param argc Number of application parameters.
	 * @param argv Array of application parameters.
	 */
	virtual void initialize(int argc, char* argv[]);

	/*!
	 * Performs single step of computations.
	 */
	virtual bool performSingleStep();

	/*!
	 * Creates data containers - for visualization/data export purposes.
	 */
	void createDataContainers();

	/*!
	 * Stores current state in data containers.
	 * @param synchronize_ If true enters critical section when adding data to containers.
	 */
	void storeCurrentStateInDataContainers(bool synchronize_);

private:

	/// Window for displaying chart with statistics on current maze number.
	WindowChart* w_current_maze_chart;

	/// Window for displaying chart with statistics on current x coordinate.
	WindowChart* w_current_coordinate_x;

	/// Window for displaying chart with statistics on current y coordinate.
	WindowChart* w_current_coordinate_y;

	/// Data collector with maximal maze/x/y/ probabilities.
	mic::data_io::DataCollectorPtr<std::string, float> max_probabilities_collector_ptr;

	/// Window for displaying chart with maximal maze/x/y/ probabilities.
	WindowFloatCollectorChart * w_max_probabilities_chart;

	/// Importer responsible for loading mazes from file.
	mic::data_io::MazeMatrixImporter importer;

	/// Histogram filter.
	mic::algorithms::MazeHistogramFilter hf;

	/// Property: variable denoting in which maze are we right now (unknown, to be determined).
	mic::configuration::Property<short> hidden_maze_number;

	/// Property: variable denoting the x position are we right now (unknown, to be determined).
	mic::configuration::Property<short> hidden_x;

	/// Property: variable denoting the y position are we right now (unknown, to be determined).
	mic::configuration::Property<short> hidden_y;


	/// Property: performed action (0-3: NESW, -3: random, -2: sumOfMostUniquePatchesActionSelection, -1: mostUniquePatchActionSelection).
	mic::configuration::Property<short> action;

	/// Property: variable denoting epsilon in aciton selection (the probability "below" which a random action will be selected).
	mic::configuration::Property<double> epsilon;

	/// Property: variable denoting the hit factor (the gain when the observation coincides with current position).
	mic::configuration::Property<double> hit_factor;

	/// Property: variable denoting the miss factor (the gain when the observation does not coincide with current position).
	mic::configuration::Property<double> miss_factor;

	/// Property: variable storing the probability that we made the exact move (x+dx).
	mic::configuration::Property<double> exact_move_probability;

	/// Property: variable storing the probability that we made the "overshoot" move (d+dx+1).
	mic::configuration::Property<double> overshoot_move_probability;

	/// Property: variable storing the probability that we made the "undershoot" move (d+dx-1).
	mic::configuration::Property<double> undershoot_move_probability;

	/// Property: name of the file to which the statistics will be exported.
	mic::configuration::Property<std::string> statistics_filename;
};

} /* namespace application */
} /* namespace mic */

#endif /* SRC_APPLICATION_HISTOGRAMFILTERMAZELOCALIZATION_HPP_ */
