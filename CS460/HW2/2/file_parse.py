import sys
from robot import Robot

def parse_problem(world_file, problem_file):
    world_data = open(world_file)
    problem_data = open(problem_file)

    world_list = []
    problem_list = []

    robot_dimensions = world_data.readline().split(' ')
    robot = Robot(float(robot_dimensions[0]), float(robot_dimensions[1]))

    for curr_obj in world_data:        
        coords = curr_obj.split(' ')
        
        curr_obj = []
        for i in range(0, len(coords), 2):
            curr_obj.insert(0,(float(coords[i]), float(coords[i+1])))

        world_list.append(curr_obj)

    for path in problem_data:
        coords = path.split(' ')

        problem_list.append([
            (float(coords[0]), float(coords[1]), float(coords[2])),
            (float(coords[3]), float(coords[4]), float(coords[5]))])

    world_data.close()
    problem_data.close()

    return (robot, world_list, problem_list)

if __name__ == "__main__":
    result = parse_problem(sys.argv[1], sys.argv[2])
    if result is not None:
        print('Robot:', (result[0].width, result[0].height))
        for x in range(len(result[1])):
            print('Obstacle ' + str(x) + ': ' + str(result[1][x]))
        for x in range(len(result[2])):
            print('Problem ' + str(x) + ': ' + str(result[2][x]))