import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection
from matplotlib.animation import ArtistAnimation, FuncAnimation
import numpy as np
from robot import Robot
import file_parse
from rrt import rrt, define_cspace, rrt_star
import sys

def visualize_problem(robot, obstacles, start, goal):
    fig, ax = plt.subplots()
    plot_setup()

    patches_set = visualize_helper(robot, obstacles, start, goal)
    patch_plotter(ax, [patches_set[0]])

    plt.show()

def visualize_configuration(robot, obstacles, start, goal):
    fig, ax = plt.subplots()
    plot_setup()

    patches_set = visualize_helper(robot, obstacles, start, goal)
    patch_plotter(ax, [patches_set[1]])

    plt.show()

def visualize_points(points, robot, obstacles, start, goal):
    fig, ax = plt.subplots()
    plot_setup()

    patches_set = visualize_helper(robot, obstacles, start, goal)
    patch_plotter(ax, patches_set)

    if points is not None:
        np_points = np.array(points)
        plt.plot(np_points[:,0],np_points[:,1], 'o')
    plt.show()

def visualize_path(robot, obstacles, path):
    fig, ax = plt.subplots()
    plot_setup()

    patches_set = visualize_helper(robot, obstacles, path[0], path[len(path)-1])
    patch_plotter(ax, patches_set)

    anim = solution_visual(fig, ax, robot, path)

    plt.show()

def visualize_rrt(robot, obstacles, start, goal, iter_n):
    fig, ax = plt.subplots()
    plot_setup()

    patches_set = visualize_helper(robot, obstacles, start, goal)
    patch_plotter(ax, patches_set)

    result = rrt(robot, obstacles, start, goal, iter_n)
    t_ani = tree_visual(fig, result[1])
    if result[0] is not None:
        path = np.array(result[0])
        plt.plot(path[:,0],path[:,1])

    plt.show()

    if result[0] is not None: visualize_path(robot, obstacles, result[0])

def visualize_rrt_star(robot, obstacles, start, goal, iter_n):
    fig, ax = plt.subplots()
    plot_setup()

    patches_set = visualize_helper(robot, obstacles, start, goal)
    patch_plotter(ax, patches_set)

    result = rrt_star(robot, obstacles, start, goal, iter_n)
    t_ani = tree_visual(fig, result[1])
    if result[0] is not None:
        path = np.array(result[0])
        plt.plot(path[:,0],path[:,1])
    
    plt.show()

    if result[0] is not None: visualize_path(robot, obstacles, result[0])

def plot_setup():
    plt.xlim(0, 10)
    plt.ylim(0, 10)
    plt.xticks(np.arange(0.0, 11.0, 1.0))
    plt.yticks(np.arange(0.0, 11.0, 1.0))
    plt.grid(True)

def patch_plotter(ax: plt.Axes, patches_set: list):
    patches_set.reverse()
    for patches in patches_set:
        p = PatchCollection(patches)
        p.set_array(np.random.rand(len(patches)))
        ax.add_collection(p)
    return

def solution_visual(fig: plt.figure, ax: plt.Axes, robot: Robot, res_path):
    robot.set_pose(res_path[0])
    np_robot = np.array(robot.transform(), copy=True)
    patch = Polygon(np_robot, True)
    ax.add_patch(patch)

    def init():
        return patch,

    def anim(a):
        robot.set_pose(a)
        anim_robot = robot.transform()
        patch.set_xy(anim_robot)
        return patch,

    if res_path is not None:
        path = np.array(res_path)
        plt.plot(path[:,0],path[:,1])
        return FuncAnimation(fig, anim, res_path, init_func=init, interval=750, blit=True)

    return None

def tree_visual(fig: plt.figure, res_tree: dict):
    imgs = []
    for i in range(len(res_tree.keys())):
        lines = []
        key_list = list(res_tree.keys())
        for p in key_list[0:i+1]:
            for c in res_tree[p][2]:
                x = [p[0],c[0]]
                y = [p[1],c[1]]
                lines.extend(plt.plot(x,y,'r--'))
        imgs.append(lines)
    return ArtistAnimation(fig, imgs, interval=50, repeat=False)

def visualize_helper(robot: Robot, obstacles, start, goal):
    patches = []
    patches2 = []

    cspace = define_cspace(robot, obstacles)

    for obs in obstacles:
        patches.append(Polygon(np.array(obs), True))

    robot.set_pose((start[0], start[1], start[2]))
    robot_start = np.array(robot.transform())
    patches.append(Polygon(robot_start, True))

    robot.set_pose((goal[0], goal[1], goal[2]))
    robot_end = np.array(robot.transform())
    patches.append(Polygon(robot_end, True))

    for cobs in cspace:
        patches2.append(Polygon(np.array(cobs), True))

    return [patches, patches2]

if __name__ == "__main__":
    map_tuple = file_parse.parse_problem(sys.argv[1], sys.argv[2])
    robot = map_tuple[0]
    obstacles = map_tuple[1]
    start = map_tuple[2][0][0]
    goal = map_tuple[2][0][1]
    iter_n = int(sys.argv[3])

    if iter_n == 0:
        visualize_problem(robot, obstacles, start, goal)
    else:
        # successes = 0
        # for i in range(10): 
        #     successes+=visualize_rrt_star(robot, obstacles, start, goal, iter_n)
        # print("Successes: ", successes)

        visualize_rrt(robot, obstacles, start, goal, iter_n)
