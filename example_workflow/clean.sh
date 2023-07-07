rm -rf TestFeatures.npy
rm -rf  Predict*
rm -rf Transform*
rm -rf Train*
rm -rf mnistweights.mat
rm -rf TestLabels.npy
rm -rf core.*
rm -rf ValLabels.npy
rm -rf ValFeatures.npy
srun  -N 2 rm -rf  /tmp/hidri/fast/*
