#!/usr/bin/python
#
# MNIST digit recognizer in distributed GPU mode.
#
# This program is based on the prior MNIST digit recognizer, and change
# the matrix computing codes from using local CPU to distributed GPUs.
#
# To distribute data to worknodes, MPI (mpi4py) is used.  To execute
# matrix operations, Theano is used.
#

import functools
import numpy as np
import math
import os
import scipy.io as sio
import time
from scipy.io import savemat
import numpy 
import struct


from mpi4py import MPI

# Init MPI
comm = MPI.COMM_WORLD



Gpu_mode = False
Distributed = True

if Gpu_mode is True:
    import theano
    import theano.tensor as T


# Structure of the 3-layer neural network.
Input_layer_size = 12544  #400 #784 #400
Hidden_layer_size = 25
Output_layer_size = 10

# Matrix product function.  Default is to use CPU mode.
Matrix_dot = np.dot


def rand_init_weights(size_in, size_out):
    epsilon_init = 0.12
     
    return np.float128( np.random.rand(size_out, 1 + size_in).astype(dtype=np.float128) * 2 * epsilon_init - epsilon_init)


def sigmoid(z):
    return 1.0 / (1 + pow(math.e, -z))


def sigmoid_gradient(z):
    return sigmoid(z) * (1 - sigmoid(z))


if Gpu_mode is True:
    def gpu_matrix_dot():
        time_start = time.time()
        x = T.matrix('x')
        y = T.matrix('y')
        z = T.dot(x, y)
        f = theano.function([x, y], z, allow_input_downcast=True)
        time_end = time.time()
        print('theano expression creation costs {} secs'.format(time_end - time_start))
        return f
else:
    def gpu_matrix_dot():
        pass


def cost_function(theta1, theta2, input_layer_size, hidden_layer_size, output_layer_size, inputs, labels, regular=0):
    '''
    Note: theta1, theta2, inputs, labels are numpy arrays:

        theta1: (25, 401)
        theta2: (10, 26)
        inputs: (5000, 400)
        labels: (5000, 1)
    '''
    input_layer = np.insert(inputs, 0, 1, axis=1)  # add bias, 5000x401

    time_start   = time.time()
    hidden_layer = Matrix_dot(input_layer, theta1.T)
    hidden_layer = sigmoid(hidden_layer)
    hidden_layer = np.insert(hidden_layer, 0, 1, axis=1)  # add bias, 5000x26
    time_end = time.time()
    #if comm.rank == 0:
    #    print('\tconstruction: hidden layer dot costs {} secs'.format(time_end - time_start))

    time_start = time.time()
    output_layer = Matrix_dot(hidden_layer, theta2.T)  # 5000x10
    output_layer = sigmoid(output_layer)
    time_end = time.time()
    #if comm.rank == 0:
    #    print('\tconstruction: output layer dot costs {} secs'.format(time_end - time_start))

    # forward propagation: calculate cost
    time_start = time.time()
    cost = 0.0
    for training_index in range(len(inputs)):
        outputs = [0] * output_layer_size
        outputs[labels[training_index]-1] = 1

        for k in range(output_layer_size):
            error = -outputs[k] * math.log(output_layer[training_index][k]) - (1 - outputs[k]) * math.log(1 - output_layer[training_index][k])
            cost += error
    cost /= len(inputs)
    time_end = time.time()
    #if comm.rank == 0:
    #    print('\tforward prop: costs {} secs'.format(time_end - time_start))

    # back propagation: calculate gradiants
    time_start = time.time()
    theta1_grad = np.zeros_like(theta1,dtype=np.float128)  # 25x401
    theta2_grad = np.zeros_like(theta2,dtype=np.float128)  # 10x26
    for index in range(len(inputs)):
        # transform label y[i] from a number to a vector.
        outputs = np.zeros((1, output_layer_size),dtype=np.float128)  # (1,10)
        outputs[0][labels[index]-1] = 1

        # calculate delta3
        delta3 = (output_layer[index] - outputs).T  # (10,1)

        # calculate delta2
        z2 = Matrix_dot(theta1, input_layer[index:index+1].T)  # (25,401) x (401,1)
        z2 = np.insert(z2, 0, 1, axis=0)  # add bias, (26,1)
        delta2 = np.multiply(
            Matrix_dot(theta2.T, delta3),  # (26,10) x (10,1)
            sigmoid_gradient(z2)  # (26,1)
        )
        delta2 = delta2[1:]  # (25,1)

        # calculate gradients of theta1 and theta2
        # (25,401) = (25,1) x (1,401)
        theta1_grad += Matrix_dot(delta2, input_layer[index:index+1])
        # (10,26) = (10,1) x (1,26)
        theta2_grad += Matrix_dot(delta3, hidden_layer[index:index+1])
    theta1_grad /= len(inputs)
    theta2_grad /= len(inputs)
    time_end = time.time()
    #if comm.rank == 0:
    #    print('\tback prop: costs {} secs'.format(time_end - time_start))

    return cost, (theta1_grad, theta2_grad)

def read_npy_chunk(filename, start_row, num_rows, datatype):

    assert start_row >= 0 and num_rows > 0
    with open(filename, 'rb') as fhandle:
        major, minor = numpy.lib.format.read_magic(fhandle)
        shape, fortran, dtype = numpy.lib.format.read_array_header_1_0(fhandle)
        assert not fortran, "Fortran order arrays not supported"
        # Make sure the offsets aren't invalid.
        assert start_row < shape[0], (
            'start_row is beyond end of file'
        )
        assert start_row + num_rows <= shape[0], (
            'start_row + num_rows > shape[0]'
        )
        if datatype=='int8':
         dt=numpy.int8
         row_size = numpy.prod(shape[1:], dtype=dt)
        else :
         row_size = numpy.prod(shape[1:],dtype=np.float32)
        # Get the number of elements in one 'row' by taking
        # a product over all other dimensions.

        start_byte = start_row * int(row_size) * dtype.itemsize
        fhandle.seek(start_byte, 1)
        n_items = row_size * num_rows
        flat = numpy.fromfile(fhandle, count=int(n_items), dtype=dtype)
        return flat.reshape((-1,) + shape[1:])




def gradient_descent( learningrate=0.8, iteration=50):
    '''
    @return cost and trained model (weights).
    '''
    if Distributed is True:
        if comm.rank == 0:
            theta1 = rand_init_weights(Input_layer_size, Hidden_layer_size)
            theta2 = rand_init_weights(Hidden_layer_size, Output_layer_size)
        else:
            theta1 = np.zeros((Hidden_layer_size, Input_layer_size + 1),dtype=np.float128)
            theta2 = np.zeros((Output_layer_size, Hidden_layer_size + 1), dtype=np.float128)
        comm.Barrier()
        if comm.rank == 0:
            time_bcast_start = time.time()
        comm.Bcast([theta1, MPI.DOUBLE])
        comm.Barrier()
        comm.Bcast([theta2, MPI.DOUBLE])
        if comm.rank == 0:
            time_bcast_end = time.time()
            #print('\tBcast theta1 and theta2 uses {} secs.'.format(time_bcast_end - time_bcast_start))
    else:
        theta1 = rand_init_weights(Input_layer_size, Hidden_layer_size)
        theta2 = rand_init_weights(Hidden_layer_size, Output_layer_size)
   
    
    cost = 0.0
    time_train_start = time.time()
    time_overall = 0 
    for i in range(iteration):
        time_iter_start = time.time()
        print('Iteration: ',i)
        if Distributed is True:
            # Scatter training data and labels.
           for j in range(100):
            #comm.Barrier() 
            time_chunk_start = time.time()
            inputs = read_npy_chunk('TrainFeatures.npy',j*868,868, 'float')
            labels = read_npy_chunk('TrainLabels.npy',j*868,868, 'int8')
            time_chunk_end = time.time() 
            time_overall = time_overall + (time_chunk_end - time_chunk_start)             

            #inputs =inputs.reshape((inputs.shape[0],inputs.shape[2]*inputs.shape[1]))            
            comm.Barrier()
                       
            sliced_inputs = np.asarray(np.split(inputs, comm.size))
            sliced_labels = np.asarray(np.split(labels, comm.size))
            inputs_buf = np.zeros((len(inputs)//comm.size, Input_layer_size), dtype=np.float32)
            labels_buf = np.zeros((len(labels)//comm.size),dtype=np.int8 )
            
            if comm.rank == 0:
                time_scatter_start = time.time()
            comm.Scatter(sliced_inputs, inputs_buf)

            if comm.rank == 0:
                time_scatter_end = time.time()
                #print('\tScatter inputs uses {} secs.'.format(time_scatter_end - time_scatter_start))

            comm.Barrier()
            if comm.rank == 0:
                time_scatter_start = time.time()
            comm.Scatter(sliced_labels, labels_buf)
            if comm.rank == 0:
                time_scatter_end = time.time()
                #print('\tScatter labels uses {} secs.'.format(time_scatter_end - time_scatter_start))

            # Calculate distributed costs and gradients of this iteration
            # by cost function.
            comm.Barrier()
            cost, (theta1_grad, theta2_grad) = cost_function(theta1, theta2,
                Input_layer_size, Hidden_layer_size, Output_layer_size,
                inputs_buf, labels_buf, regular=0)

            # Gather distributed costs and gradients.
            comm.Barrier()
            cost_buf = [0] * comm.size
            try:
                cost_buf = comm.gather(cost)
                cost = sum(cost_buf) / len(cost_buf)
            except TypeError as e:
                print('[{0}] {1}'.format(comm.rank, e))

            theta1_grad_buf = np.asarray([np.zeros_like(theta1_grad)] * comm.size)
            comm.Barrier()
            if comm.rank == 0:
                time_gather_start = time.time()
            comm.Gather(theta1_grad, theta1_grad_buf)
            if comm.rank == 0:
                time_gather_end = time.time()
                #print('\tGather theta1 uses {} secs.'.format(time_gather_end - time_gather_start))
            comm.Barrier()
            theta1_grad = functools.reduce(np.add, theta1_grad_buf) / comm.size

            theta2_grad_buf = np.asarray([np.zeros_like(theta2_grad)] * comm.size)
            comm.Barrier()
            if comm.rank == 0:
                time_gather_start = time.time()
            comm.Gather(theta2_grad, theta2_grad_buf)
            if comm.rank == 0:
                time_gather_end = time.time()
                #print('\tGather theta2 uses {} secs.'.format(time_gather_end - time_gather_start))
            comm.Barrier()
            theta2_grad = functools.reduce(np.add, theta2_grad_buf) / comm.size
            print(j,"%")    
        else:
            cost, (theta1_grad, theta2_grad) = cost_function(theta1, theta2,
                Input_layer_size, Hidden_layer_size, Output_layer_size,
                inputs, labels, regular=0)

        theta1 -= learningrate * theta1_grad
        theta2 -= learningrate * theta2_grad

        if Distributed is True:
           # Sync-up weights for distributed worknodes.
           comm.Bcast([theta1, MPI.DOUBLE])
           comm.Bcast([theta2, MPI.DOUBLE])
           comm.Barrier()

        time_iter_end = time.time()
        #if comm.rank == 0:
        #    print('Iteration {0} (learning rate {2}, iteration {3}), cost: {1}, time: {4}'.format(
        #        i+1, cost, learningrate, iteration, time_iter_end - time_iter_start)
        #    )
        if  (i % 10) ==0:
            weights  = {"Cost": cost, "Theta1":theta1, "Theta2":theta2}  
            savemat("mnistweights.mat", weights)

    if comm.rank == 0:
        weights  = {"Cost": cost, "Theta1":theta1, "Theta2":theta2} 	 
        savemat("mnistweights.mat", weights)
    time_train_end = time.time()
    if comm.rank == 0:
        print('Training Loading Chunks Time: {0}'.format(time_overall))  
        print('Training Computation Time: {0}'.format((time_train_end - time_train_start)-time_overall))
    return cost, (theta1, theta2)


def train(learningrate=0.01, iteration=250):
    cost, model = gradient_descent( learningrate, iteration)
    return model


if __name__ == '__main__':

    print('CPU mode')
    Matrix_dot = np.dot

    if Distributed is True:
        print('Parallelism: yes')
    else:
        print('Parallelism: no')
    
    model = train(learningrate=0.01, iteration=1)

