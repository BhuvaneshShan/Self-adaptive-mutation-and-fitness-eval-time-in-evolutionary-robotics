#!/bin/bash

FULLCOMMAND="$0 $@"
. ${BASEDIR}scripts/shflags

#define the flags
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

DEFINE_string 'basedir' '/home/bweel/' 'Base dir of experiment' 'b'
DEFINE_string 'templatedir' 'scripts/template' 'd'
DEFINE_string 'confname' 'scalepower_p' 'o'

# Parse the flags
FLAGS "$@" || exit 1
eval set -- "${FlAGS_ARGV}"

BASEDIR=${FLAGS_basedir}
TEMPLATEDIR=${FLAGS_templatedir}
CONFNAME=${FLAGS_confname}

RID=$RANDOM

### copy the template configuration to the config dir, making the neccesary adjustments
# Determine some properties of the test
ROBOTCOUNT=${FLAGS_robots}
TIMESTAMP=`date "+%Y%m%d.%Hh%Mm%Ss"`

# Determine where the configuration file will be placed
CONFDIR=${BASEDIR}config/
CONFFILE=${CONFDIR}${TIMESTAMP}.$RID.${CONFNAME}.$ROBOTCOUNT.properties
LOGFILE=${BASEDIR}logs/console-output.${TIMESTAMP}.$RID.log

## Normal
OUTPUTLOGFILE=logs\\/output.${TIMESTAMP}.$RID.log
GENOMELOGFILE=logs\\/genomes.${TIMESTAMP}.$RID.log
ORGANISMSIZESLOGFILE=logs\\/organism-sizes.${TIMESTAMP}.$RID.log
ORGANISMSLOGFILE=logs\\/organisms.${TIMESTAMP}.$RID.log
LOCATIONLOGFILE=logs\\/locations.${TIMESTAMP}.$RID.log

## For tuning
#OUTPUTLOGFILE=\\/dev\\/null
#GENOMELOGFILE=\\/dev\\/null
#ORGANISMSIZESLOGFILE=\\/dev\\/null
#ORGANISMSLOGFILE=\\/dev\\/null
#LOCATIONLOGFILE=\\/dev\\/null

# Prepare the rules that will fill out the configuration file
SEEDREP=s/--SEED/${FLAGS_seed:0:9}/g # extract only the first 9 decimals, because Roborobo can't handle int overflows
NBAGENTSREP=s/--NBAGENTS/${ROBOTCOUNT}/g
MUREP=s/--MU/${FLAGS_mu}/g
SPCREP=s/--STEPSPERCANDIDATE/${FLAGS_stepsPerCandidate}/g
RERREP=s/--REEVALUATIONRATE/${FLAGS_reEvaluationRate}/g
CRREP=s/--CROSSOVERRATE/${FLAGS_crossoverRate}/g
MRREP=s/--M-TATIONRATE/${FLAGS_mutationRate}/g
IMSSREP=s/--INITIALMUTATIONSTEPSIZE/${FLAGS_initialMutationStepSize}/g
NCCREP=s/--NEWSCASTCACHESIZE/${FLAGS_newscastCacheSize}/g
NCTTLREP=s/--NEWSCASTITEMTTL/${FLAGS_newscastItemTTL}/g
NOPREP=s/--NUMBEROFPARENTS/${FLAGS_numberOfParents}/g
ITERATIONREP=s/--ITERATIONS/${FLAGS_iterations}/g

SPEEDSCALEREP=s/--SPEEDSCALE/${FLAGS_speedScale}/g

OUTPUTLOGREP=s/--OUTPUTLOG/${OUTPUTLOGFILE}/g
GENOMELOGREP=s/--GENOMELOG/${GENOMELOGFILE}/g
ORGANISMSIZESREP=s/--ORGANISMSIZESLOG/${ORGANISMSIZESLOGFILE}/g
ORGANISMSREP=s/--ORGANISMSLOG/${ORGANISMSLOGFILE}/g
LOCATIONREP=s/--LOCATIONLOG/${LOCATIONLOGFILE}/g

# Fill out and place the configuration file
sed -e $SEEDREP -e $NBAGENTSREP -e $MUREP -e $SPCREP -e $RERREP -e $CRREP -e $MRREP -e $IMSSREP -e $NCCREP -e $NCTTLREP -e $OUTPUTLOGREP -e $GENOMELOGREP -e $ORGANISMSIZESREP -e $ORGANISMSREP -e $NOPREP -e $ITERATIONREP -e $LOCATIONREP -e $SPEEDSCALEREP ${BASEDIR}${TEMPLATEDIR}${CONFNAME}.template.properties > $CONFFILE

### Run RoboRobo!
BINFILE=${BASEDIR}roborobo
#BINFILE=${BASEDIR}/DerivedData/roborobo/Build/Products/Release/roborobo.app/Contents/MacOS/roborobo
$BINFILE -l $CONFFILE > $LOGFILE

### compute some statistics
#TICKSPERCONTROLLER=${FLAGS_stepsPerCandidate}
#TICKSPERRANDOMFREE=0
#TICKSPERMANUALFREE=50
#TICKSPEREVAL=$((${TICKSPERCONTROLLER}+${TICKSPERRANDOMFREE}+${TICKSPERMANUALFREE}))

# extract the x most recent fitness values, where x is
# the number of evaluations that span 10% of total ticks
#steps=$(echo "scale=0; ((${FLAGS_iterations}) * ${ROBOTCOUNT}) / ${TICKSPEREVAL}" | bc)
#values=`tail -n $steps $LOGFILE | cut -f1 -d' ' | sed 's/.*e.*/0/'`

# compute the average fitness value over the past 100% ticks
#total=0
#n=0

#for fitness in $values
#do
#  let n+=1
#  total=$(echo "scale=6; $total + $fitness" | bc)
#done

# write the average fitness value of the past 10% ticks to stdout and logfile
#echo "scale=6; $total / $n" | bc
#echo `echo "scale=6; $total / $n" | bc` : $TIMESTAMP.$RID : $FULLCOMMAND >> commands.log
#
