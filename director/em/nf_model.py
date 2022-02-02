from tokenize import String
from typing import Tuple, List
from igraph import *
from igraph import plot


class State:
    def __init__(self, state_name, properties,registration_id):
        # if the state is shared, it is shared among all the instances
        self.name = state_name
        self.shared = False
        if ("shared" in properties):
            self.shared = True

        if (self.shared == True):
            self.color = "slate grey"
        else:
            self.color = "blue"
        if ("system" in properties):
            self.color = "black"

        self.shape = "triangle-up"
        self.registration_id = registration_id


# Nfunctor are self-contained unit of state accessing
# Action are self-contained process of in terms of the state

class Action:
    def __init__(self, action_name, properties,
                 dependencies_action_list, state, read_only_local_state_list=[], registration_id=0):
        self.registration_id = registration_id
        self.name = action_name
        self.per_packet_operation = False
        if ("data_operation" in properties):
            self.per_packet_operation = True

        self.dependencies_action_list = dependencies_action_list
        self.state = state
        self.nf = None  # initialized when the NF is defined
        self.read_only_state_list = read_only_local_state_list
        self.subsequent_action = []
        # the order of subsequent action depends on the order that they are declared
        for dependency_action in self.dependencies_action_list:
            dependency_action.subsequent_action.append(self)
        self.shape = "circle"
        if self.per_packet_operation == True:
            self.color = "red"
        else:
            self.color = "yellow"

    def depends(self, action):
        return action in self.dependencies_action_list

    def is_packet_in(self):
        return self.name == "packet_in"

    def is_packet_out(self):
        return self.name == "packet_out"

    def __repr__(self):
        rep = self.name
        return rep


# the first action, recieves packet
# the last action, sends packet
# NFs are self-contained process of packet processing
#
class Nf:
    def __init__(self, nf_name: str, action_list: List[Action], paths =[]):
        self.paths = paths
        self.name = nf_name
        # self.state_action_pair = action_state_pair_list
        self.action_n = len(action_list)
        self.action_list = action_list
        for action in self.action_list:
            action.nf = self

        self.first_actions = []
        self.last_actions_before_packet_out = []

        self.state_list = []
        self.label_list = []
        self.available_color = []
        self.visual_style = {"vertex_label": [], "vertex_color": [], "edge_color": [], "vertex_shape": []}
        # self.state_instance_execution_no_to_vertex_id.setdefault(
        #     self.state_instance_execution_list.index(temp_state_execution), []).append(
        #     self.action_instance_execution_list.index(i))
        for i in list(known_colors)[::10]:
            self.available_color.append(i)

        for action in self.action_list:
            if action.state not in self.state_list and action.state != None:
                self.state_list.append(action.state)

        for action in self.action_list:
            self.visual_style["vertex_label"].append(action.name)
            self.visual_style["vertex_color"].append(action.color)
            self.visual_style["vertex_shape"].append(action.shape)

        for state in self.state_list:
            self.visual_style["vertex_label"].append(state.name)
            self.visual_style["vertex_color"].append(state.color)
            self.visual_style["vertex_shape"].append(state.shape)

        self.state_n = len(self.state_list)
        self.dependency_graph = Graph(directed=True)
        self.dependency_graph.add_vertices(self.state_n + self.action_n)

        for action in self.action_list:
            if (action.state != None):
                start = self.action_list.index(action)
                end = self.state_list.index(action.state) + len(self.action_list)
                self.dependency_graph.add_edges([(start, end)])
                self.visual_style["edge_color"].append("grey")

        temp_list = []
        for action in self.action_list:
            for previous_action in action.dependencies_action_list:
                if previous_action in self.action_list:
                    start = self.action_list.index(previous_action)
                    end = self.action_list.index(action)
                    if (start, end) not in temp_list:
                        self.dependency_graph.add_edges([(start, end)])
                        temp_list.append((start, end))
                        self.visual_style["edge_color"].append("black")

        for action in self.action_list:
            for temp_action in action.dependencies_action_list:
                if temp_action.name == "packet_in":
                    self.first_actions.append(action)
                    break

        for action in self.action_list:
            if action.name == "packet_out":
                for i in self.action_list:
                    if i in action.dependencies_action_list:
                        self.last_actions_before_packet_out.append(i)

    def get_path_id(self, src_action, dst_action):
        return self.paths.index((src_action.name,dst_action.name))

    def associated_state(self, action):
        pass

    def associated_action(self, state):
        pass

    def __repr__(self):
        rep = self.name
        return rep

    def plot(self, layout="tree", vertex_size=20, bbox=(400, 200), margin=50):
        l = self.dependency_graph.layout(layout)
        return plot(self.dependency_graph, layout=l, vertex_size=vertex_size, bbox=bbox, **self.visual_style,
                    margin=margin)


class State_instance:
    def __init__(self, state, nf_id):
        self.state = state
        self.nf_id = nf_id
        self.name = state.name + " nf_id:" + str(self.nf_id)
        self.color = state.color
        self.shape = state.shape
        self.action_instance_list = []  # the set of action_instance that has access to it


class Action_instance:
    def __init__(self, action: Action, nf_id: int, previous_actions=[]):
        self.action = action
        self.nf_id = nf_id
        self.name = action.name + " nf_id:" + str(self.nf_id)
        self.state_instance = None
        self.dependency_list = []
        for i in previous_actions:
            self.dependency_list.append(i)

        self.color = action.color
        self.shape = action.shape
        # self.state_instance = state_instance

    def __repr__(self):
        rep = self.name
        return rep


class Sfc:
    l = []

    def __init__(self, nf_list: List[Nf]):
        self.l = nf_list
        self.total_action = 0
        self.total_state = 0
        self.nf_instance_list = []
        self.action_instance_list = []
        self.state_instance_list = []
        self.visual_style = {}
        self.visual_style["vertex_label"] = []
        self.visual_style["vertex_color"] = []
        self.visual_style["edge_color"] = []
        self.visual_style["vertex_shape"] = []

        nf_id = 0
        temp_nf = None
        for i, nf in enumerate(nf_list):
            if i == 0:
                previous_actions = []
            else:
                previous_actions = temp_nf.last_action_instances

            if i == len(nf_list) - 1:
                last_actions = True
            else:
                last_actions = False

            temp_nf = Nf_instance(nf, nf_id, previous_actions, last_actions)
            self.nf_instance_list.append(temp_nf)
            nf_id = nf_id + 1

        for nf_instance in self.nf_instance_list:
            for action_instance in nf_instance.action_instance_list:
                self.action_instance_list.append(action_instance)

        for nf_instance in self.nf_instance_list:
            for state_instance in nf_instance.state_instance_list:
                self.state_instance_list.append(state_instance)

        # assigning label
        for action_instance in self.action_instance_list:
            self.visual_style["vertex_label"].append(action_instance.name)
            self.visual_style["vertex_color"].append(action_instance.color)
            self.visual_style["vertex_shape"].append(action_instance.shape)

        for state_instance in self.state_instance_list:
            self.visual_style["vertex_label"].append(state_instance.name)
            self.visual_style["vertex_color"].append(state_instance.color)
            self.visual_style["vertex_shape"].append(state_instance.shape)

        self.action_n = len(self.action_instance_list)
        self.state_n = len(self.state_instance_list)
        self.dependency_graph = Graph(directed=True)
        self.dependency_graph.add_vertices(self.state_n + self.action_n)

        for action_instance in self.action_instance_list:
            for state_instance in self.state_instance_list:
                if action_instance.nf_id == state_instance.nf_id:
                    if action_instance.action.state == state_instance.state:
                        start = self.action_instance_list.index(action_instance)
                        end = self.state_instance_list.index(state_instance) + len(self.action_instance_list)
                        self.dependency_graph.add_edges([(start, end)])
                        self.visual_style["edge_color"].append("grey")

        for action_instance_1 in self.action_instance_list:
            for action_instance_2 in self.action_instance_list:
                if action_instance_1 in action_instance_2.dependency_list:
                    start = self.action_instance_list.index(action_instance_1)
                    end = self.action_instance_list.index(action_instance_2)
                    self.dependency_graph.add_edges([(start, end)])
                    self.visual_style["edge_color"].append("black")

    def plot(self, layout="auto", vertex_size=20, size=400, margin=50):
        bbox = (size, size)
        l = self.dependency_graph.layout(layout)
        return plot(self.dependency_graph, layout=l, vertex_size=vertex_size, bbox=bbox, **self.visual_style,
                    margin=margin)


# only meaningful when in the context of SFC
class Nf_instance:
    def __init__(self, nf: Nf, nf_id, previous_nf_last_actions=None, last_action=True):
        if previous_nf_last_actions is None:
            previous_nf_last_actions = []
        self.nf = nf
        self.id = nf_id
        self.name = nf.name + "_" + str(nf_id)
        self.action_instance_list = []
        self.first_action_instances = []
        self.state_instance_list = []
        # get all the actions whose previous actions is "pacekt_in"
        for action in nf.first_actions:
            action_instance = Action_instance(action, nf_id, previous_nf_last_actions)
            self.first_action_instances.append(action_instance)

        for action in nf.action_list:
            if previous_nf_last_actions:
                if action.is_packet_in():
                    continue
            if not last_action:
                if action.is_packet_out():
                    continue
            if action in nf.first_actions:
                for a_i in self.first_action_instances:
                    if action == a_i.action:
                        self.action_instance_list.append(a_i)
                        break
            else:
                self.action_instance_list.append(Action_instance(action, nf_id))

        for state in nf.state_list:
            temp_state_instance = State_instance(state, nf_id)
            self.state_instance_list.append(temp_state_instance)
            for action_instance in self.action_instance_list:
                if action_instance.action.state == state:
                    action_instance.state_instance = temp_state_instance
                    if action_instance not in temp_state_instance.action_instance_list:
                        temp_state_instance.action_instance_list.append(action_instance)

        for action_instance_1 in self.action_instance_list:
            for action_instance_2 in self.action_instance_list:
                if action_instance_2.action in action_instance_1.action.dependencies_action_list:
                    if action_instance_1.nf_id == action_instance_2.nf_id:
                        if action_instance_2 not in action_instance_1.dependency_list:
                            action_instance_1.dependency_list.append(action_instance_2)
                            # print(action_instance_1,action_instance_2)

        self.last_action_instances = self.get_packet_out_previous_actions()

    def first_action_instance(self):
        return self.action_instance_list[0]

    def last_action_instance(self):
        return self.action_instance_list[-1]

    def get_packet_out_previous_actions(self):
        action_list = []
        for action_instance in self.action_instance_list:
            if action_instance.action in self.nf.last_actions_before_packet_out:
                action_list.append(action_instance)

        return action_list


class State_instance_execution:
    def __init__(self, state_instance: State_instance, execution_id):
        self.state_instance = state_instance
        self.nf_id = self.state_instance.nf_id
        self.execution_id = execution_id
        self.name = state_instance.name + " ex:" + str(execution_id)
        self.core_id = -1
        self.a_i_e_list = []
        self.num_buckets = -1
        self.starting_bucket = -1
        self.first_layer = False

        self.color = state_instance.color
        self.shape = state_instance.shape

    def assign_core(self, core_id):
        for a_i_e in self.a_i_e_list:
            a_i_e.assign_core(core_id)
        self.core_id = core_id

    def assign_buckets(self, starting_bucket, num_buckets):
        self.starting_bucket = starting_bucket
        self.num_buckets = num_buckets

    def __repr__(self):
        rep = self.name
        temp = " "
        for i in self.a_i_e_list:
            temp = temp + i.name + " "
        return str(rep) + " associated_actions: " + temp


class Action_instance_execution:
    def __init__(self, a_i: Action_instance, execution_id):
        self.action_instance = a_i
        self.name = a_i.name + " ex:" + str(execution_id)
        # self.name = a_i.action.name
        self.nf_id = self.action_instance.nf_id
        self.execution_id = execution_id
        self.nf_instance_execution = None
        self.state_instance_execution = None
        self.dependency_list = []
        # 10 possible path
        # for each path, maximum 24 instances
        self.path_list = []
        for i in range(10):
            temp = []
            for j in range(24):
                temp.append(-1)
            self.path_list.append(temp)
        self.first_layer = False
        self.last_layer = False
        self.core_id = -1
        self.shape = a_i.shape
        self.color = a_i.color

        self.num_buckets = -1
        self.starting_bucket = -1
        # self.state_instance_execution = state_instance_execution(a_i.state_instance, execution_id)

    def __repr__(self):
        rep = self.name
        return str(rep)

    def assign_core(self, core_id):
        self.core_id = core_id
        self.state_instance_execution.core_id = core_id

    # path_id is the relative address that is defined in the network function
    # needs to translate it into the absolute address that is the id of the action_id
    def add_path(self, path_id, action_id):
        temp_list = self.path_list[path_id]
        index = temp_list.index(-1)
        temp_list[index] = action_id


class Nf_instance_execution:
    def __init__(self, nf_i: Nf_instance, scaling_plan: List[int]):
        self.nf_instance = nf_i
        self.name = nf_i.name + "_execution"
        self.action_instance_execution_list = []
        self.state_instance_execution_list = []
        self.visual_style = {}
        self.visual_style["vertex_label"] = []
        self.visual_style["vertex_color"] = []
        self.visual_style["edge_color"] = []
        self.visual_style["vertex_shape"] = []

        # create a_i_e
        for action_instance in nf_i.action_instance_list:
            execution_id = 0
            if (action_instance.action.per_packet_operation == True):
                n = scaling_plan[0]
            else:
                n = scaling_plan[1]

            for i in range(n):
                temp_action_instance_execution = Action_instance_execution(action_instance, execution_id)
                self.action_instance_execution_list.append(temp_action_instance_execution)
                execution_id = execution_id + 1

        # connect a_i_e
        for i in range(len(self.nf_instance.action_instance_list)):
            current_action_instance = self.nf_instance.action_instance_list[i]
            for previous_action_instance in current_action_instance.dependency_list:
                layer_previous_action_instance_execution = []
                layer_current_action_instance_execution = []
                for i in self.action_instance_execution_list:
                    if (i.action_instance == previous_action_instance):
                        layer_previous_action_instance_execution.append(i)
                    if (i.action_instance == current_action_instance):
                        layer_current_action_instance_execution.append(i)

                if (len(layer_previous_action_instance_execution) > len(layer_current_action_instance_execution)):
                    for a_i_e in layer_previous_action_instance_execution:
                        index = layer_previous_action_instance_execution.index(a_i_e)
                        current_index = index % len(layer_current_action_instance_execution)
                        layer_current_action_instance_execution[current_index].dependency_list.append(
                            layer_previous_action_instance_execution[index])
                else:
                    for a_i_e in layer_current_action_instance_execution:
                        a_i_e.dependency_list.append(layer_previous_action_instance_execution[
                                                         layer_current_action_instance_execution.index(a_i_e) % len(
                                                             layer_previous_action_instance_execution)])

        # create s_i_e
        for state_instance in nf_i.state_instance_list:
            execution_id = 0
            for action_instance_execution in self.action_instance_execution_list:
                if action_instance_execution.action_instance.state_instance == state_instance:
                    if (action_instance_execution.action_instance.action.per_packet_operation == True):
                        n = scaling_plan[0]
                    else:
                        n = scaling_plan[1]

                    # if not state_instance.state.shared:
                    for i in range(n):
                        temp_state_instance_execution = State_instance_execution(state_instance, execution_id)
                        self.state_instance_execution_list.append(temp_state_instance_execution)
                        execution_id = execution_id + 1
                    break
                    # else:
                    #     temp_state_instance_execution = State_instance_execution(state_instance, 0)
                    #     self.state_instance_execution_list.append(temp_state_instance_execution)
                    #     break

        # connect a_i_e and s_i_e
        for a_i_e in self.action_instance_execution_list:
            for s_i_e in self.state_instance_execution_list:
                # if not s_i_e.state_instance.state.shared:
                if a_i_e.execution_id == s_i_e.execution_id and a_i_e.action_instance.state_instance == s_i_e.state_instance:
                    a_i_e.state_instance_execution = s_i_e
                    s_i_e.a_i_e_list.append(a_i_e)
                # else:
                #     if a_i_e.action_instance.state_instance == s_i_e.state_instance:
                #         a_i_e.state_instance_execution = s_i_e

        self.action_n = len(self.action_instance_execution_list)
        self.state_n = len(self.state_instance_execution_list)
        self.dependency_graph = Graph(directed=True)
        self.dependency_graph.add_vertices(self.action_n + self.state_n)

        # assigning label
        for a_i_e in self.action_instance_execution_list:
            self.visual_style["vertex_label"].append(a_i_e.name)
            self.visual_style["vertex_shape"].append(a_i_e.shape)
            self.visual_style["vertex_color"].append(a_i_e.color)

        for s_i_e in self.state_instance_execution_list:
            self.visual_style["vertex_label"].append(s_i_e.name)
            self.visual_style["vertex_shape"].append(s_i_e.shape)
            self.visual_style["vertex_color"].append(s_i_e.color)

        for a_i_e in self.action_instance_execution_list:
            start = self.action_instance_execution_list.index(a_i_e)
            # print(a_i_e)
            end = self.state_instance_execution_list.index(a_i_e.state_instance_execution) + self.action_n
            self.dependency_graph.add_edges([(start, end)])
            self.visual_style["edge_color"].append("grey")

        for a_i_e in self.action_instance_execution_list:
            for a_i_e_2 in self.action_instance_execution_list:
                if a_i_e_2 in a_i_e.dependency_list:
                    start = self.action_instance_execution_list.index(a_i_e_2)
                    end = self.action_instance_execution_list.index(a_i_e)
                    self.dependency_graph.add_edges([(start, end)])
                    self.visual_style["edge_color"].append("black")

    def is_successor(self, id_1, id_2):
        return id_2 in self.dependency_graph.neighbors(id_1, mode='out')

    def first_layer_action_instance_execution(self):
        first_action_instance = self.nf_instance.first_action_instance()
        temp_result = [a_i_e for a_i_e in self.action_instance_execution_list if
                       a_i_e.action_instance == first_action_instance]
        return temp_result

    def last_layer_instance_execution(self):
        last_action_instance = self.nf_instance.last_action_instance()
        temp_result = [a_i_e for a_i_e in self.action_instance_execution_list if
                       a_i_e.action_instance == last_action_instance]
        return temp_result


class Parameter:
    def __init__(self):
        self.d = {}
        self.d["rand_range"] = 0
        self.d["data_complexity"] = 0
        self.d["control_complexity"] = 0
        self.d["control_frequency"] = 0
        self.d["execution_id"] = 0
        self.d["state_size"] = 0
        self.d["data_state"] = 0
        self.d["control_state"] = 0

    def to_string(self):
        result = ""
        for key in self.d:
            result = result + str(self.d[key]) + " "

        return result[:-1]
