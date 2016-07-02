#ifndef _StretchController_h_
#define _StretchController_h_
/* -------------------------------------------------------------------------- *
 *                   OpenSim:  StretchController.h                    *
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

using namespace std;
namespace OpenSim {

//------------------------------------------------------------------------------
// StretchController
//------------------------------------------------------------------------------
class StretchController : public Controller {
	OpenSim_DECLARE_CONCRETE_OBJECT(StretchController, Controller);

public:
	// Property of the controller for gain on the error signal
	OpenSim_DECLARE_PROPERTY(length_gain, double,
		"Gain used to convert length error to a control signal");

	// TODO threshold

	// Connector to the ScalarActuator for which the controller is computing a
	// control signal.
	OpenSim_DECLARE_CONNECTOR(actuator, ScalarActuator,
		"The actuator for which the controller is computing a control signal");

	// Input the muscle fiber length to use in the stretch controller
	OpenSim_DECLARE_INPUT(fiberLength, double, SimTK::Stage::Model,
		"The input length that is compared to the set point");

	// Input the set point for the muscle fiber length 
	OpenSim_DECLARE_INPUT(fiberLength_setpoint, double, SimTK::Stage::Model,
		"The set point on fiber length");

	// Output the control signal generated by the controller.
	OpenSim_DECLARE_OUTPUT(stretch_control, double, computeControl,
		SimTK::Stage::Time);

	StretchController() {
		constructProperties();
	}

	// Member function for computing the proportional control signal k*a.
	double computeControl(const SimTK::State& s) const
	{
		double fiberLength = getInputValue<double>(s, "fiberLength");
		double fiberLength_setPoint = getInputValue<double>(s,
			"fiberLength_setpoint");
		cout << "length and lengthST: " << fiberLength << "  " << fiberLength_setPoint << endl;
		cout << "gain: " << get_length_gain() << endl;
		//TODO: Design a control strategy that improves jump height.
		//return 0;
		double lengthError = fiberLength - fiberLength_setPoint;
		//return (error < threshold) ? 0. : get_gain() * error;
		cout << "control for muscle: " << get_length_gain() * lengthError << endl;
		return get_length_gain() * lengthError;

	}

	// Member function for adding the control signal computed above into the
	// actuator's Vector of controls.
	void computeControls(const SimTK::State& s,
		SimTK::Vector& controls) const override
	{
		double signal = computeControl(s);
		const auto& actuator = getConnectee<ScalarActuator>("actuator");
		SimTK::Vector thisActuatorsControls(1, signal);
		actuator.addInControls(thisActuatorsControls, controls);
	}

private:
	void constructProperties() {
		constructProperty_length_gain(1.0);
	}

}; // end of StretchController


} // end of namespace OpenSim

#endif // _StretchController_h_
