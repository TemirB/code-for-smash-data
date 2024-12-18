#!/bin/bash
# nohup sh superScript.sh 16 125 = 10M events
# Количество ядер
export N_TASKS=$1
# Кол-во раз пробегаем цикл по сути (Нужно event)/(кол-во event в config.yaml) 
export N_RUNS=$2
export JOB_NAME=${3:-"myjob"}

run_smash() {
    for TASK in $(seq 1 $N_TASKS); do
        OUT_DIR=/home/ubuntu/Temir/smash/build/data/tmp/tmp_${JOB_NAME}_${TASK}
        LOG=/home/ubuntu/Temir/smash/build/data/oscarDst/smash_${JOB_NAME}_${TASK}.log
        mkdir -p $OUT_DIR
        touch $LOG
        echo "OUTFILE: ${OUT_DIR}" >> $LOG
        echo "Everything is set. Let's do some physics!" >> $LOG

        cd /home/ubuntu/Temir/smash/build/
        nohup ./smash -o ${OUT_DIR} >> $LOG 2>&1 &
    done

    # Ждём завершения всех smash процессов
    wait
    echo "Все SMASH задачи завершены."
}

run_oscar_reader() {
    for TASK in $(seq 1 $N_TASKS); do
        OUT_DIR=/home/ubuntu/Temir/smash/build/data/tmp/tmp_${JOB_NAME}_${TASK}
        LOG=/home/ubuntu/Temir/smash/build/data/oscarDst/oscarReader_${JOB_NAME}_${TASK}.log
        touch $LOG
        echo "OUTFILE: ${OUT_DIR}" >> $LOG
        echo "Everything is set. Let's do some physics!" >> $LOG

        # Включаем номер RUN в имя выходного .root файла
        cd /home/ubuntu/Temir/smash/build/data/forOscar
        nohup ./oscarFileReader $OUT_DIR/full_event_history.oscar /home/ubuntu/Temir/smash/build/data/oscarDst/oscarDst_${JOB_NAME}_${TASK}_${RUN}.root >> $LOG 2>&1 &
    done

    # Ждём завершения всех oscarFileReader процессов
    wait
    echo "Все oscarFileReader задачи завершены."
}

for RUN in $(seq 1 $N_RUNS); do
    echo "Запуск $RUN из $N_RUNS..."

    run_smash
    
    run_oscar_reader

    # Очистка временных директорий
    for TASK in $(seq 1 $N_TASKS); do
        OUT_DIR=/home/ubuntu/Temir/smash/build/data/tmp/tmp_${JOB_NAME}_${TASK}
        rm -rf $OUT_DIR
    done

    echo "Запуск $RUN завершён."
done

echo "Все этапы завершены."
