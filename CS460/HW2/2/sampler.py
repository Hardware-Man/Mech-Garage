from math import pi
from numpy import random

def sample():
    return (round(random.uniform(0, 10), 2), round(random.uniform(0, 10), 2), round(random.uniform(-pi,pi), 2))