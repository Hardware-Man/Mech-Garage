from math import pi
from numpy import random

def sample():
    return (round(random.uniform(0, 10), 2), round(random.uniform(0, 10), 2), round(random.uniform(-pi,pi), 2))

def sample_control():
    return (round(random.uniform(-3, 3), 2), round(random.uniform(-pi/4, pi/4), 2))