# -*- coding: utf-8 -*-
"""
Created on Tue Nov 10 09:03:53 2015

@author: Alvaro
"""

from emgesture import fextraction as fex
from emgesture import classifier as clf
import scipy.io as sio
import numpy as np

### Data loading ###
emg_data = sio.loadmat('emg_data_5_class_2ch')
#emg_data = sio.loadmat('emg_25-11-15')
nGestures = 5
nChannels = 2
nIterations = 4
emg = []
segmented_emg = []

for m in range(1,nGestures+1):
        for i in range(nIterations):
            for c in range(1,nChannels+1):
                emg.append(emg_data['motion'+str(m)+'_ch'+str(c)][:,i]) #motion1_ch1_i1, motion1_ch2_i1, motion1_ch1_i2, motion1_ch2_i2

### Segmentation ###
for n in range(len(emg)):
    segmented_emg.append(fex.segmentation(emg[n]))

### Feature calculation ###
feature_list = [fex.mav, fex.rms, fex.var, fex.ssi, fex.zc, fex.wl, fex.ssc, fex.wamp]

nSegments = len(segmented_emg[0][0])
nFeatures = len(feature_list)

feature_matrix = np.zeros((nSegments*nIterations*nGestures,nFeatures*nChannels))
n = 0

for i in range(0,len(segmented_emg),nChannels):
    for j in range(nSegments):
        feature_matrix[n] = fex.features((segmented_emg[i][:,j],segmented_emg[i+1][:,j]),feature_list)
        n += 1

import timeit

### Target matrix generation ###
y = fex.gestures(nIterations*nSegments,nGestures)

tic = timeit.default_timer()

### Feature scaling ###
[X,reductor,scaler] = fex.feature_scaling(feature_matrix, y)

### Classifier training ###
classifier = clf.train(X,y)

toc = timeit.default_timer()

print("Feature transformation + training time = %0.5f s." %(toc - tic))

emg_test = []
segmented_emg_test = []

for m in range(1,nGestures+1):
    for c in range(1,nChannels+1):
        emg_test.append(emg_data['motion'+str(m)+'_ch'+str(c)][:,4]) #motion1_ch1_i1, motion1_ch2_i1, motion1_ch1_i2, motion1_ch2_i2

### Segmentation ###
for n in range(len(emg_test)):
    segmented_emg_test.append(fex.segmentation(emg_test[n]))

### Feature calculation ###
nIterations = 1 #To test the classifier, the last iteration of each gesture is used
nSegments = len(segmented_emg_test[0][0])

feature_matrix_test = np.zeros((nSegments*nIterations*nGestures,nFeatures*nChannels))
n = 0

for i in range(0,len(segmented_emg_test),nChannels):
    for j in range(len(segmented_emg_test[0][0])):
        feature_matrix_test[n] = fex.features((segmented_emg_test[i][:,j],segmented_emg_test[i+1][:,j]),feature_list)
        n = n + 1

### Target matrix generation ###
y_test = fex.gestures(nSegments,nGestures)

tic = timeit.default_timer()

### Feature scaling ###
[X_test,reductor,scaler] = fex.feature_scaling(feature_matrix_test,y_test,reductor,scaler)

### Classification ###
predict = clf.classify(X_test,classifier)

toc = timeit.default_timer()

print("Feature transformation + classification time = %0.5f s." %(toc - tic))

print("Classification accuracy = %0.5f." %(classifier.score(X_test,y_test)))
