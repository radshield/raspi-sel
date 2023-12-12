# Helper script to load and test against a generated model

import pickle
import numpy as np
from sklearn.linear_model import LinearRegression

n_features = 29

# Load a previous regression model
def load_model(location):
    model = pickle.load(open(location, 'rb'))
    return model


# Make a prediction on the loaded model
def test_model(input_features, model):
    feature_values = input_features.decode().split(",")
    if(len(feature_values) != n_features):
        raise SystemError("Incorrect number of features")

    feature_values = [int(i) for i in feature_values]
    feature_values = np.array(feature_values).reshape(1,n_features)
    output = model.predict(feature_values)
    return output
