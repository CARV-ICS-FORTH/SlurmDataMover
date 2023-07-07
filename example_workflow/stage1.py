import numpy as np
import h5py
import time 








def load_mnist_h5_tr_te_val(n_train, n_test, n_val, filename1,filename2,filename3, 
                    path):
    """Data will be in format [n_samples, width, height, n_channels] """
     
    with h5py.File(path + filename1, 'r') as f:
        # Data should be floating point
        x_train = np.array( f["/x_train"], dtype=np.float32)
        x_test = np.array( f["/x_test"], dtype=np.float32)
        x_val= np.array( f["/x_val"], dtype=np.float32)

        # Labels should normally be integers
        y_train = np.array( f["/y_train"], dtype=np.int8)
        y_test = np.array( f["/y_test"], dtype=np.int8)
        y_val= np.array( f["/y_val"], dtype=np.int8)
        
        # Labels should normally be 1D vectors, shape (n_labels,) 
        y_train = np.reshape(y_train,(np.size(y_train)))
        y_test = np.reshape(y_test,(np.size(y_test)))
        y_val = np.reshape(y_val,(np.size(y_val)))
  
    x_test = np.squeeze(x_test ) 
    x_train= np.reshape(x_train,(x_train.shape[2],x_train.shape[0],x_train.shape[1]) ) 
    y_test = np.squeeze( y_test) 
    x_train= np.concatenate((x_train,x_test), axis=0)        
    y_train= np.concatenate((y_train,y_test), axis=0)
     

    with h5py.File(path + filename2, 'r') as f:
        # Data should be floating point
        x_train1 = np.array( f["/x_train"], dtype=np.float32)
        x_test1 = np.array( f["/x_test"], dtype=np.float32)
        x_val1= np.array( f["/x_val"], dtype=np.float32)

        # Labels should normally be integers
        y_train1= np.array( f["/y_train"], dtype=np.int8)
        y_test1 = np.array( f["/y_test"], dtype=np.int8)
        y_val1  = np.array( f["/y_val"], dtype=np.int8)
        
        # Labels should normally be 1D vectors, shape (n_labels,) 
        y_train1 = np.reshape(y_train1,(np.size(y_train1)))
        y_test1 = np.reshape(y_test1,(np.size(y_test1)))
        y_val1 = np.reshape(y_val1,(np.size(y_val1)))
  
    x_train1 =  np.squeeze(x_train1 ) 
    x_test1 =  np.squeeze(x_test1 ) 
    x_val1 =  np.squeeze(x_val1 ) 

    x_train1 = np.concatenate((x_train1,x_test1), axis=0)  
    x_train1 = np.concatenate((x_train1,x_val1), axis=0)     
    y_train1 = np.concatenate((y_train1,y_test1), axis=0)  
    y_train1 = np.concatenate((y_train1,y_val1), axis=0)   

    x_train1 = np.concatenate((x_train1,x_train), axis=0)  
    y_train1 = np.concatenate((y_train1,y_train), axis=0)  

    



    with h5py.File(path + filename3, 'r') as f:
        # Data should be floating point
        x_train2 = np.array( f["/x_train"], dtype=np.float32)
        x_test2  = np.array( f["/x_test"], dtype=np.float32)
        x_val2   = np.array( f["/x_val"], dtype=np.float32)

        # Labels should normally be integers
        y_train2 = np.array( f["/y_train"], dtype=np.int8)
        y_test2  = np.array( f["/y_test"], dtype=np.int8)
        y_val2   = np.array( f["/y_val"], dtype=np.int8)
        
        # Labels should normally be 1D vectors, shape (n_labels,) 
        y_train2 = np.reshape( y_train2 , (np.size(y_train2)) )
        y_test2  = np.reshape( y_test2, (np.size(y_test2)) )
        y_val2   = np.reshape( y_val2, (np.size(y_val2)) )
     
    x_test2  =  np.squeeze(x_test2 ) 
    x_val2   =  np.reshape( x_train2, (x_val2.shape[2], x_val2.shape[0], x_val2.shape[1]) )
    x_train2 =  np.reshape( x_train2, (x_train2.shape[2], x_train2.shape[0], x_train2.shape[1]) ) 
    y_test2  = np.squeeze( y_test2) 
    x_train2 = np.concatenate((x_train2,x_test2), axis=0)        
    x_train2 = np.concatenate((x_train2,x_val2), axis=0)
    y_train2 = np.concatenate((y_train2,y_test2), axis=0)  
    y_train2 = np.concatenate((y_train2,y_val2), axis=0)


    x_train2 = np.concatenate((x_train2,x_train1), axis=0)        
    y_train2 = np.concatenate((y_train2,y_train1), axis=0)  


    
    x_train2 =x_train2.reshape((x_train2.shape[0],x_train2.shape[2]*x_train2.shape[1]))
    x_test =x_test.reshape((x_test.shape[0],x_test.shape[2]*x_test.shape[1]))
    x_val1 =x_val1.reshape((x_val1.shape[0],x_val1.shape[2]*x_val1.shape[1]))
    return (x_train2, y_train2), (x_test, y_test), (x_val1, y_val1)

# This specific dataset contains scale variations in the range [1,4] relative the original MNIST dataset

path = '/home/cadisa/slurmCluster/dataset/mnist-large/'
f2 ='mnist_large_scale_tr50000_vl10000_te10000_outsize112-112_sctr4p000_scte4p000.h5'
f1 ='mnist_large_scale_te10000_outsize112-112_scte4p000.h5'
f3 ='mnist_large_scale_te10000_outsize112-112_scte8p000.h5'
#path = '/home/cadisa/slurmCluster/dataset/mnist-small/'
#filename =    'mnist_large_scale_te10000_outsize112-112_scte0p500.h5'


#

n_train = 10000
n_val = 1000
n_test = 1000


time_start = time.time()
(x_train, y_train), (x_test, y_test), (x_val, y_val) = load_mnist_h5_tr_te_val(
                                                             n_train, n_test, n_val, 
                                                             f1,f2,f3, path)
time_end = time.time()
print('Reading And transforming Dataset Time: {0}'.format(time_end - time_start))





time_start = time.time()

with open('ValFeatures.npy', 'wb') as f:
       np.save(f,x_val)

with open('ValLabels.npy', 'wb') as f:
       np.save(f,y_val )


with open('TestFeatures.npy', 'wb') as f:
       np.save(f,x_test)

with open('TestLabels.npy', 'wb') as f:
       np.save(f,y_test )

with open('TrainFeatures.npy', 'wb') as f:
       np.save(f,x_train)

with open('TrainLabels.npy', 'wb') as f:
       np.save(f, y_train)


time_end = time.time()
print('Writting Dataset On HDD Disc Time: {0}'.format(time_end - time_start))


print("Shape training data:\t", np.shape(x_train))
print("Shape testing data:\t", np.shape(x_test))
print("Shape validation data:\t", np.shape(x_val))
print()
print("Shape training labels:\t", np.shape(y_train))
print("Shape test labels:\t", np.shape(y_test))
print("Shape validation labels: ", np.shape(y_val))
print()
print("Intensity range: [{:.2f}, {:.2f}] ".format(np.min(x_train), np.max(x_train)))

