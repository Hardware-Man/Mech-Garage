import sys
import numpy as np
import matplotlib.pyplot as plt
from numpy.core.defchararray import upper

if __name__ == "__main__":
    
    # Retrive file name for input data
    if(len(sys.argv) < 5):
        print ("Four arguments required: python kalman2d.py [datafile] [x1] [x2] [lambda]")
        exit()
    
    filename = sys.argv[1]
    x10 = float(sys.argv[2])
    x20 = float(sys.argv[3])
    scaler = float(sys.argv[4])

    # Read data
    lines = [line.rstrip('\n') for line in open(filename)]
    data = []
    for line in lines:
        data.append([float(d) for d in line.split(' ')])

    # Print out the data
    print ("The input data points in the format of 'k [u1, u2, z1, z2]', are:")
    for it in range(0, len(data)):
        print (str(it + 1) + ": ", end='')
        print (*list(data[it]))
        

    def kalman(mu0, sig0, u, z):
        R = np.array([[0.01, 0.005], [0.005, 0.02]])
        Q = np.array([[0.0001, 0.00002], [0.00002, 0.0001]])

        # Predict
        predict_mu = mu0 + u
        predict_sig = sig0 + Q

        # Update
        K = predict_sig @ np.linalg.inv(predict_sig + R)
        mu1 = predict_mu + K @ (z - predict_mu)
        sig1 = (scaler*np.identity(2) - K) @ predict_sig

        return mu1, sig1

    q0 = np.array([x10, x20])
    sig0 = scaler * np.identity(2)
    prediction = [q0]
    observation = [q0]

    for k in range(0, len(data)):
        vars = list(data[k])
        u = np.array([vars[0],vars[1]])
        z = np.array([vars[2],vars[3]])
        q0, sig0 = kalman(q0, sig0, u, z)
        prediction.append(q0)
        observation.append(z)

    predict_x, predict_y = zip(*prediction)
    observe_x, observe_y = zip(*observation)
    plt.plot(predict_x, predict_y,'b--')
    plt.plot(observe_x, observe_y,'r--')
    plt.legend(['Predicted','Observed'])
    plt.show()
