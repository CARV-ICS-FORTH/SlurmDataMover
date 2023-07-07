#!/bin/bash

#SBATCH --job-name=Train			# Job name:
#SBATCH -q preempt 
#SBATCH --partition=mpi-low			# Partition:  
#SBATCH --nodes=2				# Request one node:
#SBATCH --ntasks=62			# Specify one task:
#SBATCH --cpus-per-task=1			# Number of processors for single task:
#SBATCH --time=01:00:00				# Wall clock limit:
#SBATCH --output=Train%j
#SBATCH --mem=80000M			# Amount of RAM for each core:
#SBATCH --comment=01:00
#SBATCH --requeue
#SBATCH --open-mode=append
#SBATCH --mail-type=end,fail
#SBATCH --mail-user=klodi_hidri@hotmail.com

srun  bash basic_script.sh 

#export OMP_PROC_BIND=true
#export OMP_PLACES=threads
#export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}
export OMPI_MCA_pml="^ucx"
export OMPI_MCA_oob_tcp_listen_mode="listen_thread"


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

srun -s -u --mpi=pmix_v3  /home/cadisa/slurmCluster/slurm-data-manager/build/sdm -r hidri-node01\
        -gTrainFeatures.npy@fast\
	-gTrainLabels.npy@fast\
	-pmnistweights.mat@fast\
	-gmnist-nn-data-parallelism.py@fast\
	-e "python mnist-nn-data-parallelism.py" &


srun -s -u --mpi=pmix_v3  /home/cadisa/slurmCluster/slurm-data-manager/build/sdm -r hidri-node01 -gdata.mat@fast -e "python mnist-nn-data-parallelism.py"    &
echo "The script pid is $(pidof srun)" 
#wait_term
wait 

