#!/bin/bash

# run_dynamic.sh
# roborobo
#
# Created by B.P.M. Weel on 3/20/12.
# Copyright 2012 VU. All rights reserved.

### Set the simulation-specific parameters
DIR=$(dirname $(readlink -f $0))
BASEDIR="$(dirname $DIR)/"
TEMPLATEDIR=scripts/templates/
CONFNAME=dynamic_p

cpu=12 # run 6 at a time

FULLCOMMAND="$0 $@"
. ${BASEDIR}scripts/shflags

#define the flags
DEFINE_integer 'repeats' '50' 'The number of times the simulation should be run';
DEFINE_string 'robots' '10' 'The number of robots in the simulation' 'n';
DEFINE_integer 'seed' '-1' 'Seed' 'x';
DEFINE_string 'mu' '5' 'Mu' 'm'
DEFINE_string 'stepsPerCandidate' '500' 'Steps per candidate' 't'
DEFINE_string 'reEvaluationRate' '0.1' 'Re-evaluation Rate' 'r'
DEFINE_string 'crossoverRate' '1.0' 'Crossover rate' 'c'
DEFINE_string 'mutationRate' '1.0' 'Mutation rate' 'p'
DEFINE_string 'initialMutationStepSize' '5' 'Initial mutation step size' 's'
DEFINE_string 'newscastCacheSize' '4' 'Newscast cache size' 'C'
DEFINE_string 'newscastItemTTL' '8' 'Number of generations a Newscast item can live' 'L'
DEFINE_string 'numberOfParents' '2' 'Number of parents' 'P'
DEFINE_string 'iterations' '50000' 'Number of iterations' 'i'
DEFINE_string 'speedScale' '4' 'Scale of organism speed increase' 'a'

RANDOM=42

MU=6
ROBOTS=100
MUTATIONRATE=1.0
CROSSOVERRATE=0.05
REEVALUATIONRATE=0.5
STEPSPERCANDIDATE=950
INITIALMUTATIONSTEPSIZE=0.1
ITERATIONS=1000000

# Parse the flags
FLAGS "$@" || exit 1
eval set -- "${FlAGS_ARGV}"

### Run the simulation
COUNTER=${FLAGS_repeats}

echo "starting simulations, repeating $COUNTER times"
cd $BASEDIR
MIN=1


until [ "$COUNTER" -lt "$MIN" ];  do
    count=`ps aux | grep roborobo | grep -v grep | awk '{print $2}' | wc -w`
    if [ $(($count-$cpu)) -lt 0 ]; then
        SEED=$RANDOM
        let COUNTER-=1
        let EXPERIMENT=${FLAGS_repeats}-$COUNTER

        echo "Running experiment #$EXPERIMENT with parameters: --robots $ROBOTS --mu $MU --stepsPerCandidate $STEPSPERCANDIDATE --mutationRate $MUTATIONRATE --crossoverRate $CROSSOVERRATE --reEvaluationRate $REEVALUATIONRATE --newscastCacheSize ${FLAGS_newscastCacheSize} --newscastItemTTL ${FLAGS_newscastItemTTL} --initialMutationStepSize $INITIALMUTATIONSTEPSIZE --iterations $ITERATIONS --speedScale ${FLAGS_speedScale} --seed $SEED --basedir $BASEDIR --templatedir $TEMPLATEDIR --confname $CONFNAME"
        ${BASEDIR}scripts/run_scale.sh --robots $ROBOTS --mu $MU --stepsPerCandidate $STEPSPERCANDIDATE --mutationRate $MUTATIONRATE --crossoverRate $CROSSOVERRATE --reEvaluationRate $REEVALUATIONRATE --newscastCacheSize ${FLAGS_newscastCacheSize} --newscastItemTTL ${FLAGS_newscastItemTTL} --initialMutationStepSize $INITIALMUTATIONSTEPSIZE --iterations $ITERATIONS --speedScale ${FLAGS_speedScale} --seed $SEED --basedir $BASEDIR --templatedir $TEMPLATEDIR --confname $CONFNAME &
    fi
    sleep 2s
done

wait
