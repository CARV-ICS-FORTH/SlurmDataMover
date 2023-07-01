import commands, os





cmd = "sbatch transformImages.sh"

print( "Job1: %s" % cmd)

status, job1 = commands.getstatusoutput(cmd)
job1=job1.split(' ')[3]

if (status == 0 ):
    print( "Job1 is %s" % job1)
else:
    print( "Error submitting Job1")


# submit the second job to be dependent on the first
cmd = "sbatch --depend=afterany:%s TRAIN.sh" % job1
print( "Submitting Job2 with command: %s" % cmd)
status,job2 = commands.getstatusoutput(cmd)
print(job2 , "STAtus",status)
job2=job2.split(' ')[3]

if (status == 0 ):
    print( "JobID is %s" % job2)
else:
    print( "Error submitting Job2")



# submit the third job (a swarm) to be dependent on the second
cmd = "sbatch  --depend=afterany:%s PREDICT.sh" % job2
print( "Submitting swarm job  with command: %s" % cmd)
status,job3 = commands.getstatusoutput(cmd)
job3=job3.split(' ')[3]
if (status == 0 ):
    print( "Job3 is %s" % job3)
else:
    print( "Error submitting Job3")

print( "\nCurrent status:\n")
#show the current status with 'sjobs'
os.system("squeue")


