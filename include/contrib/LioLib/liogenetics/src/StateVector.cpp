/*
 * $Id: StateVector.cpp 1338 2009-07-31 14:33:01Z evert $
 */
#include "genetics/StateVector.h"

#include "stdlib.h"

StateVector StateVector::robotState;

StateVector::StateVector(void)
{

}

StateVector::~StateVector(void)
{

}

void StateVector::set(StateID id, StateVector::StateType value)
{
	if (id < STATECOUNT)
		vector_[id] = value;
}

StateVector::StateType StateVector::get(StateID id) const
{
	if (id < STATECOUNT)
		return vector_[id];
	else
		return 0;
}

StateVector* StateVector::getRobotState(void)
{
	return &robotState;
}
