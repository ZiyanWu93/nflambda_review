from .em import *
from .nfunctor import *
from igraph import *
from igraph import plot


p_t = Table()
p_t.add(name = "system")
p_t.add(name = "per_packet_operation")

p_per_packet_operation = p_t["per_packet_operation"]

s_t = State_table()
s_t.add(name = "gd_data_state",properties = [])
s_t.add(name = "gd_control_state", properties = [])

a_t = Action_table()
a_t.add(name= "packet_in",properties = [p_t["system"], p_t["per_packet_operation"]], registration_id = 0)
a_t.add(name= "packet_out",properties = [p_t["system"], p_t["per_packet_operation"]], registration_id = 1)
a_t.add(name = "gd_classifier",properties = [p_t["per_packet_operation"]],registration_id = 3)
a_t.add(name = "gd_data",properties = [p_t["per_packet_operation"]],registration_id = 4)
a_t.add(name = "gd_control",properties = [None],registration_id = 5)


nfunctor1 = Nfunctor(action = a_t["packet_in"], state = None)
nfunctor2 = Nfunctor(action = a_t["gd_classifier"], state = s_t["gd_data_state"])
nfunctor3 = Nfunctor(action = a_t["gd_data"], state = s_t["gd_data_state"])
nfunctor4 = Nfunctor(action = a_t["gd_control"], state = s_t["gd_control_state"])
nfunctor5 = Nfunctor(action = a_t["packet_out"], state = None)
nfunctor_list = [nfunctor1,nfunctor2,nfunctor3,nfunctor4,nfunctor5]

Nfunctor_relationship_1 = Nfunctor_relationship(predecessor = nfunctor1, successor = nfunctor2, path_id = 0)
Nfunctor_relationship_2 = Nfunctor_relationship(predecessor = nfunctor2, successor = nfunctor3, path_id = 0)
Nfunctor_relationship_3 = Nfunctor_relationship(predecessor = nfunctor2, successor = nfunctor4, path_id = 1)
Nfunctor_relationship_4 = Nfunctor_relationship(predecessor = nfunctor4, successor = nfunctor3, path_id = 0)
Nfunctor_relationship_5 = Nfunctor_relationship(predecessor = nfunctor3, successor = nfunctor5, path_id = 2)
nfunctor_relationship_list = [Nfunctor_relationship_1,Nfunctor_relationship_2,Nfunctor_relationship_3,Nfunctor_relationship_4,Nfunctor_relationship_5]

def in_relationship_list(nfunctor_relationship_list, action_1, action_2):
    for i in nfunctor_relationship_list:
        if (action_1 == i.predecessor.action and action_2 == i.successor.action):
            return i.path_id
    return -1

#in_relationship_list(nfunctor_relationship_list, nfunctor2.action, nfunctor4.action)

gd = Nf(name = "gd", nfunctor_list = nfunctor_list,nfunctor_relationship_list=nfunctor_relationship_list)

nfi_1 = NF_Instance(gd)
nfi_2 = NF_Instance(gd)


# execution_id is the id of the instance
class Nfunctor_runtime(Obj):
    def __init__(self, nfunctor_instance, **kwargs):
        self.nfunctor_instance = nfunctor_instance
        self.execution_id = kwargs["execution_id"]
        self.core_id = 1
        self.name = self.nfunctor_instance.name + "_" + str(self.execution_id) + "_" + str(self.core_id)

        # only valid for first action for RSS
        self.starting_bucket = 0
        self.ending_bucket = 0

        self.path = []
        for i in range(10):
            self.path.append([-1 for i in range(20)])

    def is_per_packet(self):
        return p_per_packet_operation in self.nfunctor_instance.action.properties

        # self.nf_instance.nfunctor_relationship_instance_list

    def assign(self, core_id):
        self.core_id = core_id
        self.name = self.nfunctor_instance.name + "_" + str(self.execution_id) + "_" + str(self.core_id)


class Nfunctor_relationship_runtime(Obj):
    def __init__(self, nfunctor_runtime_1, nfunctor_runtime_2, path_id, **kwargs):
        self.predecessor = nfunctor_runtime_1
        self.successor = nfunctor_runtime_2
        self.path_id = path_id

    def __str__(self):
        temp = (self.predecessor, self.successor, self.path_id)
        return str(temp)


# execution_id is the id of the instance
class Nfunctor_runtime(Obj):
    def __init__(self, nfunctor_instance, **kwargs):
        self.nfunctor_instance = nfunctor_instance
        self.execution_id = kwargs["execution_id"]
        self.core_id = 1
        self.name = self.nfunctor_instance.name + "_" + str(self.execution_id) + "_" + str(self.core_id)

        # only valid for first action for RSS
        self.starting_bucket = 0
        self.ending_bucket = 0

        self.path = []
        for i in range(10):
            self.path.append([-1 for i in range(20)])

    def is_per_packet(self):
        return p_per_packet_operation in self.nfunctor_instance.action.properties

        # self.nf_instance.nfunctor_relationship_instance_list

    def assign(self, core_id):
        self.core_id = core_id
        self.name = self.nfunctor_instance.name + "_" + str(self.execution_id) + "_" + str(self.core_id)


class Nfunctor_relationship_runtime(Obj):
    def __init__(self, nfunctor_runtime_1, nfunctor_runtime_2, path_id, **kwargs):
        self.predecessor = nfunctor_runtime_1
        self.successor = nfunctor_runtime_2
        self.path_id = path_id

    def __str__(self):
        temp = (self.predecessor, self.successor, self.path_id)
        return str(temp)


class state_runtime():
    def __init__(self, state, nf_id, execution_id):
        self.state = state
        self.nf_id = nf_id
        self.execution_id = execution_id
        self.path = [-1 for i in range(10)]

    def __repr__(self):
        return self.state.name + " nf_id: " + str(self.nf_id) + " execution_id: " + str(
            self.execution_id) + " path" + str(self.path)

    def __str__(self):
        return self.state.name + " nf_id: " + str(self.nf_id) + " execution_id: " + str(
            self.execution_id) + " path" + str(self.path)


class state_runtime_table():
    def __init__(self):
        self.data = []

    def search(self, state, nf_id, execution_id):
        for i in self.data:
            if i.state == state and i.nf_id == nf_id and execution_id == i.execution_id:
                return self.data.index(i)

        return None

    def insert(self, state, nf_id, execution_id):
        search_result = self.search(state, nf_id, execution_id)
        if search_result == None:
            # print(state.name, nf_id,execution_id)
            self.data.append(state_runtime(state, nf_id, execution_id))
            return self.search(state, nf_id, execution_id)
        else:
            return search_result


class action_runtime():
    def __init__(self, action, nf_id, execution_id, state_id):
        self.action = action
        self.nf_id = nf_id
        self.execution_id = execution_id
        self.state_id = state_id
        self.core_id = 1
        self.starting_bucket = 0
        self.num_bucket = 0
        self.next_action = -1  # reserver for packet_in

    def is_data(self):
        return p_per_packet_operation in self.action.action.properties

    def __repr__(self):
        return self.action.name + " nf_id: " + str(self.nf_id) + " execution_id: " + str(
            self.execution_id) + " state_id: " + str(self.state_id)

    def __str__(self):
        return self.action.name + " nf_id: " + str(self.nf_id) + " execution_id: " + str(
            self.execution_id) + " state_id: " + str(self.state_id)


class action_runtime_table():
    def __init__(self):
        self.data = []

    def search(self, action, nf_id, execution_id, state_id):
        for i in self.data:
            if i.action == action and i.nf_id == nf_id and execution_id == i.execution_id:
                return self.data.index(i)

        return None

    def insert(self, action, nf_id, execution_id, state_id):
        search_result = self.search(action, nf_id, execution_id, state_id)
        if search_result == None:
            # print(action, nf_id, execution_id, state_id)
            self.data.append(action_runtime(action, nf_id, execution_id, state_id))
            # print(self.search(action, nf_id, execution_id, state_id))
            return self.search(action, nf_id, execution_id, state_id)
        else:
            return search_result


class Nfunctor_graph(Obj):
    def __init__(self, nf_instance, **kwargs):
        self.nf_instance = nf_instance
        self.nfunctor_runtime_list = []
        self.nfunctor_relationship_runtime = []
        self.state_runtime_table = state_runtime_table()
        self.action_runtime_table = action_runtime_table()

        self.scaling_n = kwargs["scaling_n"]

        self.num_buckets = (int)(512 / self.scaling_n)
        self.starting_bucket = 0

        for i in range(self.scaling_n):
            temp = []
            for nfunctor_instance in self.nf_instance.nfunctor_instance_list:
                state_id = -1
                state = nfunctor_instance.nfunctor.state
                nf_id = nfunctor_instance.nf_id
                execution_id = i
                if (state != None):
                    state_id = self.state_runtime_table.insert(state, nf_id, execution_id)
                # new_nfunctor_runtime = Nfunctor_runtime(nfunctor_instance, execution_id = i)
                # temp.append(new_nfunctor_runtime)
                action = nfunctor_instance.nfunctor
                # if action.action.name =="packet_in":
                self.action_runtime_table.insert(action, nf_id, execution_id, state_id)
                if action.action.name == "packet_in":
                    self.action_runtime_table.data[-1].starting_bucket = self.starting_bucket
                    self.action_runtime_table.data[-1].num_bucket = self.num_buckets
                    self.starting_bucket = self.starting_bucket + self.num_buckets
                    # what is the next hop

        for i in self.action_runtime_table.data:
            for j in self.action_runtime_table.data:
                path_id = in_relationship_list(nfunctor_relationship_list, i.action.action, j.action.action)
                if in_relationship_list(nfunctor_relationship_list, i.action.action,
                                        j.action.action) != -1 and i.execution_id == j.execution_id:
                    if (i.action.action.name == "packet_in"):
                        i.next_action = self.action_runtime_table.data.index(j)

                    state_id = i.state_id
                    self.state_runtime_table.data[state_id].path[path_id] = self.action_runtime_table.data.index(j)

    def deploy(self, data_core_list, control_core_list):
        # distribution
        # even distribution
        # manual distribution
        if control_core_list == []:
            num_partition = len(data_core_list)
#
# state_fwd = State(
#     state_name="fwd_state", shared=False)
#
# action_fwd = Action(
#     action_name="fwd",
#     per_packet_operation=True,
#     depenencies_action_list=[],
#     state=state_fwd,
# )
#
# nf_fwd = Nf(
#     "fwd",
#     action_list=[action_fwd])
#
# state_generic_decomposed_data = State(
#     state_name="gdd_state", shared=False)
#
# state_generic_decomposed_control = State(
#     state_name="gdc_state", shared=False)
#
# action_generic_decomposed_classifier = Action(
#     action_name="gd_classifier",
#     per_packet_operation=True,
#     depenencies_action_list=[],
#     state=state_generic_decomposed_data)
#
# action_generic_decomposed_control = Action(
#     action_name="gd_control",
#     per_packet_operation=False,
#     depenencies_action_list=[action_generic_decomposed_classifier],
#     state=state_generic_decomposed_control)
#
# action_generic_decomposed_data = Action(
#     action_name="gd_data",
#     per_packet_operation=True,
#     depenencies_action_list=[action_generic_decomposed_control, action_generic_decomposed_classifier],
#     state=state_generic_decomposed_data)
#
# nf_generic_decomposed = Nf("gd",
#                            action_list=[action_generic_decomposed_classifier, action_generic_decomposed_control,
#                                         action_generic_decomposed_data]
#                            )
#
# state_generic_decomposed_data_shared = State(
#     state_name="gdd_state_shared", shared=False)
#
# state_generic_decomposed_control_shared = State(
#     state_name="gdc_state_shared", shared=True)
#
# action_generic_decomposed_classifier_shared = Action(
#     action_name="gd_classifier_shared",
#     per_packet_operation=True,
#     depenencies_action_list=[],
#     state=state_generic_decomposed_data_shared)
#
# action_generic_decomposed_control_shared = Action(
#     action_name="gd_control_shared",
#     per_packet_operation=False,
#     depenencies_action_list=[action_generic_decomposed_classifier_shared],
#     state=state_generic_decomposed_control_shared)
#
# action_generic_decomposed_data_shared = Action(
#     action_name="gd_data_shared",
#     per_packet_operation=True,
#     depenencies_action_list=[action_generic_decomposed_control_shared, action_generic_decomposed_classifier_shared],
#     state=state_generic_decomposed_data_shared)
#
# nf_generic_decomposed_shared_state = Nf("gd_shared",
#                                         action_list=[action_generic_decomposed_classifier_shared,
#                                                      action_generic_decomposed_control_shared,
#                                                      action_generic_decomposed_data_shared])
#
# state_lb_flow_mapper = State(
#     state_name="flow mapper state", shared=False)
#
# state_lb_server_selector = State(
#     state_name="server selector state", shared=False)
#
# action_lb_classifier = Action(
#     action_name="lb classifier",
#     per_packet_operation=True,
#     depenencies_action_list=[],
#     state=state_lb_flow_mapper)
#
# action_lb_server_selector = Action(
#     action_name="service selector action",
#     per_packet_operation=False,
#     depenencies_action_list=[action_lb_classifier],
#     state=state_lb_server_selector)
#
# action_lb_flow_mapper = Action(
#     action_name="flow mapper action",
#     per_packet_operation=True,
#     depenencies_action_list=[action_lb_server_selector, action_lb_classifier],
#     state=state_lb_flow_mapper)
#
# nf_lb = Nf("lb",
#            action_list=[action_lb_classifier, action_lb_server_selector,
#                         action_lb_flow_mapper]
#            )
#
# state_ids_data = State(
#     state_name="ids data state", shared=False)
#
# state_ids_control = State(
#     state_name="ids control state", shared=False)
#
# action_ids_classifier = Action(
#     action_name="ids classifier",
#     per_packet_operation=True,
#     depenencies_action_list=[],
#     state=state_ids_data)
#
# action_ids_dpi = Action(
#     action_name="ids dpi",
#     per_packet_operation=False,
#     depenencies_action_list=[action_ids_classifier],
#     state=state_ids_control)
#
# action_ids_fwd = Action(
#     action_name="ids fwder",
#     per_packet_operation=True,
#     depenencies_action_list=[action_ids_dpi, action_ids_classifier],
#     state=state_ids_data)
#
# nf_ids = Nf("ids",
#            action_list=[action_ids_classifier, action_ids_dpi,
#                         action_ids_fwd]
#            )

# ######################################################################
# state_generic_decomposed_parallel_data = State(
#     state_name="gdp_d_state", shared=False)
#
# state_generic_decomposed_parallel_control = State(
#     state_name="gdp_c_state", shared=False)
#
# action_generic_decomposed_parallel_classifier = Action(
#     action_name="gdp_classifer",
#     per_packet_operation=True,
#     depenencies_action_list=[],
#     state=state_generic_decomposed_parallel_data)
#
# action_generic_decomposed_parallel_control = Action(
#     action_name="gdp_control",
#     per_packet_operation=False,
#     depenencies_action_list=[action_generic_decomposed_parallel_classifier],
#     state=state_generic_decomposed_parallel_control)
#
# action_generic_decomposed_parallel_data = Action(
#     action_name="gdp_data",
#     per_packet_operation=False,
#     depenencies_action_list=[action_generic_decomposed_parallel_classifier],
#     state=state_generic_decomposed_parallel_data)
#
# nf_generic_decomposed_parallel = Nf(
#     "generic_decomposed_parallel",
#     action_list=[
#         action_generic_decomposed_parallel_classifier,
#         action_generic_decomposed_parallel_control,
#         action_generic_decomposed_parallel_data
#     ])

##################################################
