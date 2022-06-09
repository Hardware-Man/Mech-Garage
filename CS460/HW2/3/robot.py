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
    
    def kinematics(self, state, control):
        return (control[0] + cos(state[2] + pi/2), control[0] + sin(state[2] + pi/2), control[1])
    
    def propogate(self, state, controls, durations, dt):
        def mult_qp_dt(qp, n, dt):
            return tuple(map(lambda x: round(x * n * dt, 2), qp))
        def add_tup(a, b):
            return tuple(map(lambda x, y: x + y, a, b))

        result = [state]
        curr_state = state
        seq_len = len(controls)
        for n in range(seq_len):
            qp = self.kinematics(curr_state, controls[n])
            qp_dt = mult_qp_dt(qp, durations[n], dt)
            curr_state = add_tup(curr_state, qp_dt)
            result.append(curr_state)
        return result
    

def pose_dist(pose1, pose2):
    dx = pose2[0] - pose1[0]
    dy = pose2[1] - pose1[1]
    dtheta = pose2[2] - pose1[2]
    if dtheta > pi: dtheta-=2*pi
    elif dtheta < -pi: dtheta+=2*pi
    return round(sqrt(dx*dx + dy*dy + dtheta*dtheta), 2)
