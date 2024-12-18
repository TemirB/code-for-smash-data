#!/bin/bash

if [ $# -ne 2 ] ; then
    echo "Usage: sh GenerateList.sh <dataset> <number_of_files_per_task>"
    exit
fi

# The STAR data are stored here
export DATA_DIR=/home/ubuntu/Temir/smash/build/data/oscarDst


export DATASET=$1

if  [ ${DATASET} != "3gev" ] ; then
  echo "Sorry! For the time being, only the following datasets are available:"
  echo "3gev"
  exit
fi

export N_FILES_PER_TASK=$2

# The main working directory
# export MAIN_DIR=`dirname "$( readlink -f ${BASH_SOURCE}; )"`/runlist
export MAIN_DIR=/home/ubuntu/Temir/smash/build/data/script/runlist
mkdir -p ${MAIN_DIR}

# The input file list 
export FILELIST=${MAIN_DIR}/runlist_slurm_${DATASET}_${N_FILES_PER_TASK}FilesPerTask.list

# Checking for the filelist existence in MAIN_DIR
if [ -f ${FILELIST} ] ; then
  echo "This list has already been created."
  exit
fi

echo "Initializing a list containing lists of ROOT files (Inception by Christopher Nolan)"
if [ ! -d ${MAIN_DIR}/split/${DATASET}_${N_FILES_PER_TASK}FilesPerTask ]; then
  mkdir -p ${MAIN_DIR}/split/${DATASET}_${N_FILES_PER_TASK}FilesPerTask
else
  echo "This list has already been created."
  export N_TASKS=$( ls ${MAIN_DIR}/split/${DATASET}_${N_FILES_PER_TASK}FilesPerTask | wc -l )
  echo "Number of needed job array for ${DATASET} dataset: ${N_TASKS}"
  exit
fi

echo "Listing all ROOT tree of $DATASET data set (Inception - level 1)"
if [ ! -f ${MAIN_DIR}/split/runlist_${DATASET}.list ] ; then
  # find ${DATA_DIR}/${DATASET}/subdata?/  -type f -name "*.femtoDst.root" > ${MAIN_DIR}/split/runlist_${DATASET}.list
  find ${DATA_DIR}/  -type f -name "oscarDst_10M_force_*.root" > ${MAIN_DIR}/split/runlist_${DATASET}.list
  # find /home/ubuntu/Temir/smash/build/data/oscarDst/  -type f -name "oscarDst_ten_millon_events_run_*.root" > ${MAIN_DIR}/split/runlist_${DATASET}.list
  # find /home/ubuntu/Temir/smash/build/data/oscarDst/  -type f -name "oscarDst_10_mill_*.root" >> ${MAIN_DIR}/split/runlist_${DATASET}.list
  # find /media/ubuntu/SMASH/rootFile_10kk/  -type f -name "oscarDst_*.root" >> ${MAIN_DIR}/split/runlist_${DATASET}.list
  
fi

echo "Creating list for each job (Inception - level 2)"
split -l${N_FILES_PER_TASK} -d ${MAIN_DIR}/split/runlist_${DATASET}.list ${MAIN_DIR}/split/${DATASET}_${N_FILES_PER_TASK}FilesPerTask/runlist_${DATASET}_
for filename in ${MAIN_DIR}/split/${DATASET}_${N_FILES_PER_TASK}FilesPerTask/runlist_${DATASET}_*;do mv "$filename" "$filename.list"; done;


echo "Initializing runlist for SGE JOB (Inception - level 3)"
ls ${MAIN_DIR}/split/${DATASET}_${N_FILES_PER_TASK}FilesPerTask/runlist_${DATASET}_*.list > ${FILELIST}
echo "Non, Je ne regrette rien... Wake up!"

export N_TASKS=$( wc -l ${FILELIST} | cut -f1 -d' ' )
echo "Number of needed tasks for full statistics of the ${DATASET} dataset: ${N_TASKS}"

# echo "Now, to run the hole statistics, one may use the following command:"
# echo "sbatch -a 1-${N_TASKS} SubmitJob.sh ${DATASET} ${N_FILES_PER_TASK}"
# echo ""
# echo "To run 5 tasks for QAs before going full statistics, one can use the following command:"
# echo "sbatch -a 1-5 SubmitJob.sh ${DATASET} ${N_FILES_PER_TASK}"
