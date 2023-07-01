import functools
import numpy as np
import math
import os
import scipy.io as sio
import time
from mpi4py import MPI
import time


# Init MPI
comm = MPI.COMM_WORLD

with open('TestFeatures.npy', 'rb') as f:
     inputs = np.load(f)

with open('TestLabels.npy', 'rb') as f:
      labels = np.load(f)



def sigmoid(z):
    return 1.0 / (1 + pow(math.e, -z))




def convert_memory_ordering_f2c(array):
    if np.isfortran(array) is True:
       return np.ascontiguousarray(array)
    else:
       return array



def predict(model, inputs):
    theta1, theta2 = model
    a1 = np.insert(inputs, 0, 1, axis=1)# add bias, (5000,401)
    a2 = np.dot(a1, theta1.T)  		# (5000,401) x (401,25)
    a2 = sigmoid(a2)
    a2 = np.insert(a2, 0, 1, axis=1)  	# add bias, (5000,26)
    a3 = np.dot(a2, theta2.T)  		# (5000,26) x (26,10)
    a3 = sigmoid(a3)  			# (5000,10)
    return [i.argmax()+1 for i in a3]



def accuracy(outputs):
    correct_prediction = 0
    for i, predict in enumerate(outputs):
        
        if predict == labels[i]:
            correct_prediction += 1
    precision = float(correct_prediction) / len(labels)
    print( 'Accuracy: {}'.format(precision))


def load_weights(weight_file='mnistweights.mat'):

    weights = sio.loadmat(weight_file)
    theta1 = convert_memory_ordering_f2c(weights['Theta1'].astype('f8'))  # size: 25 entries, each has 401 numbers
    theta2 = convert_memory_ordering_f2c(weights['Theta2'].astype('f8'))  # size: 10 entries, each has  26 numbers
    return (theta1, theta2)


if __name__ == '__main__':

    if comm.rank == 0:
	    time_start = time.time()
	    weights = load_weights() 
	    outputs = predict(weights, inputs)
	    accuracy(outputs)
	    time_end = time.time() 
	    print('Inference Time: {0}'.format(time_end - time_start))

