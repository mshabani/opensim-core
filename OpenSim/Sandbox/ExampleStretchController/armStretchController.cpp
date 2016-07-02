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

static const double SIGNAL_GEN_CONSTANT{ 0.4 };
static const double REPORTING_INTERVAL{ 0.2 };
static const double LENGTH_GAIN = 10000.0;
static const std::string testbedAttachment1{"ground"};
static const std::string testbedAttachment2{"load"};
using namespace std;
namespace OpenSim {

// Forward declarations for methods used below.
Model buildTestbed();   //defined in defineStretchController.h

class ControllerHolder : public ModelComponent {
	OpenSim_DECLARE_CONCRETE_OBJECT(ControllerHolder, ModelComponent);
public:
	
	ControllerHolder() {
		//this->setName("controllerHolder");
	}
	
	/*void addControllerToMuscle(Muscle& muscle, double lengthSetPoint, double lengthGain) {
		
		auto controller = new StretchController();
		controller->setName(muscle.getName() + "_stretchController");
		controller->set_length_gain(lengthGain);
		//Muscle& mus = muscles[i];
		//controller->updConnector("actuator").connect(mus);
		holder->addComponent(controller);
	}*/
protected:
	/*void extendFinalizeFromProperties() override {
		Super::extendFinalizeFromProperties();
		//TODO add channels 
		cout << "extend called" << endl;

		auto& outputs = updOutput("stretch_control");
		cout << "extend 1" << endl;
		
		for (int i = 0; i < 6; i++) {
			//cout << "chan name" << actuators[i].getName() << endl;// Name is Empty
			string name = ""; //actuators[i].getName();
			if (false) {
				//outputs.addChannel(actuators[i].getName());
				//cout << "added " << name << " to output" << endl;
			}
			else {
				try {
					name = "muscle_" + to_string(i);
					outputs.addChannel(name);
					cout << "added " << name << " to output" << endl;
				}
				catch (Exception& e) {
					cout << e.what() << endl;
				}
			}

		}
		cout << "extend ended" << endl;
	}

	void extendAddToSystem(SimTK::MultibodySystem& sys) const override {
		Super::extendAddToSystem(sys);
	}*/

	/*void extendConnectToModel(OpenSim::Model& model) {
		OPENSIM_THROW_IF_FRMOBJ(
			!dynamic_cast<const ControllerHolder*>(&getParent()),
			Exception, getName() + ",This compoment must be a subcomponent of "
			"ControllerHolder.");

	}*/
	
};
//------------------------------------------------------------------------------
// Build the StretchController.
//------------------------------------------------------------------------------

Controller* buildStretchController(const Muscle& m) {
	try {
		cout << "building controller for muscle " << m.getName() << endl;
		auto controller = new StretchController();
		controller->setName("stretchCon_"+m.getName());
		controller->set_length_gain(LENGTH_GAIN);
		
		controller->updInput("fiberLength").connect(m.getOutput("fiber_length"));  
		controller->updConnector("actuator").connect(m); 
		return controller;
	}
	catch (Exception e) {
		cout << e.what() << endl;
	}

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
void addDeviceConsoleReporterToModel(Model& model, Component& holder,
    const std::vector<std::string>& deviceOutputs)
{
    // Create a new ConsoleReporter. Set its name and reporting interval.
    auto reporter = new ConsoleReporter();
    reporter->setName(model.getName() + "_" + holder.getName() + "_results");
    reporter->set_report_time_interval(REPORTING_INTERVAL);

    // Loop through the desired device outputs and add them to the reporter.
	for (auto thisOutputName : deviceOutputs) {
		reporter->updInput("inputs").connect(holder.getOutput(thisOutputName));
		cout << "added" << endl;
	}

    // Add the reporter to the model.
    model.addComponent(reporter);
}

ControllerHolder* buildHolder(Model& model) {

	auto holder = new ControllerHolder();
	holder->setName("StretchConHolder");

	auto lengthSignalGen = new SignalGenerator();
	lengthSignalGen->setName("lengthSetPointGen");

	lengthSignalGen->set_function(Constant(SIGNAL_GEN_CONSTANT));
	holder->addComponent(lengthSignalGen);

	auto& muscles = model.getMuscles();
	for (int i = 0; i < muscles.getSize(); i++) {
		// Create a StretchController
		auto controller = new StretchController();
		controller->setName(muscles[i].getName() + "_controller");
		controller->set_length_gain(SIGNAL_GEN_CONSTANT);
		Muscle& mus = muscles[i];
		//controller->updConnector("actuator").connect(mus);
		holder->addComponent(controller);
	}


	return holder;
}
} // namespace OpenSim


//------------------------------------------------------------------------------
// START HERE! Toggle "if (false)" to "if (true)" to enable/disable each step in
// the exercise. The project should execute without making any changes (you
// should see the unassisted hopper hop slightly).
//------------------------------------------------------------------------------


using namespace OpenSim;

int main() {
	
	Model& model = Model("arm26.osim");
	auto holder = buildHolder(model);
	auto& muscles = model.getMuscles();
	model.setUseVisualizer(true);
	model.addComponent(holder);

	//showSubcomponentInfo(*holder);
	//showAllOutputs(*holder, true);

	try {
		for (int i = 0; i < muscles.getSize(); i++) {
			
			string controllerPath = "/" + holder->getName() + "/" + muscles[i].getName() + "_controller"; 
			holder->updConnector(controllerPath+"/actuator").connect(muscles[i]);
			
			holder->updInput(controllerPath+"/fiberLength")
				.connect(muscles[i].getOutput("fiber_length"));

			holder->updInput(controllerPath+"/fiberLength_setpoint")
				.connect(holder->getComponent("lengthSetPointGen").getOutput("signal"));

			/*auto& component = model.updComponent(pathname);
			//showSubcomponentInfo(component);
			
			component.updConnector("actuator").connect(muscles[i]);
			component.updInput("fiberLength").connect(muscles[i].getOutput("fiber_length"));
			component.updInput("fiberLength_setpoint").
				connect(holder->getComponent("lengthSetPointGen").getOutput("signal"));
			*/
		}
	}
	catch (Exception& e) {
		cout << e.what() << endl;
	}
	
	//std::vector<std::string> controllerOutputs{ "stretch_control" };
	//addDeviceConsoleReporterToModel(model, *holder, controllerOutputs);
	
	SimTK::State& sDev = model.initSystem();
	simulate(model, sDev);
	
	system("pause");
	return 0;
}

