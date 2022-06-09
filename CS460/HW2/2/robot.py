from math import pi, sin, cos, sqrt

class Robot:
    def __init__(self, width: float, height: float):
        self.width = width
        self.height = height
        self.translation = (0.0,0.0)
        self.rotation = 0.0
    
    def set_pose(self, pose):
        self.translation = (pose[0],pose[1])
        self.rotation = pose[2]
    
    def transform(self):
        x = self.translation[0]
        y = self.translation[1]
        w = self.width
        h = self.height
        theta = self.rotation

        base = [(-w/2,-h/2), (-w/2,h/2), (w/2,h/2), (w/2,-h/2)]

        result = []

        for p in base:
            newX = x + cos(theta)*p[0] - sin(theta)*p[1]
            newY = y + sin(theta)*p[0] + cos(theta)*p[1]
            result.append((newX, newY))

        return result
    

def pose_dist(pose1, pose2):
    dx = pose2[0] - pose1[0]
    dy = pose2[1] - pose1[1]
    dtheta = pose2[2] - pose1[2]
    if dtheta > pi: dtheta-=2*pi
    elif dtheta < -pi: dtheta+=2*pi
    return round(sqrt(dx*dx + dy*dy + dtheta*dtheta), 2)
