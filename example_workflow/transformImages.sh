#!/bin/bash

#SBATCH --job-name=DataTransform			# Job name:
#SBATCH -q preempt 
#SBATCH --partition=mpi-low			# Partition:  
#SBATCH --nodes=1				# Request one node:
#SBATCH --ntasks-per-node=1			# Specify one task:
#SBATCH --cpus-per-task=1			# Number of processors for single task:
#SBATCH --time=00:30:00				# Wall clock limit:
#SBATCH --output=Transform%j
#SBATCH --mem-per-cpu=10000M			# Amount of RAM for each core:
#SBATCH --comment=01:00
#SBATCH --requeue
#SBATCH --open-mode=append
#SBATCH --mail-type=end,fail
#SBATCH --mail-user=klodi_hidri@hotmail.com




srun bash  basic_script.sh 

export OMP_PROC_BIND=true
export OMP_PLACES=threads
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}


export DMTCP_COORD_HOST=hidri-node01
export DMTCP_COORD_PORT=7779


echo "Hello From ${HOSTNAME} node"

echo "Hello from the batch queue on node ${SLURM_NODELIST}"


echo "== Starting run at $(date)"
echo "== Job ID: ${SLURM_JOBID}"
echo "== Node list: ${SLURM_NODELIST}"
echo "== Submit dir. : ${SLURM_SUBMIT_DIR}"
echo "== Number Of Tasks: ${SLURM_NTASKS}"

#srunProlog=srun_prolog.sh
#srunEpilog=srun_epilog.sh
taskProlog=task_prolog.sh
taskEpilog=/tmp/task_epilog.sh


srun -s  -u /home/cadisa/slurmCluster/slurm-data-manager/build/sdm -r hidri-node01\
		 -pTestFeatures.npy@fast\
		 -pTestLabels.npy@fast\
		 -pTrainFeatures.npy@fast\
		 -pTrainLabels.npy@fast\
		 -gstage1.py@fast\
		 -e "python stage1.py" &

#srun -s -u /home/cadisa/slurmCluster/slurm-data-manager/build/sdm -r hidri-node01 -gdata.mat@fast -e  "python stage1.py"   &

echo "The script pid is $(pidof srun)" 
#wait_term
wait 
##
