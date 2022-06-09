from matplotlib.patches import Polygon
import numpy as np
from matplotlib.path import Path

class Tree:
    def __init__(self, robot, obstacles, start, goal):
        self.robot = robot[0]
        self.obstacles = obstacles
        self.start = start
        self.goal = goal
        self.ordered_list = [start]
        self.data = {}
    
    def add(self, point1, point2):
        point2_cost = self.get_cost(point1) + round(np.math.dist(point1,point2), 2)
        
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

    def rewire(self, point, r):
        near_neighbors = []

        all_points = list(self.data.keys())
        all_points.remove(point)
        
        for p in all_points:
            p_dist = round(np.math.dist(point,p), 2)
            if p_dist <= r:
                near_neighbors.append(p)

        optimal_neighbor = self.parent(point)
        optimal_cost = self.get_cost(point)
        for n in near_neighbors:
            if path_intersection(self.obstacles, point, n) is None:
                n_cost = round(np.math.dist(point,n), 2) + self.get_cost(n)
                if n_cost < optimal_cost:
                    optimal_neighbor = n
                    optimal_cost = n_cost

        self.add(optimal_neighbor, point)

        point_cost = self.get_cost(point)
        for n in near_neighbors:
            if path_intersection(self.obstacles, point, n) is None:
                n_cost = self.get_cost(n)
                if (point_cost + round(np.math.dist(point,n), 2)) < n_cost:
                    self.add(point,n)
    
    def nearest(self, point):
        if point == self.start: return self.start

        near = self.start
        near_dist = np.math.dist(point,near)
        for p in self.data.keys():
            p_near_dist = np.math.dist(point,p)
            if p_near_dist < near_dist:
                near = p
                near_dist = p_near_dist

        return near

    def extend(self, point1, point2):
        near_p1 = path_intersection(self.obstacles, point1, point2)

        if near_p1 is None:
            self.add(point1, point2)
            return point2
        else:
            if near_p1 == point1: return None
            verts = [point1, near_p1]
            codes = [Path.MOVETO, Path.LINETO]
            seg = Path(verts, codes)
            for obs in self.obstacles:
                obs_path = Polygon(np.array(obs), True).get_path()
                if obs_path.intersects_path(seg, True): return None
            self.add(point1, near_p1)
            return near_p1
    
def seg_intersect(seg1, seg2):
    def deter(n, m):
        return n[0] * m[1] - m[0] * n[1]
    
    a = (seg1[1][0]-seg1[0][0], seg1[1][1]-seg1[0][1])
    b = (seg2[1][0]-seg2[0][0], seg2[1][1]-seg2[0][1])
    c = (seg1[0][0]-seg2[0][0], seg1[0][1]-seg2[0][1])
    denom = deter(a, b)
    if denom == 0: return None
    u = deter(a,c)/denom
    v = deter(b,c)/denom
    result = None
    if u >= 0 and u <=1 and v >= 0 and v <= 1:
        result = (round(seg1[0][0]+(v*a[0]), 2), round(seg1[0][1]+(v*a[1]), 2))
    return result

def path_intersection(obstacles, point1, point2):
    near_p1 = None
    near_dist = None
    for obs in obstacles:
        circ_obs = obs + [obs[0]]
        obs_range = range(len(circ_obs)-1)
        for i in obs_range:
            inter = seg_intersect((point1, point2), (circ_obs[i], circ_obs[i+1]))
            if inter is not None:
                if near_p1 is None:
                    near_p1 = inter
                    near_dist = round(np.math.dist(point1, inter), 2)
                    continue
                i_dist = round(np.math.dist(point1, inter), 2)
                if i_dist < near_dist:
                    near_p1 = inter
                    near_dist = i_dist
    
    return near_p1