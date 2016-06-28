/* -------------------------------------------------------------------------- *
 *                     OpenSim:  exampleHopperDevice.cpp                      *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2016 Stanford University and the Authors                *
 * Author(s): Chris Dembia, Shrinidhi K. Lakshmikanth, Ajay Seth,             *
 *            Thomas Uchida                                                   *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */


#include <OpenSim/OpenSim.h>
#include "helperMethods.h"
#include "StretchController.h"

static const double SIGNAL_GEN_CONSTANT{ 0.33 };
static const double REPORTING_INTERVAL{ 0.2 };
static const double LENGTH_GAIN = 1.0;
static const std::string testbedAttachment1{"ground"};
static const std::string testbedAttachment2{"load"};
using namespace std;
namespace OpenSim {

// Forward declarations for methods used below.
Model buildTestbed();   //defined in defineStretchController.h

//------------------------------------------------------------------------------
// Build the StretchController.
//------------------------------------------------------------------------------

Controller* buildStretchController(PathActuator& pa) {
	try {
		cout << "building controller" << endl;
		auto controller = new StretchController();
		controller->setName("stretchCon");
		controller->set_length_gain(LENGTH_GAIN);

		Muscle* mus = dynamic_cast <Muscle*> (&pa);
		controller->updInput("fiberLength").connect(mus->getOutput("fiber_length"));   // TODO connect this to what?
		controller->updConnector("actuator").connect(pa); // TODO need pointer to muscle!
		return controller;
	}
	catch (Exception e) {
		cout << e.what() << endl;
	}
}


//------------------------------------------------------------------------------
// Attach the stretch controller to a model
//------------------------------------------------------------------------------
void connectControllerToModel(Controller& controller, Model& model)
{
	model.addController(&controller);
}

//------------------------------------------------------------------------------
// Add a SignalGenerator to a StretchController.
//------------------------------------------------------------------------------
void addSignalGeneratorToController(Controller& controller)
{
    auto lengthSignalGen = new SignalGenerator();
	lengthSignalGen->setName("lengthSetPointGen");

    // Try changing the constant value and/or the function (e.g., try a
    // LinearFunction).
	lengthSignalGen->set_function(Constant(SIGNAL_GEN_CONSTANT));
	controller.addComponent(lengthSignalGen);
	
	// Connect the signal generator's output signal to the controller's
    // setpoint 
	controller.updInput("fiberLength_setpoint")
        .connect(lengthSignalGen->getOutput("signal"));
} 


//------------------------------------------------------------------------------
// Add a ConsoleReporter to a model for displaying outputs from a device.
//------------------------------------------------------------------------------
void addDeviceConsoleReporterToModel(Model& model, Controller& controller,
    const std::vector<std::string>& deviceOutputs)
{
    // Create a new ConsoleReporter. Set its name and reporting interval.
    auto reporter = new ConsoleReporter();
    reporter->setName(model.getName() + "_" + controller.getName() + "_results");
    reporter->set_report_time_interval(REPORTING_INTERVAL);

    // Loop through the desired device outputs and add them to the reporter.
    for (auto thisOutputName : deviceOutputs)
        reporter->updInput("inputs").connect(controller.getOutput(thisOutputName));

    // Add the reporter to the model.
    model.addComponent(reporter);
}

} // namespace OpenSim


//------------------------------------------------------------------------------
// START HERE! Toggle "if (false)" to "if (true)" to enable/disable each step in
// the exercise. The project should execute without making any changes (you
// should see the unassisted hopper hop slightly).
//------------------------------------------------------------------------------
int main()
{
	try {
    using namespace OpenSim;
	using namespace std;
    //==========================================================================
    // Step 1. Build an stretch controller and test it on a simple testbed.
    //==========================================================================
    if (true){
		// Build the testbed and controller.
		cout << "starting" << endl;
		auto& testbed = buildTestbed();
		cout << "model created" << endl;
		
		auto& pathActuator = testbed.updComponent<PathActuator>("muscle");
		auto controller = buildStretchController(pathActuator);
		

		cout << "model and controller created" << endl;
        
		// Connect the controller to the testbed. 
		connectControllerToModel(*controller, testbed);

		cout << "connected" << endl;
		// Use a SignalGenerator to create a set point signal for testing the
		// controller. 
		addSignalGeneratorToController(*controller);
		cout << "generator added" << endl;

		// Show all Components in the testbed.
		showSubcomponentInfo(testbed);
		showSubcomponentInfo(*controller);


		cout << "sub info are shown" << endl;

		// List the device outputs we wish to display during the simulation.
		std::vector<std::string> controllerOutputs{ "stretch_control"};

		// Add a ConsoleReporter to report deviceOutputs.
		addDeviceConsoleReporterToModel(testbed, *controller, controllerOutputs);
		cout << "before init" << endl;
		// Create the system, initialize the state, and simulate.
		

		SimTK::State& sDev = testbed.initSystem();
		cout << "before sim" << endl;
		OpenSim::simulate(testbed, sDev);
    }
	//==========================================================================
	// Step 2. Build an stretch controller for arm model.
	//==========================================================================
	if (false){
		
		auto& model = Model("arm26.osim");
		model.setUseVisualizer(true);
		model.setGravity(SimTK::Vec3(0));
		
		Set<Muscle> muscles = model.getMuscles();
		for (int i = 0; i < muscles.getSize(); i++) {
			auto& pathActuator = model.updComponent<PathActuator>(muscles[i].getName());
			auto controller = buildStretchController(pathActuator);
			
			// Connect the controller to the testbed.
			connectControllerToModel(*controller, model);
			
			// Use a SignalGenerator to create a set point signal for testing the
			// controller. 
			addSignalGeneratorToController(*controller);

			// Show all Components in the model.
			//showSubcomponentInfo(model);
			//showSubcomponentInfo(*controller);

			// List the device outputs we wish to display during the simulation.
			std::vector<std::string> controllerOutputs{ "stretch_control" };

			// Add a ConsoleReporter to report deviceOutputs.
			addDeviceConsoleReporterToModel(model, *controller, controllerOutputs);

		}
		
		// Create the system, initialize the state, and simulate.
		SimTK::State& sDev = model.initSystem();
		
		
		OpenSim::simulate(model, sDev);
	}

}
catch (const std::exception& e) {
	std::cout << e.what() << std::endl;
}
system("pause");
return 0;

};
