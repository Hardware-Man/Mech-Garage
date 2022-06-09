from math import pi, sqrt
import numpy as np
from matplotlib.patches import Polygon
from matplotlib.path import Path
from robot import Robot, pose_dist
from tree import Tree
from sampler import sample
from collision import isCollisionFree

def rrt(robot: Robot, obstacles, start, goal, iter_n):
    vertices = Tree(robot, obstacles, start, goal)
    vertices.data[start] = [None, 0, []]

    i = 0
    while i in range(iter_n):
        q = sample()
        while not isCollisionFree(robot, q, obstacles) or q in vertices.data.keys():
            q = sample()
        near = vertices.nearest(q)
        n2 = pose_dist(near, q)
        if n2 < 3: n2 = 3.0
        if not vertices.extend(near, 0.25, n2, 0.25): continue
        i+=1

    near_goal = vertices.nearest(goal)
    dist_to_goal = pose_dist(near_goal, goal)
    if (dist_to_goal < sqrt(2 + (pi*pi)/16)):
        verts = [(goal[0], goal[1]), (near_goal[0], near_goal[1])]
        codes = [Path.MOVETO, Path.LINETO]
        seg = Path(verts, codes)
        cspace = define_cspace(robot, obstacles)
        for obs in cspace:
            obs_path = Polygon(np.array(obs), True).get_path()
            if obs_path.intersects_path(seg, True):
                return (None, vertices.data)
        vertices.add(near_goal, goal)
    else: return (None, vertices.data)

    solution_path = [goal]
    curr_vert = vertices.parent(goal)
    while curr_vert is not None:
        solution_path.append(curr_vert)
        curr_vert = vertices.parent(curr_vert)
    
    solution_path.reverse()
    return (solution_path, vertices.data)

def cross_prod(p, a, b):
    return (a[0] - p[0]) * (b[1] - p[1]) - (a[1] - p[1]) * (b[0] - p[0])

def define_cspace(robot, obstacles):
    robot_min = min(robot.width, robot.height)/2
    robot_points = [(-robot_min, -robot_min), (-robot_min, robot_min), (robot_min, robot_min), (robot_min, -robot_min)]

    new_obstacles = []

    for obs in obstacles:
        points = []
        for i in obs:
            for j in robot_points:
                points.append((i[0]+j[0],i[1]+j[1]))

        points = sorted(set(points))

        if (len(points) <= 1):
            new_obstacles.append(points)
            continue

        lowHull = []
        for p in points:
            while len(lowHull) >= 2 and cross_prod(lowHull[-2], lowHull[-1], p) <= 0:
                lowHull.pop()
            lowHull.append(p)
        
        upHull = []
        for p in reversed(points):
            while len(upHull) >= 2 and cross_prod(upHull[-2], upHull[-1], p) <= 0:
                upHull.pop()
            upHull.append(p)

        new_obstacles.append(lowHull[:-1] + upHull[:-1])

    robot_len = len(robot_points)
    
    asc_x_sort, asc_y_sort = sorted(robot_points, key=lambda k: [k[0], k[1]]), sorted(robot_points, key=lambda k: [k[1], k[0]])

    low_x, low_y = asc_x_sort[0][0], asc_y_sort[0][1]
    high_x, high_y = asc_x_sort[robot_len-1][0], asc_y_sort[robot_len-1][1]
    
    bound_free = [(0-low_x, 0-low_y),(0-low_x, 10-high_y), (10-high_x,10-high_y), (10-high_x,0-low_y)]
    
    if (low_x < 0): new_obstacles.append([(0,0),bound_free[0],bound_free[1],(0,10)])
    if (low_y < 0): new_obstacles.append([(0,0),bound_free[0],bound_free[3],(10,0)])
    new_obstacles.append([(0,10),bound_free[1],bound_free[2],(10,10)])
    new_obstacles.append([(10,0),bound_free[3],bound_free[2],(10,10)])

    return new_obstacles
