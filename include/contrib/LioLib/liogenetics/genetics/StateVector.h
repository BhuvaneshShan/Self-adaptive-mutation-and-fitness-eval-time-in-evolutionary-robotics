/*
 * $Id: StateVector.h 1338 2009-07-31 14:33:01Z evert $
 */
#ifndef STATEVECTORH
#define STATEVECTORH

#include <genetics/GeneticConfig.h>
#include <lioutils/Fixed32.h>

// Not used, for now
class StateVector
{
public:
	typedef Fixed32 StateType;

	StateVector(void);
	~StateVector(void);

	void set(StateID id, StateType value);
	StateType get(StateID id) const;
	StateType operator[](StateID id)
	{
		return get(id);
	}

	static StateVector* getRobotState(void);

private:
	StateType vector_[STATECOUNT];

	static StateVector robotState;
};

#endif // STATEVECTORH
