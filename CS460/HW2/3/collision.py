from matplotlib.patches import Polygon
from robot import Robot
import numpy as np
import sampler
import file_parse
import sys

def isCollisionFree(robot: Robot, point, obstacles):
    robot.set_pose((point[0], point[1], point[2]))
    robot_min = min(robot.width, robot.height)
    robot_points = np.array(robot.transform())
    for p in robot_points:
        if p[0] < robot_min or p[0] > 10-robot_min or p[1] < robot_min or p[1] > 10-robot_min:
            return False
    robot_path = Polygon(np.array(robot_points), True).get_path()

    boundary = Polygon(np.array([(0,0),(10,0),(10,10),(0,10)]), True).get_path()
    if robot_path.intersects_path(boundary, False): return False

    for obs in obstacles:
        obs_path = Polygon(np.array(obs), True).get_path()
        if robot_path.intersects_path(obs_path, True):
            return False

    return True

if __name__ == "__main__":
    map_tuple = file_parse.parse_problem(sys.argv[1], sys.argv[2])
    point = sampler.sample()
    print('Point Sampled:' + str(point))
    print(isCollisionFree(map_tuple[0], point, map_tuple[1]))