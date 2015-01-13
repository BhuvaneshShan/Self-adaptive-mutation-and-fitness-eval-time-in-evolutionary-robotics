/*
 *  EvolutionaryControlArchitecture.cpp
 *  roborobo
 *
 *  Created by Robert-Jan Huijsman on 06-10-10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "rjh/EvolutionaryControlArchitecture.h"

int EvolutionaryControlArchitecture::stepsPerCandidate = 500; // should be overwritten by properties file
EvolutionaryControlArchitecture::CalculationMethod EvolutionaryControlArchitecture::fitnessCalculation = AVERAGE;

EvolutionaryControlArchitecture::EvolutionaryControlArchitecture( RobotAgentWorldModel *__wm ) : DifferentialControlArchitecture ( __wm )
{
	this->phase = EVALUATING;
	this->stepsDone = -1;
	this->candidateFitness = 0;
	
	Mathlib::setRandSeed(gRandomSeed);
}

EvolutionaryControlArchitecture::~EvolutionaryControlArchitecture()
{
	// nothing to do
	// candidate gets cleaned up elsewhere
}

void EvolutionaryControlArchitecture::step(){
    switch (phase) {
        case EVALUATING:
            this->doEvaluation();
            break;
        case FREEING_RANDOM:
            this->doFreeRandom();
            break;
        case FREEING_MANUAL:
            this->doFreeManual();
            break;
    }
}

void EvolutionaryControlArchitecture::doEvaluation() {
    /* Handle the phase in which a robot controller is being evaluated */
    if (stepsDone == 0) {
        if (debug) { cout << "Started evaluation of " << candidate->ToShortString() << endl; }
        candidate->reset();
    }

    /* Update the statistics about any possible running evaluation */
    if (stepsDone >= 0) {
        // determine the fitness during the past step
        double stepFitness = this->calculateStepFitness();
        
        switch (fitnessCalculation) {
        case AVERAGE:
        case CUMULATIVE:
            candidateFitness += stepFitness;
            break;
        }
        
        // mark the past step as completed
        stepsDone++;
    }

    /* Check if a running evaluation is now completed */
    if (stepsDone == getPhaseLength(phase)) {
        // running evaluation has completed. Tell the subclass about this.
        this->setCandidateFitness(candidate, (fitnessCalculation == AVERAGE) ?
                candidateFitness/stepsDone : candidateFitness);
        
        // signal that a new candidate must be found
        stepsDone = -1;
        
        // make sure that a 'free' phase will be performed before the new evaluation
        phase = FREEING_RANDOM;
    }

    /* Find a new candidate, if required */
    if (stepsDone == -1) { // indicates a new candidate must be found
        // ask the subclass for a new candidate
        candidate = this->getNextCandidate();

        // if the subclass returns NULL, go into the freeing phase again instead of evaluating
		if (candidate == NULL) {
			phase = FREEING_RANDOM;
		} else {
			candidate->setWorldModel(_wm);

			// reset the counters
			this->stepsDone = 0;
			this->candidateFitness = 0;
		}
    } else {
        // let the controller do it's thing
        double left = 0, right = 0;
        candidate->step(left,right);
        this->setSpeed(left,right);
    }
}

void EvolutionaryControlArchitecture::doFreeRandom() {
    if (stepsDone == 0) {
        if (debug) { cout << "Starting random free..." << endl; }
        double left = (Rand::randouble() * 2) - 1;
        double right = (Rand::randouble() * 2) - 1;
        this->setSpeed(left, right);
    }
    
    if (stepsDone == getPhaseLength(phase)) {
        if (debug) { cout << "... random free done." << endl; }
        phase = FREEING_MANUAL;
        stepsDone = 0;
    } else {
        stepsDone++;
    }
}

void EvolutionaryControlArchitecture::doFreeManual() {
    if (stepsDone == 0) {
        if (debug) { cout << "Starting manual free..." << endl; }
    }

    /* See if the free-controller has run for long enough */
    if (stepsDone == getPhaseLength(phase)) {
        // controller is done
        phase = EVALUATING;
        stepsDone = 0;
        if (candidate == NULL) {
            this->stepsDone = -1; // there's no controller for evaluation, find one
        }
        
        if (debug) { cout << "... manual free done." << endl; }
        
    } else {
        this->doBasicObstacleAvoidance();
        
        stepsDone++;
    }
}

void EvolutionaryControlArchitecture::doBasicObstacleAvoidance()
{
    /* goal: move the robot as far away from any walls as possible,
     * or at least out of sensor range */
    double frontSpace = (_wm->_sensors[2][5] + _wm->_sensors[3][5]) / 2.0;
    double leftSpace = (_wm->_sensors[0][5] + _wm->_sensors[1][5] + _wm->_sensors[2][5]) / 3.0;
    double rightSpace = (_wm->_sensors[3][5] + _wm->_sensors[4][5] + _wm->_sensors[5][5]) / 3.0;
    
    _wm->_desiredTranslationalValue = frontSpace / gSensorRange;
    
    double hdg = _wm->_desiredRotationalVelocity;
    
    if (leftSpace < rightSpace) 
        hdg = +5;
    else if (rightSpace < gSensorRange) 
        hdg = -5;
    else if (hdg > 1) 
        hdg -= 1;
    else if (hdg < -1)
        hdg += 1;
    else
        hdg = 0.01 - (double)(rand()%10)/10.*0.02;
    
    _wm->_desiredRotationalVelocity = hdg;
}

int EvolutionaryControlArchitecture::getPhaseLength(unsigned int phase) {
    switch (phase) {
        case EVALUATING:
        	return stepsPerCandidate;
        case FREEING_RANDOM:
            return stepsPerRandomFree;
        case FREEING_MANUAL:
            return stepsPerFree;
        default:
            return 1;
    }
}

ControllerPtr EvolutionaryControlArchitecture::getActiveController() {
	return candidate;
}

double EvolutionaryControlArchitecture::calculateStepFitness() {
	// determine the distance traveled, as a fraction of the maximum
	double travelDistance = abs(_wm->_actualTranslationalValue) / abs(_wm->_maxTranslationalDeltaValue);
	
	// determine the difference in bearing (0 = no change, 1 = complete 360 made)
	double bearingDiff = abs(_wm->_actualRotationalVelocity / 360.0);
	
	double fitness = (travelDistance * (1 - bearingDiff));
	//cout << travelDistance << " * (1 - " << bearingDiff << ") = " << fitness << endl;
	return fitness;
}

void EvolutionaryControlArchitecture::reset(){
	candidate.reset();
}
