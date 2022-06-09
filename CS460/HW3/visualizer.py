from math import pi, cos, sin, atan2, sqrt
import matplotlib.pyplot as plt
from numpy.random import normal
from numpy import array
from random import uniform
import sys

def generate_landmarks(n, id):
    f = open('landmark_' + id + '.txt', "w")

    f.write(str(n)+'\n')

    def sample_point():
        return [round(uniform(0, 100), 2), round(uniform(0, 100), 2)]
    
    landmarks = []
    for i in range(n):
        l = sample_point()
        landmarks.append(l)
        f.write(' '.join(map(str, l))+'\n')

    return landmarks

def parse_landmarks(landmark_file):
    lines = [line.rstrip('\n') for line in landmark_file]

    landmarks = []
    for line in lines[1:]:
        landmarks.append([float(d) for d in line.split(' ')])
    
    return landmarks, int(lines[0])

def generate_ground_truths(k, id):
    trjs = [[0,0,0]]

    f = open('ground_truth_' + id + '.txt', "w")

    f.write(str(k+1)+'\n')
    f.write(' '.join(map(str, trjs[0]))+'\n')

    i = 0
    while i < k:
        u = [round(uniform(-pi/12,pi/12), 2), round(uniform(-pi/12,pi/12), 2), round(uniform(0, 5), 2)]

        x1 = round(trjs[i][0] + u[2] * cos(trjs[i][2] + u[0]), 2)
        if x1 > 100 or x1 < 0:
            continue
        y1 = round(trjs[i][1] + u[2] * sin(trjs[i][2] + u[0]), 2)
        if y1 > 100 or y1 < 0:
            continue
        theta1 = round(trjs[i][2] + u[0] + u[1], 2)

        trjs.append([x1, y1, theta1])
        f.write(' '.join(map(str, [x1, y1, theta1]))+'\n')
        i+=1

    return trjs

def parse_ground_truths(ground_truths_file):
    lines = [line.rstrip('\n') for line in ground_truths_file]

    trjs = []
    for line in lines[1:]:
        trjs.append([float(d) for d in line.split(' ')])

    return trjs, int(lines[0])-1

def compute_odometry(trjs):
    odoms = []
    odoms_h = []
    for i in range(len(trjs)-1):
        dx = trjs[i+1][0] - trjs[i][0]
        dy = trjs[i+1][1] - trjs[i][1]
        t0, t1 = trjs[i][2], trjs[i+1][2]
        rot1 = atan2(dy, dx) - t0
        trans = sqrt(dx**2 + dy**2)
        rot2 = t1 - t0 - rot1

        odoms.append([round(rot1, 2), round(rot2, 2), round(trans, 2)])

        rot1_h = round(normal(rot1, 0.05), 2)
        rot2_h = round(normal(rot2, 0.05), 2)
        trans_h = round(normal(trans, 0.1), 2)
        odoms_h.append([rot1_h, rot2_h, trans_h])

    return odoms, odoms_h

def compute_bearings(landmarks, trjs):
    bearings = []
    bearings_h = []
    for trj in trjs:
        z = []
        z_h = []
        for l in list(landmarks):
            b = atan2(l[1]-trj[1], l[0]-trj[0]) - trj[2]

            z.append(round(b, 2))

            b_h = round(normal(b, 0.0523599), 2)
            z_h.append(b_h)

        bearings.append(z)
        bearings_h.append(z_h)

    return bearings, bearings_h

def generate_measurements(id, landmarks_file, ground_truths_file):
    landmarks, n = parse_landmarks(landmarks_file)
    ground_truths, k = parse_ground_truths(ground_truths_file)
    odoms, odoms_h = compute_odometry(ground_truths)
    bearings, bearings_h = compute_bearings(landmarks, ground_truths[1:])

    f = open('measurements_' + id + '.txt', "w")
    f.write(' '.join(map(str, ground_truths[0]))+'\n')
    act_k = int(k) - 1
    f.write(str(act_k)+'\n')
    for i in range(act_k):
        f.write(' '.join(map(str, odoms_h[i]))+'\n')
        f.write(' '.join(map(str, bearings_h[i]))+'\n')
    f.close()

    return odoms, odoms_h, bearings, bearings_h

def visualize_landmarks(landmarks, n):
    land_x, land_y = zip(*landmarks)
    plt.xlim(0, 100)
    plt.ylim(0, 100)
    plt.title('N = ' + str(n))
    plt.plot(land_x, land_y, 'bo')
    plt.show()

def visualize_ground_truths(trjs, k):
    trjs_x, trjs_y, trjs_theta = zip(*trjs)
    trjs_u = [cos(theta) for theta in list(trjs_theta)]
    trjs_v = [sin(theta) for theta in trjs_theta]
    plt.xlim(0, 100)
    plt.ylim(0, 100)
    plt.title('K = ' + str(k))
    plt.quiver(trjs_x,trjs_y,trjs_u,trjs_v, color=['r'])
    plt.show()

def visualize_measure_diffs(odoms, odoms_h, bearings, bearings_h, id):
    rot1, rot2, trans = zip(*odoms)
    rot1_h, rot2_h, trans_h = zip(*odoms_h)
    b_diff = array(bearings) - array(bearings_h)
    num_b = len(b_diff[0])
    avg_b_diff = [sum(b)/num_b for b in b_diff]
    sample_num = [i for i in range(len(odoms))]

    plt.suptitle('ID = ' + id)
    plt.subplot(2, 2, 1)
    plt.title(f'δrot1 Difference')
    plt.plot(sample_num, array(rot1)-array(rot1_h), color='r')
    plt.subplot(2, 2, 2)
    plt.title(f'δrot2 Difference')
    plt.plot(sample_num, array(rot2)-array(rot2_h), color='g')
    plt.subplot(2, 2, 3)
    plt.title(f'δtrans Difference')
    plt.plot(sample_num, array(trans)-array(trans_h), color='b')
    plt.subplot(2, 2, 4)
    plt.title("Avg b Difference")
    plt.plot(sample_num, avg_b_diff, color='tab:orange')
    plt.tight_layout()
    plt.subplots_adjust(top=0.88)
    plt.show()

if __name__ == "__main__":
    # n = 300
    
    odoms, odoms_h, bearings, bearings_h = generate_measurements(sys.argv[1], open(sys.argv[2]), open(sys.argv[3]))

    visualize_measure_diffs(odoms, odoms_h, bearings, bearings_h, sys.argv[1])
