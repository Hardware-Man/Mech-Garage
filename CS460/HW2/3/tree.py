from matplotlib.patches import Polygon
from matplotlib.path import Path
import numpy as np
from robot import Robot, pose_dist
from collision import isCollisionFree
from sampler import sample_control

class Tree:
    def __init__(self, robot: Robot, obstacles, start, goal):
        self.robot = robot
        self.obstacles = obstacles
        from rrt import define_cspace
        self.cspace = define_cspace(robot, obstacles)
        self.start = start
        self.goal = goal
        self.ordered_list = [start]
        self.data = {}
    
    def add(self, point1, point2):
        point2_cost = self.get_cost(point1) + pose_dist(point1,point2)
        
        if self.exists(point2):
            self.data[self.parent(point2)][2].remove(point2)
            self.data[point2][0] = point1
            self.data[point2][1] = point2_cost
        else:
            self.data[point2] = [point1, point2_cost, []]
        
        self.data[point1][2].append(point2)
    
    def exists(self, point):
        return point in self.data
    
    def parent(self, point):
        return self.data[point][0]
    
    def get_cost(self, point):
        return self.data[point][1]
    
    def nearest(self, point):
        if point == self.start: return self.start

        near = self.start
        near_dist = pose_dist(point,near)
        for p in self.data.keys():
            p_near_dist = pose_dist(point,p)
            if p_near_dist < near_dist:
                near = p
                near_dist = p_near_dist

        return near

    def extend(self, point, n1, n2, dt):
        from rrt import define_cspace
        n = n1
        controls = []
        durations = []

        while n < n2:
            controls.append(sample_control())
            durations.append(n)
            n+=dt
        
        states = self.robot.propogate(point, controls, durations, dt)

        results = []

        curr_point = point
        for s in states[1:]:
            if isCollisionFree(self.robot, s, self.obstacles) is True:
                verts = [(curr_point[0],curr_point[1]), (s[0], s[1])]
                codes = [Path.MOVETO, Path.LINETO]
                seg = Path(verts, codes)
                valid = True
                cspace = define_cspace(self.robot, self.obstacles)
                
                for obs in cspace:
                    obs_path = Polygon(np.array(obs), True).get_path()
                    if obs_path.intersects_path(seg, True):
                        valid = False
                
                if valid is False: break
                
                curr_point = s
                results.append(s)

        curr_point = point
        for r in results:
            self.add(curr_point, r)
            curr_point = r

        return results