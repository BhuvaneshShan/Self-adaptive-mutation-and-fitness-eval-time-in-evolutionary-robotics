#include "ahhs/AHHSConfiguration.h"
#include "RoboroboMain/roborobo.h"
#include "AHHSScalePower/include/AHHSScalePowerSharedData.h"

/**********************************************************//**
 * Meth AHHSConfiguration: default constructor
 *
 * Default constructor. Builds a robot model instance and sets default
 * values
 *
 * @param
 *
 * @author Heiko Hamann, Jürgen Stradner, Thomas Schmickl
 *
 * @date 20091112
 **************************************************************/

AHHSConfiguration::AHHSConfiguration() {
	//setConfiguration();
}

AHHSConfiguration::AHHSConfiguration(AHHSConfiguration *src) {
	number_of_compartments_ = src->number_of_compartments_;
	for (int i = 0; i < src->getNumberOfActuators(); i++) {
		addActuator(new Actuator(src->getActuator(i)));
	}
	for (int i = 0; i < src->getNumberOfSensors(); i++) {
		addSensor(new Sensor(src->getSensor(i)));
	}
	//actuators_ = src->actuators_;
	//sensors_ = src->sensors_;
}

AHHSConfiguration::~AHHSConfiguration() {
	//cout << "AHHSConfiguration::~AHHSConfiguration(): dead" << endl;
	if (!sensors_.empty()) {
		for (int i = 0; i < getNumberOfSensors(); i++) {
			if (sensors_[i] != 0) {
				delete sensors_[i];
				sensors_[i] = 0;
			}
		}
		sensors_.clear();
	}
	//cout << "AHHSConfiguration::~AHHSConfiguration(). Before actu clear." << endl;
	if (!actuators_.empty()) {
		for (int i = 0; i < getNumberOfActuators(); i++) {
			//cout << "AHHSConfiguration::~AHHSConfiguration(). inside: " << i << endl;
			if (actuators_[i] != 0) {
				delete actuators_[i];
			}
		}
		actuators_.clear();
	}
	//cout << "AHHSConfiguration::~AHHSConfiguration(). After clear." << endl;
}

void AHHSConfiguration::addSensor(Sensor *new_sensor) {
	sensors_.push_back(new_sensor);
}

void AHHSConfiguration::addActuator(Actuator *new_actuator) {
	actuators_.push_back(new_actuator);
}

int AHHSConfiguration::getNumberOfSensors() {
	return (int) sensors_.size();
}

int AHHSConfiguration::getNumberOfActuators() {
	return (int) actuators_.size();
}

int AHHSConfiguration::getNumberOfCompartments() {
	return number_of_compartments_;
}

Actuator* AHHSConfiguration::getActuator(int pos) {
	if (pos >= (int) actuators_.size()) {
		cout
				<< "AHHSRobotConfiguration::getActuator(). Error: Requested Actuator does not exist. Requested Actuator: "
				<< pos << ", but there are only " << actuators_.size()
				<< " actuators registered (0-(n-1)). Exit" << endl;
	}
	return actuators_[pos];
}

Sensor* AHHSConfiguration::getSensor(int pos) {
	if (pos >= (int) sensors_.size()) {
		cout
				<< "AHHSRobotConfiguration::getSensor(). Error: Requested Sensor does not exist. Requested sensor: "
				<< pos << ", but there are only " << actuators_.size()
				<< " sensors registered (0-(n-1)). Exit" << endl;
	}
	return sensors_[pos];
}

vector<Actuator*> AHHSConfiguration::getActuators() {
	return actuators_;
}

vector<Sensor*> AHHSConfiguration::getSensors() {
	return sensors_;
}

vector<int> AHHSConfiguration::getNeighborCompIDs(int index) {
	vector<int> neighbor_ids;
	for (int i = 0; i < number_of_compartments_; i++) {
		if (i != index) {
			neighbor_ids.push_back(i);
		}
	}
	return neighbor_ids;
}

void AHHSConfiguration::setNrOfCompartments(int nr_of_compartments) {
	number_of_compartments_ = nr_of_compartments;
}

/**********************************************************//**
 * Meth setConfiguration: sets configuration
 *
 * Build the AHHS system into virtual configuration
 *
 * @param
 * @author Jürgen Stradner
 * @date 20091112
 * @todo Senors setting implemented only for 2 compartments
 * @todo Actuator setting: every actor in its own compartment
 **************************************************************/
void AHHSConfiguration::setConfiguration(int nr_of_actuators,
		int nr_of_sensors, int nr_of_compartments) {
	double sensor_arr[11] = { 270.0 * PI / 180.0, 90.0 * PI / 180.0, 180.0 * PI
			/ 180.0, 345.0 * PI / 180.0, 310.0 * PI / 180.0,
			270.0 * PI / 180.0, 210.0 * PI / 180.0, 150.0 * PI / 180.0, 90.0
					* PI / 180.0, 50.0 * PI / 180.0, 15.0 * PI / 180.0 };

	number_of_compartments_ = nr_of_compartments;

	for (int i = 0; i < nr_of_sensors; i++) {
		Sensor *my_sensor = new Sensor();
		my_sensor->setSensorValue(0.0);
		// ATTENTION: Implemented only for 2 compartments
		if (sensor_arr[i] > PI)
			my_sensor->setCompartmentID(0);
		else
			my_sensor->setCompartmentID(1);
		my_sensor->setMinValue(0.0);
		my_sensor->setMaxValue(1.0);
		addSensor(my_sensor);
	}

	for (int i = 0; i < nr_of_actuators; i++) {
		Actuator *Motor = new Actuator();
		Motor->setActuatorValue(0.0);
		Motor->setCompartmentID(i);
		//Motor->setCompartmentID(1);
		Motor->setMinValue(-1.0);
		Motor->setMaxValue(1.0);
		addActuator(Motor);
	}

	//cout << "AHHSConfiguration::setConfiguration(int, int, int). done. Act: " << actuators_.size() << " sens: " << sensors_.size() << " comp: " << number_of_compartments_ << endl;
}

void AHHSConfiguration::setCustomConfiguration(int nr_of_actuators,
		int nr_of_sensors, int nr_of_compartments, double sensor_arr[]) {
	number_of_compartments_ = nr_of_compartments;

	// Add the distance sensors
	for (int i = 0; i < 8; i++) {
		Sensor *distanceSensor = new Sensor();
		distanceSensor->setSensorValue(0.0);
		//distanceSensor->setCompartmentID(0);
		if (sensor_arr[i] > PI)
			distanceSensor->setCompartmentID(0);
		else
			distanceSensor->setCompartmentID(0);
		distanceSensor->setMinValue(0.0);
		distanceSensor->setMaxValue(1.0);
		addSensor(distanceSensor);
	}
	Sensor *extraSensor;

	// Add the sensor for organism
	extraSensor = new Sensor();
	extraSensor->setSensorValue(0.0);
	extraSensor->setCompartmentID(0);
	extraSensor->setMinValue(0.0);
	extraSensor->setMaxValue(1.0);
	addSensor(extraSensor);

	// Add sensor for feeding ground angle
	extraSensor = new Sensor();
	extraSensor->setSensorValue(0.0);
	extraSensor->setCompartmentID(0);
	extraSensor->setMinValue(-1.0);
	extraSensor->setMaxValue(1.0);
	addSensor(extraSensor);

	// Add sensor for feeding ground distance
	extraSensor = new Sensor();
	extraSensor->setSensorValue(0.0);
	extraSensor->setCompartmentID(0);
	extraSensor->setMinValue(0.0);
	extraSensor->setMaxValue(1.0);
	addSensor(extraSensor);

	// Add sensor for energy level
	extraSensor = new Sensor();
	extraSensor->setSensorValue(0.0);
	extraSensor->setCompartmentID(0);
	extraSensor->setMinValue(0.0);
	extraSensor->setMaxValue(1.0);
	addSensor(extraSensor);


	// Add actuators for the four strategies
	for (int i = 0; i < 4; i++) {
		Actuator *strategy = new Actuator();
		strategy->setActuatorValue(0.0);
		strategy->setCompartmentID(0);
		strategy->setMinValue(0.0);
		strategy->setMaxValue(1.0);
		addActuator(strategy);
	}

	for (int i=0;i<3; i++){
		// Add actuators for the magnet
		Actuator *magnet = new Actuator();
		magnet->setActuatorValue(0.0);
		magnet->setCompartmentID(0);
		magnet->setMinValue(0.0);
		magnet->setMaxValue(1.0);
		addActuator(magnet);
	}
}

