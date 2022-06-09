import numpy as np
from tree import Tree
from sampler import sample
from collision import isCollisionFree

def rrt(robot, obstacles, start, goal, iter_n):
    cspace = define_cspace(robot, obstacles)
    vertices = Tree(robot, cspace, start, goal)
    vertices.data[start] = [None, 0, []]

    i = 0
    while i in range(iter_n):
        q = sample()
        while not isCollisionFree(robot, q, obstacles) or q in vertices.data.keys():
            q = sample()
        near = vertices.nearest(q)
        if vertices.extend(near, q) == None: continue
        i+=1
    
    near_goal = vertices.nearest(goal)
    vertices.extend(near_goal, goal)

    if not vertices.exists(goal): return (None, vertices.data)

    solution_path = [goal]
    curr_vert = vertices.parent(goal)
    while curr_vert is not None:
        solution_path.append(curr_vert)
        curr_vert = vertices.parent(curr_vert)
    
    solution_path.reverse()
    return (solution_path, vertices.data)

def rrt_star(robot, obstacles, start, goal, iter_n):

    cspace = define_cspace(robot, obstacles)
    vertices = Tree(robot, cspace, start, goal)
    vertices.data[start] = [None, 0, []]

    i = 0
    while i in range(iter_n):
        q = sample()
        while not isCollisionFree(robot, q, obstacles) or q in vertices.data.keys():
            q = sample()
        near = vertices.nearest(q)
        new_vert = vertices.extend(near, q)
        if new_vert == None: continue
        vertices.rewire(new_vert, np.math.dist(near,new_vert)+5)
        i+=1

    near_goal = vertices.nearest(goal)
    vertices.extend(near_goal, goal)

    if not vertices.exists(goal): return (None, vertices.data)

    vertices.rewire(goal, np.math.dist(goal,near_goal)+5)

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
    invert_bot = [(-p[0], -p[1]) for p in robot]

    new_obstacles = []

    for obs in obstacles:
        points = []
        for i in obs:
            for j in invert_bot:
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

    robot_len = len(robot)
    
    asc_x_sort, asc_y_sort = sorted(robot, key=lambda k: [k[0], k[1]]), sorted(robot, key=lambda k: [k[1], k[0]])

    low_x, low_y = asc_x_sort[0][0], asc_y_sort[0][1]
    high_x, high_y = asc_x_sort[robot_len-1][0], asc_y_sort[robot_len-1][1]
    
    bound_free = [(0-low_x, 0-low_y),(0-low_x, 10-high_y), (10-high_x,10-high_y), (10-high_x,0-low_y)]
    
    if (low_x < 0): new_obstacles.append([(0,0),bound_free[0],bound_free[1],(0,10)])
    if (low_y < 0): new_obstacles.append([(0,0),bound_free[0],bound_free[3],(10,0)])
    new_obstacles.append([(0,10),bound_free[1],bound_free[2],(10,10)])
    new_obstacles.append([(10,0),bound_free[3],bound_free[2],(10,10)])

    return new_obstacles
