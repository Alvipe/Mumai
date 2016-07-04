# -*- coding: utf-8 -*-
"""
Created on Tue Nov 10 09:03:53 2015

@author: Alvaro
"""

from emgesture import fextraction as fex
import numpy as np
from sklearn.cross_validation import train_test_split
from sklearn.svm import SVC

# Data loading
emg_data = np.load('emg_data_5class_2channel.npy').item()

nGestures = 5
nChannels = 2
nIterations = 5
emg = []
segmented_emg = []

for m in range(1,nGestures+1):
    for i in range(nIterations):
        for c in range(1,nChannels+1):
            emg.append(emg_data['motion'+str(m)+'_ch'+str(c)][:,i]) #motion1_ch1_i1, motion1_ch2_i1, motion1_ch1_i2, motion1_ch2_i2

nSignals = len(emg)

# Segmentation
for n in range(nSignals):
    segmented_emg.append(fex.segmentation(emg[n]))

# Feature calculation
feature_list = [fex.mav, fex.rms, fex.var, fex.ssi, fex.zc, fex.wl, fex.ssc, fex.wamp]

nSegments = len(segmented_emg[0][0])
nFeatures = len(feature_list)
feature_matrix = np.zeros((nSegments*nGestures*nIterations,nFeatures*nChannels))
n = 0

for i in range(0,nSignals,nChannels):
    for j in range(nSegments):
        feature_matrix[n] = fex.features((segmented_emg[i][:,j],segmented_emg[i+1][:,j]),feature_list)
        n = n + 1

# Target matrix generation
y = fex.gestures(nSegments*nIterations,nGestures)

# Dimensionality reduction and feature scaling
[X,reductor,scaler] = fex.feature_scaling(feature_matrix, y)

# Split dataset into training and testing datasets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.4, random_state=42)

# Classifier training
classifier = SVC(kernel='rbf',C=10,gamma=10)
classifier.fit(X_train,y_train)

# Classification
predict = classifier.predict(X_test)
print("Classification accuracy = %0.5f." %(classifier.score(X_test,y_test)))

## Cross validation (optional; takes a lot of time)
#from sklearn.cross_validation import StratifiedShuffleSplit
#from sklearn.grid_search import GridSearchCV
#from sklearn.svm import SVC
#
#C_range = np.logspace(-5,5,11)
#gamma_range = np.logspace(-30,1,32)
#param_grid = dict(gamma=gamma_range,C=C_range)
#cv = StratifiedShuffleSplit(y, n_iter=20,test_size=0.2,random_state=42)
#grid = GridSearchCV(SVC(),param_grid=param_grid,cv=cv)
#grid.fit(X,y)
#print("The best parameters are %s with a score of %0.2f" % (grid.best_params_,grid.best_score_))
