import functools
import numpy as np
import math
import os
import scipy.io as sio
import time
from sklearn.datasets import load_digits
import pandas as pd


def convert_memory_ordering_f2c(array):
    if np.isfortran(array) is True:
        return np.ascontiguousarray(array)
    else:
        return array


def downloadMnistData():
    mnist = load_digits()
    inputs = mnist.data
    labels = mnist.target
    return (inputs,labels)

def load_testing_data(testing_file='data.mat'):
    '''Load training data (mnistdata.mat) and return (inputs, labels).

    inputs: numpy array with size (5000, 400).
    labels: numpy array with size (5000, 1).

    The training data is from Andrew Ng's exercise of the Coursera
    machine learning course (ex4data1.mat).
    '''
    
      
    testing_data = sio.loadmat(testing_file)
    inputs = testing_data['X'].astype('f8')
    inputs = convert_memory_ordering_f2c(inputs)
    labels = testing_data['y'].reshape(testing_data['y'].shape[0])
    labels = convert_memory_ordering_f2c(labels)
    return (inputs, labels)

if __name__ == '__main__':

    Testinputs, Testlabels = load_testing_data()
    inputs, labels = downloadMnistData()
    
    with open('TestFeatures.npy', 'wb') as f:
       np.save(f,Testinputs)

    with open('TestLabels.npy', 'wb') as f:
       np.save(f, Testlabels)


    with open('TrainFeatures.npy', 'wb') as f:
       np.save(f,inputs)

    with open('TrainLabels.npy', 'wb') as f:
       np.save(f, labels)


    print(inputs,labels)
