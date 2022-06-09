from matplotlib.patches import Polygon
import numpy as np
import sampler
import file_parse
import sys

def isCollisionFree(robot, point, obstacles):
    robot_point = []
    for i in robot:
        robot_point.append([i[0]+point[0], i[1]+point[1]])
    robot_path = Polygon(np.array(robot_point), True).get_path()

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