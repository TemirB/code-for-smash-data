#!/bin/bash

# Compiler
export N_TASKS=$1

# Number of tasks to be submitted
export FILELIST=$2

export DATASET=$3

# Pick a job name
export JOB_NAME=${4:-"myjob"}
# The main working directory
# export MAIN_DIR=`dirname "$( readlink -f ${BASH_SOURCE}; )"`
export MAIN_DIR=/home/ubuntu/Temir/smash/build/data/script

# The directory holding all output files 
export OUT_DIR=${MAIN_DIR}/OUT

# Output of given job ID will be stored here
export OUT=${OUT_DIR}/${JOB_NAME}

# # for TASK in {1..$N_TASKS}
for TASK in `seq $N_TASKS`
do
  # IN_FILE=`sed "${TASK}q;d" $FILELIST`
  IN_FILE=$(sed "${TASK}q;d" $FILELIST )
  OUT_FILE=${OUT}/out_${JOB_NAME}_${TASK}.root
  OUT_LOG=${OUT}/log
  LOG=${OUT_LOG}/JOB_${JOB_NAME}_${TASK}.log
  mkdir -p $OUT_LOG
  touch $LOG
  echo "Everything is set. Let's do some physics!" &>>$LOG
  cd /home/ubuntu/Temir/smash/build/data/forOscar/
  nohup ./Particles ${IN_FILE} ${OUT_FILE} > $LOG &  
  echo "INFILE: ${IN_FILE}" &>> $LOG
  echo "OUTFILE: ${OUT_FILE}" &>> $LOG
done
