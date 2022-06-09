import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from matplotlib.collections import PatchCollection
from matplotlib.animation import ArtistAnimation, FuncAnimation
import numpy as np
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
    patch_plotter(ax, [patches_set[0]])

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

def solution_visual(fig: plt.figure, ax: plt.Axes, robot, res_path):
    np_robot = np.array(robot, copy=True)
    patch = Polygon(np_robot, True)
    ax.add_patch(patch)

    def init():
        return patch,

    def anim(a):
        anim_robot = []
        for r in robot:
            anim_robot.append((r[0]+a[0], r[1]+a[1]))
        patch.set_xy(anim_robot)
        return patch,

    if res_path is not None:
        path = np.array(res_path)
        plt.plot(path[:,0],path[:,1])
        return FuncAnimation(fig, anim, res_path, init_func=init, interval=1000, blit=True)

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
    return ArtistAnimation(fig, imgs, interval=100, repeat=False)

def visualize_helper(robot, obstacles, start, goal):
    patches = []
    patches2 = []

    cspace = define_cspace(robot, obstacles)

    for obs in obstacles:
        patches.append(Polygon(np.array(obs), True))

    robot_start = []
    for i in robot:
        robot_start.append([i[0]+start[0], i[1]+start[1]])
    patches.append(Polygon(np.array(robot_start), True))

    robot_goal = []
    for i in robot:
        robot_goal.append([i[0]+goal[0], i[1]+goal[1]])
    patches.append(Polygon(np.array(robot_goal), True))

    for cobs in cspace:
        patches2.append(Polygon(np.array(cobs), True))

    return [patches, patches2]

if __name__ == "__main__":
    map_tuple = file_parse.parse_problem(sys.argv[1], sys.argv[2])
    robot = map_tuple[0]
    obstacles = map_tuple[1]
    start = map_tuple[2][2][0]
    goal = map_tuple[2][2][1]
    iter_n = int(sys.argv[3])

    if iter_n == 0: visualize_problem(robot, obstacles, start, goal)
    else:
        visualize_rrt_star(robot, obstacles, start, goal, iter_n)
        # successes = 0
        # for i in range(10): 
        #     successes+=visualize_rrt_star(robot, obstacles, start, goal, iter_n)
        # print("Successes: ", successes)
