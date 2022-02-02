from typing import List

import igraph as ig
from igraph import *
from copy import deepcopy, copy

from .nf_model import Nf_instance_execution, Sfc, Action_instance_execution, State_instance_execution


class execution_model:

    # name that is nicer to read
    def __init__(self, sfc: Sfc):
        self.state_instance_list = []
        self.action_instance_list = []
        self.nf_instance_list = []
        self.deployed_result = {}
        self.sfc = sfc
        self.nf_instance_execution_list = []
        self.nf_instance_execution_list = []
        self.action_instance_execution_list = []
        self.state_instance_execution_list = []
        self.visual_style = {}
        self.visual_style["vertex_label"] = []
        self.visual_style["vertex_color"] = []
        self.visual_style["edge_color"] = []
        self.visual_style["vertex_shape"] = []
        self.shard_num = 0
        # print("number of action instance:", len(self.sfc.action_instance_list))
        # self.partition = [item for sublist in self.partition for item in sublist]

    def scaling(self, scaling_plan_list):
        self.scaling_plan_list = scaling_plan_list

        for i, n in enumerate(scaling_plan_list):
            for j in range(n):
                a_i_e = Action_instance_execution(self.sfc.action_instance_list[i], j)
                self.action_instance_execution_list.append(a_i_e)

        for s_i in self.sfc.state_instance_list:
            a_i = s_i.action_instance_list[0]
            n = scaling_plan_list[self.sfc.action_instance_list.index(a_i)]

            for i in range(n):
                s_i_e = State_instance_execution(s_i, i)
                a_i_e_list = self.get_a_i_e_list(s_i.action_instance_list, s_i.nf_id, i)
                for a_i_e in a_i_e_list:
                    a_i_e.state_instance_execution = s_i_e
                    s_i_e.a_i_e_list.append(a_i_e)
                self.state_instance_execution_list.append(s_i_e)

        for i1, a_i_e_1 in enumerate(self.action_instance_execution_list):
            for i2, a_i_e_2 in enumerate(self.action_instance_execution_list):
                index1 = self.sfc.action_instance_list.index(a_i_e_1.action_instance)
                index2 = self.sfc.action_instance_list.index(a_i_e_2.action_instance)
                # print(index1,index2)
                # print(scaling_plan_list[index1],scaling_plan_list[index2])
                if index1 < index2 and a_i_e_1.action_instance in a_i_e_2.action_instance.dependency_list:
                    if scaling_plan_list[index1] < scaling_plan_list[index2]:
                        e_id_1 = a_i_e_1.execution_id
                        if e_id_1 % scaling_plan_list[index2] == 0:
                            a_i_e_2.dependency_list.append(a_i_e_1)
                    elif scaling_plan_list[index1] > scaling_plan_list[index2]:
                        e_id_2 = a_i_e_2.execution_id
                        if e_id_2 % scaling_plan_list[index1] == 0:
                            a_i_e_2.dependency_list.append(a_i_e_1)
                    else:
                        if a_i_e_1.execution_id == a_i_e_2.execution_id:
                            a_i_e_2.dependency_list.append(a_i_e_1)

        self.action_n = len(self.action_instance_execution_list)
        self.state_n = len(self.state_instance_execution_list)
        self.dependency_graph = Graph(directed=True)
        self.dependency_graph.add_vertices(self.action_n + self.state_n)
        for a_i_e in self.action_instance_execution_list:
            self.visual_style["vertex_label"].append(a_i_e.name)
            self.visual_style["vertex_color"].append("black")
            self.visual_style["vertex_shape"].append("circle")

        for s_i_e in self.state_instance_execution_list:
            self.visual_style["vertex_label"].append(s_i_e.name)
            self.visual_style["vertex_color"].append("red")
            self.visual_style["vertex_shape"].append("triangle")
            for a_i_e in self.action_instance_execution_list:
                if a_i_e.state_instance_execution == s_i_e:
                    start = self.action_instance_execution_list.index(a_i_e)
                    end = self.state_instance_execution_list.index(s_i_e) + self.action_n
                    self.dependency_graph.add_edges([(start, end)])
                    self.visual_style["edge_color"].append("grey")

        for index1, a_i_e_1 in enumerate(self.action_instance_execution_list):
            for index2, a_i_e_2 in enumerate(self.action_instance_execution_list):
                if a_i_e_1 in a_i_e_2.dependency_list:
                    start = self.action_instance_execution_list.index(a_i_e_1)
                    end = self.action_instance_execution_list.index(a_i_e_2)
                    self.dependency_graph.add_edges([(start, end)])
                    self.visual_style["edge_color"].append("black")


        return

    def plot(self, layout="auto", vertex_size=20, size=400, margin=50):
        bbox = (size, size)
        l = self.dependency_graph.layout(layout)
        return plot(self.dependency_graph, layout=l, vertex_size=vertex_size, bbox=bbox, **self.visual_style,
                    margin=margin)

    def get_a_i_e_list(self, action_instance_list, nf_id, execution_id):
        a_i_e_list = []
        for a_i_e in self.action_instance_execution_list:
            if a_i_e.action_instance in action_instance_list:
                if a_i_e.nf_id == nf_id and a_i_e.execution_id == execution_id:
                    a_i_e_list.append(a_i_e)
        return a_i_e_list

    def add_nf(self, nf):
        pass

    # def add_action(self, action_name, n_instance):
    #     self.action_list.append(action)
    #     pass

    def deep_index(self, lst, w):
        return [(i, sub.index(w)) for (i, sub) in enumerate(lst) if w in sub]

    def deep_in(self, lst, w):
        for i in lst:
            if w in i:
                return True
        return False

    def deep_connected(self, lst, w):
        pass
        # for i in lst:
        #     if self.execution_graph.edge_connectivity(i, w) != 0:
        #         return True
        # return False

    def which_partition(self, partition, element):
        for i in partition:
            if element in i:
                return partition.index(i)

    def deployment(self, decouple=False, pipeline=False, inter_pipeline=True, available_cores=None):
        # deployment huristic
        # access the same state should reside on the same core, except that the state is sharable
        # if each the scaiing of the data components of each nf_instance
        if available_cores is None:
            available_cores = []

        self.partition = []
        if not pipeline:
            if not decouple:
                n = self.scaling_plan_list[0][0]
                for i in range(n):
                    self.partition.append([])
                for a_i_e in self.action_instance_execution_list:
                    self.partition[a_i_e.execution_id].append(a_i_e)
            else:
                pass

        self.partition_id_to_core_id = {}
        core_id_index = 0
        for i in self.partition:
            partition_id = self.partition.index(i)
            self.partition_id_to_core_id[partition_id] = available_cores[core_id_index]
            core_id_index = (core_id_index + 1) % len(available_cores)

        for a_i_e in self.action_instance_execution_list:
            partition_id = self.which_partition(self.partition, a_i_e)
            core_id = self.partition_id_to_core_id[partition_id]
            a_i_e.assign_core(core_id)

    def node_to_action_name(self, node_id):
        return self.vertex_name[node_id]


class deployment_model(execution_model):

    # name that is nicer to read
    def __init__(self, em: execution_model):
        vars(self).update(vars(em))
        self.placement_function = {}
        self.n_a_i_e = len(self.action_instance_execution_list)
        # print("how many actions needs to be deployed:", self.n_a_i_e)

    def state_index(self, s_i_e):
        return self.state_instance_execution_list.index(s_i_e)

    def action_index(self, a_i_e):
        return self.action_instance_execution_list.index(a_i_e)

    def assign(self, a_i_e_index, core_id):
        self.placement_function[a_i_e_index] = core_id
        self.action_instance_execution_list[a_i_e_index].core_id = core_id


