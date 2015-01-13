/*
 *  PopulationControlArchitecture.cpp
 *  roborobo
 *
 *  Created by Robert-Jan Huijsman on 15-11-10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#include "newscast/PopulationControlArchitecture.h"

#include "Dynamic/include/DynamicController.h"

unsigned int PopulationControlArchitecture::mu = 3; // TODO: make dynamic
double PopulationControlArchitecture::reEvaluationRate = 0.1; // should be overwritten by properties file
double PopulationControlArchitecture::crossoverRate = 1.0; // should be overwritten by properties file
double PopulationControlArchitecture::mutationRate = 1.0; // should be overwritten by properties file
int PopulationControlArchitecture::numberOfParents = 2; // should be overwritten by properties file

PopulationControlArchitecture::PopulationControlArchitecture(RobotAgentWorldModel *__wm) : EvolutionaryControlArchitecture(__wm) {
	
        currentTime = 0;
	population = NULL;

        //bot_cur_fitness=0;
	//std::cout << "Using a mu of: " << mu <<  " reEvaluationRate: " << reEvaluationRate << std::endl;
}

PopulationControlArchitecture::~PopulationControlArchitecture() {
	delete population;

	activeSolutions.clear(); // destructs the solutions in this vector
}

void PopulationControlArchitecture::setPopulation(PopulationMember *pop) {
	population = pop;
	population->update(currentTime);
}

ControllerPtr PopulationControlArchitecture::getNextCandidate() {
	currentTime++;
	ControllerPtr candidate;

	// if there is no full list of active solutions yet, generate a new candidate randomly
	if (activeSolutions.size() < mu) 
        {
		candidate = this->createRandomGenome();
		if (debug) {
			cout << "Generated " << candidate->ToString() << endl;
		}
               
	}

	// get news from the population, spreading the active solution
	population->update(currentTime);

	// if there is a full list of active solutions, mutate an existing solution or re-evaluate an active to create a new candidate
	if (activeSolutions.size() == mu) 
        {
		// decide whether to re-evaluate or mutate a controller
		if (Rand::randouble() < reEvaluationRate)
                { // re-evaluate
			// pick one of the active solutions for re-evaluation
			candidate = activeSolutions[Rand::randint(0, activeSolutions.size())];
			if (debug) {
				cout << "Selected re-evaluation of " << candidate->ToShortString() << endl;
			}
		} else
                { // mutate existing

			// parent selection: random from the population
			ControllerPtr parentA = selectParentByBinaryTournament(ControllerPtr());
                      //  ControllerPtr parentA = selectParentByBinaryTournamentA();
			if (debug) {
				cout << "Selected first parent: " << parentA->ToShortString() << endl;
			}

			// crossover or clone; crossover only if there is a second active solution in the hivemind
			if (population->getActiveCount() > 1 && Rand::randouble() < crossoverRate) 
                        {
				if (debug) {
					cout << "Selecting second parent, from a choice of " << population->getActiveCount() << endl;
				}
				// crossover
				// select a second parent, excluding the first from the selection
				vector<ControllerPtr> *parents = new vector<ControllerPtr>();
				parents->push_back(parentA);
				while (parents->size()<(unsigned) numberOfParents) 
                                {
                                    
					if (debug) {
						cout << parents->size() << endl;
					}
                                        
                                        
					ControllerPtr tmpC = selectParentsByBinaryTournament(parents);
                                        // ControllerPtr tmpC = selectParentsByBinaryTournamentA(parentA);
                                        //std::cout<<"\n###GOING INTO BIN TOURN N"<<endl;
                                        //ControllerPtr tmpC = selectParentsByBinaryTournamentN(parents);
					parents->push_back(tmpC);
				}
				// create a candidate from crossover between the parents
				candidate = parentA->crossOverWithMP(parents);

				if (debug) {
					cout << " Created " << candidate->ToShortString() << " from crossover between " << parents->at(0)->ToShortString();
					for (uint i = 1; i < parents->size(); i++) cout << " and " << parents->at(i)->ToShortString();
					cout << endl;
				}
                                
				parents->clear();
				delete parents;
			} else {
				// clone
				candidate = parentA->Clone();
				if (debug) {
					cout << "Spawned " << candidate->ToShortString() << " from " << parentA->ToShortString() << endl;
				}
			}

			// mutation
			if (Rand::randouble() < mutationRate) {
				//cout << "Before mutation: " << candidate->ToString() << endl;
                            if(debug)
                            {
                                cout<<"Before Mutation: "<<candidate->ToString()<<endl;
                            }
				candidate->mutate();
				//cout << "After mutation: " << candidate->ToString() << endl;
				if (debug) {
					cout << "After Mutation " << candidate->ToString() << endl;
				}
                               	

	
			}
		}
	}

	return candidate;
}

void PopulationControlArchitecture::setCandidateFitness(ControllerPtr candidate, double scoredFitness) {

	// running evaluation has completed. Decide if the candidate will become the active solution.
	if (candidate->GetFitness() > -1) 
        {
		// this is a re-evaluation, average out the previous and the new score
		if (debug) 
                {
			cout << "This was a re-evaluation, not adding a new candidate" << endl;
		}
                
		candidate->SetFitness((candidate->GetFitness() + scoredFitness) / 2);
               // bot_cur_fitness=(candidate->GetFitness() + scoredFitness) / 2; //we added
		
                if (debug || verbose) 
                {
			cout << "Finished re-evaluating " << candidate->ToShortString() << " - fitness: " << scoredFitness << ", avg: " << candidate->GetFitness() << endl;
		}

	} else 
        {
		candidate->SetFitness(scoredFitness);
                // bot_cur_fitness=scoredFitness;
		if (activeSolutions.size() < mu) 
                {
			activeSolutions.push_back(candidate);

			// send out a news item about the new active solution
                        ControllerPtr active = candidate->Clone();
			population->setNews(active->Clone(), activeSolutions.size() - 1, currentTime);

		} else 
                {
			int worstIndex = -1;
			ControllerPtr worst = candidate;

			for (unsigned int i = 0; i < activeSolutions.size(); i++) 
                        {
				if (worst->GetFitness() > activeSolutions[i]->GetFitness()) 
                                {
					worst = activeSolutions[i];
					worstIndex = i;
			int worstIndex = -1;
			ControllerPtr worst = candidate;	}
			}

			// if the candidate is better than the worst controller in the list, have it take that controller's place
			if (scoredFitness > worst->GetFitness()) {
				if (debug) {
					cout << "This was first evaluation, candidate became active solution" << endl;
				}
				activeSolutions[worstIndex] = candidate;

				// send out a news item about the new active solution
                ControllerPtr active = candidate->Clone();
				population->setNews(active, worstIndex, currentTime);

			} else 
                        {
				if (debug) 
                                {
					cout << "This was first evaluation, candidate failed" << endl;
				}
				// the candidate has lost the competition; have it deleted
				// done automatically by smart pointer
			}
		}
		// Don't print if candidate was deleted
		if (candidate && (debug || verbose)) 
                {
			cout << "Finished evaluating " << candidate->ToShortString() << " - fitness: " << scoredFitness << endl;
		}
	}

	if (!debug && !verbose) 
        {
		// there has been no output on the fitness yet, do that now
		//cout << scoredFitness << endl;
	}
}
ControllerPtr PopulationControlArchitecture::selectParentByBinaryTournamentA()
{
    int i=Rand::randint(0,activeSolutions.size());
    ControllerPtr parentA=activeSolutions.at(i);
    cout<<"##PARENT A: "<<parentA->ToShortString()<<endl;
    int j=Rand::randint(0,activeSolutions.size());
    while(j==i)
    {
        j=Rand::randint(0,activeSolutions.size());
    }
    ControllerPtr parentB=activeSolutions.at(j);
    cout<<"##PARENT B: "<<parentB->ToShortString()<<endl;
    if (parentA->GetOriginalFitness() > parentB->GetOriginalFitness()) 
    {
		return parentA;
    }
    else 
    {
		return parentB;
    }
    
    
}

ControllerPtr PopulationControlArchitecture::selectParentsByBinaryTournamentN(vector<ControllerPtr> *exclude) {
    int candidate_id=candidate->getWorldModel()->_agentId;
        //std::cout<<"\n###GOING INTO RAND NEWS"<<endl; 
    
        //NewsItem *selectedSolutionA = population->randomNews(exclude,candidate_id);
    NewsItem *selectedSolutionA = population->randomNews(exclude);    
	ControllerPtr parentA;
	if (selectedSolutionA != NULL) 
        {
           parentA = selectedSolutionA->getContent();
	}
	if (!parentA) 
        { 
            if (debug) 
                {
			cout << "Got NULL parent A, generating random parent A." << endl;
		}
		parentA = this->createRandomGenome();
	}

	exclude->push_back(parentA);
	NewsItem *selectedSolutionB = population->randomNews(exclude);
	exclude->pop_back();
	ControllerPtr parentB;
	if (selectedSolutionB != NULL) 
        {
		parentB = selectedSolutionB->getContent();
	}
	if (!parentB) 
        { // the population gave us a NULL parent; generate a random one
		//cout << population->getId() << " - ParentB: NULL" << endl;
		if (debug) 
                {
			cout << "Got NULL parent B, generating random parent B." << endl;
		}
		parentB = this->createRandomGenome();
	}

	if (debug) 
        {
		cout << "ParentA: " << parentA->GetOriginalFitness() << " vs ParentB: " << parentB->GetOriginalFitness() << endl;
	}

	return (parentA->GetOriginalFitness() > parentB->GetOriginalFitness() ? parentA : parentB);
}



/*
ControllerPtr PopulationControlArchitecture::selectParentsByBinaryTournamentA(ControllerPtr exclude)
{
    int ex=exclude->getId();
    
    int i=Rand::randint(0,activeSolutions.size());
    ControllerPtr parentA=activeSolutions.at(i);
    
    while(ex==parentA->getId())
    {
        i=Rand::randint(0,activeSolutions.size());
        parentA=activeSolutions.at(i);
    }
    
    cout<<"##PARENT A: "<<parentA->ToShortString()<<endl;
    int j=Rand::randint(0,activeSolutions.size());
    ControllerPtr parentB=activeSolutions.at(j);
    while(j==i || ex==parentB->getId())
    {
        j=Rand::randint(0,activeSolutions.size());
        parentB=activeSolutions.at(j);
    }
    
    cout<<"##PARENT B: "<<parentB->ToShortString()<<endl;
    if (parentA->GetOriginalFitness() > parentB->GetOriginalFitness()) 
    {
		return parentA;
    }
    else 
    {
		return parentB;
    }
}
*/

ControllerPtr PopulationControlArchitecture::selectParentByBinaryTournament(ControllerPtr exclude) {
	NewsItem *selectedSolutionA = population->randomNews(exclude);
	ControllerPtr parentA;
	if (selectedSolutionA != NULL) 
        {
		/*if (debug) {
			cout << population->getId() << " - ParentA: " << selectedSolutionA->getAgentId() << " (" << selectedSolutionA->getTimestamp() << ")" << endl;
		}*/
		parentA = selectedSolutionA->getContent();
	}
    
	if (!parentA) { // the population gave us a NULL parent; generate a random one
		//cout << population->getId() << " - ParentA: NULL" << endl;
		if (debug) {
			cout << "Got NULL parent A, generating random parent A." << endl;
		}
		parentA = this->createRandomGenome();
	}

	// select a second parent, excluding the first from the selection
	NewsItem *selectedSolutionB = population->randomNews(parentA, exclude);
	ControllerPtr parentB;
	if (selectedSolutionB != NULL) {
		/*
                if (debug) {
			cout << population->getId() << " - ParentB: " << selectedSolutionB->getAgentId() << " (" << selectedSolutionB->getTimestamp() << ")" << endl;
		}*/
		parentB = selectedSolutionB->getContent();
	}
    
	if (!parentB) { // the population gave us a NULL parent; generate a random one
		//cout << population->getId() << " - ParentB: NULL" << endl;
		if (debug) {
			cout << "Got NULL parent B, generating random parent B." << endl;
		}
		parentB = this->createRandomGenome();
	}
        /*
	if (debug) {
		cout << "ParentA: " << parentA->GetOriginalFitness() << " vs ParentB: " << parentB->GetOriginalFitness() << endl;
	}
         */ 
	if (parentA->GetOriginalFitness() > parentB->GetOriginalFitness()) {
		return parentA;
	} else {
		return parentB;
	}
}

ControllerPtr PopulationControlArchitecture::selectParentsByBinaryTournament(vector<ControllerPtr> *exclude) {
	NewsItem *selectedSolutionA = population->randomNews(exclude);
	ControllerPtr parentA;
	if (selectedSolutionA != NULL) 
        {
            /*
		if (debug) 
                {
			cout << population->getId() << " - ParentA: " << selectedSolutionA->getAgentId() << " (" << selectedSolutionA->getTimestamp() << ")" << endl;
		}*/
		parentA = selectedSolutionA->getContent();
	}
	if (!parentA) 
        { 
            if (debug) 
                {
			cout << "Got NULL parent A, generating random parent A." << endl;
		}
		parentA = this->createRandomGenome();
	}

	exclude->push_back(parentA);
	NewsItem *selectedSolutionB = population->randomNews(exclude);
	exclude->pop_back();
	ControllerPtr parentB;
	if (selectedSolutionB != NULL) 
        {
            /*
		if (debug) {
			cout << population->getId() << " - ParentB: " << selectedSolutionB->getAgentId() << " (" << selectedSolutionB->getTimestamp() << ")" << endl;
		}
              */
		parentB = selectedSolutionB->getContent();
	}
	if (!parentB) 
        { // the population gave us a NULL parent; generate a random one
		//cout << population->getId() << " - ParentB: NULL" << endl;
		if (debug) 
                {
			cout << "Got NULL parent B, generating random parent B." << endl;
		}
		parentB = this->createRandomGenome();
	}

	if (debug) 
        {
		cout << "ParentA: " << parentA->GetOriginalFitness() << " vs ParentB: " << parentB->GetOriginalFitness() << endl;
	}

	return (parentA->GetOriginalFitness() > parentB->GetOriginalFitness() ? parentA : parentB);
}
