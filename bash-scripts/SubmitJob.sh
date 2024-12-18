#!/bin/bash

if [ $# -lt 2 ] ; then
  echo "Usage:"
  echo "1. Run one testing task:"
  echo "sh SubmitJob.sh <dataset> <number_of_files_per_task>"
  echo ""
  echo "2. Run N tasks:"!
  echo "sh SubmitJob.sh <dataset> <number_of_files_per_task> <n_tasks>"
  echo ""
  echo "3. Specify time limit in minutes:"
  echo "sh SubmitJob.sh <dataset> <number_of_files_per_task> <n_tasks> <time_limit>"
  echo ""
  echo "4. Specify compiler: g++ or root"
  echo "sh SubmitJob.sh <dataset> <number_of_files_per_task> <n_tasks> <time_limit> <compiler>"
  echo ""
  echo "5. Specify job name:"
  echo "sh SubmitJob.sh <dataset> <number_of_files_per_task> <n_tasks> <time_limit> <compiler> <job_name>"
  echo ""
  echo "6. To run full statistics:"
  echo "sh SubmitJob.sh <dataset> <number_of_files_per_task> full"
  echo ""
  echo "The available <datasets>:"
  echo "14gev_bes2  19gev_bes2  27gev_bes2  dau200  fxt_3gev  fxt_3p2gev  fxt_7p7gev  pau200  ruru200  zrzr200"
  exit
fi

# Define the working dataset
export DATASET=$1
if [ ${DATASET} != "14gev_bes2" ] && [ ${DATASET} != "19gev_bes2" ] && [ ${DATASET} != "27gev_bes2" ] && [ ${DATASET} != "dau200" ] && [ ${DATASET} != "3gev" ] && [ ${DATASET} != "fxt_3p2gev" ] && [ ${DATASET} != "fxt_7p7gev" ] && [ ${DATASET} != "pau200" ] && [ ${DATASET} != "ruru200" ] && [ ${DATASET} != "zrzr200" ]; then
  echo "Sorry! For the time being, only the following <dataset> are available:"
  echo "3gev 14gev_bes2  19gev_bes2  27gev_bes2  dau200  fxt_3gev  fxt_3p2gev  fxt_7p7gev  pau200  ruru200  zrzr200"
  exit
fi

# Define the number of files per task
export N_FILES_PER_TASK=$2

# Define number of tasks to be submitted
export N_TASKS=${3:-"1"}

# Job's name, stphys by default
export JOB_NAME=${4:-"stphys"}

# The main working directory
# export MAIN_DIR=`dirname "$( readlink -f "${BASH_SOURCE}"; )"`
export MAIN_DIR=/home/ubuntu/Temir/smash/build/data/script

# The input file list 
export FILELIST=${MAIN_DIR}/runlist/runlist_slurm_${DATASET}_${N_FILES_PER_TASK}FilesPerTask.list

# Checking for the filelist existence in MAIN_DIR
if [ ! -f ${FILELIST} ] ; then
  sh ${MAIN_DIR}/GenerateRunlist.sh ${DATASET} ${N_FILES_PER_TASK}
fi

export N_TASKS_FULL_STAT=$( wc -l ${FILELIST} | cut -f1 -d' ' )

cd ${MAIN_DIR}
if [ "${N_TASKS}" != "full" ]; then
  sh ${MAIN_DIR}/BashScript.sh ${N_TASKS} ${FILELIST} ${DATASET} ${JOB_NAME}
else
  sh ${MAIN_DIR}/BashScript.sh ${N_TASKS_FULL_STAT} ${FILELIST} ${DATASET} ${JOB_NAME}
fi
 