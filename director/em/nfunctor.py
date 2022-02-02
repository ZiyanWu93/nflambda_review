from igraph import *


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


class Obj:
    def __init__(self, **argv):
        self.name = argv["name"]

    def __str__(self):
        #     attrs = vars(self)
        #     return ''.join("%s: %s\n" % item for item in attrs.items())
        return self.name

    def __repr__(self):
        return self.__str__()


class Table:
    def __init__(self):
        self.table = {}
        self.init_function = Obj

    def add(self, **argv):
        # print(argv)
        self._add(self.init_function(**argv))

    def _add(self, obj):
        self.table[obj.name] = obj

    def __getitem__(self, name):
        return self.table[name]

    def __setitem__(self, key, newvalue):
        self.table[key] = newvalue

    def __str__(self):
        return self.table.__str__()

    def __repr__(self):
        return self.__str__()

class Property(Obj):
    def __init__(self, **argv):
        super().__init__(**argv)

class Property_table(Table):
    def __init__(self):
        super().__init__()
        self.init_function = Property


class State(Obj):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.properties = kwargs["properties"]

    def __repr__(self):
        return "(State) Name: " + self.name + " | Properties: " + self.properties.__str__() + "\n"


class Action(Obj):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.properties = kwargs["properties"]
        self.registration_id = kwargs["registration_id"]

    def __repr__(self):
        return "(Action) Name: " + self.name + " | Properties: " + self.properties.__str__() + "\n"


class State_table(Table):
    def __init__(self):
        super().__init__()
        self.init_function = State


class Action_table(Table):
    def __init__(self):
        super().__init__()
        self.init_function = Action


class Nfunctor(Obj):
    def __init__(self, **kwargs):
        self.state = kwargs["state"]
        self.action = kwargs["action"]
        self.name = "("+str(self.action) + ", " + str(self.state)+")"


class Nfunctor_relationship(Obj):
    def __init__(self, **kwargs):
        self.predecessor = kwargs["predecessor"]
        self.successor = kwargs["successor"]
        self.path_id = kwargs["path_id"]

    def __eq__(self, other):
        return self.predecessor == other.predecessor and self.successor == other.successor

    def __str__(self):
        return "(" + str(self.predecessor.action) + ", " + str(self.successor.action) + ")"

    def __contains__(self, key):
        return key in self.predecessor.name or key in self.successor.name


class Nf(Obj):
    def __init__(self, **kwargs):
        self.name = kwargs["name"]
        self.nfunctor_list = kwargs["nfunctor_list"]
        self.nfunctor_relationship_list = kwargs["nfunctor_relationship_list"]

    def packet_in(self):
        for i in self.nfunctor_list:
            if i.action.name == "packet_in":
                packet_in = i
        return packet_in

    def packet_out(self):
        for i in self.nfunctor_list:
            if i.action.name == "packet_out":
                packet_out = i
        return packet_out

    def first_action(self):
        result = []
        for i in self.nfunctor_list:
            if i.action.name == "packet_in":
                packet_in = i
        for j in self.nfunctor_relationship_list:
            if packet_in == j.predecessor:
                result.append(j.successor)
        return result

    def last_action(self):
        result = []
        for i in self.nfunctor_list:
            if i.action.name == "packet_out":
                packet_out = i
        for j in self.nfunctor_relationship_list:
            if packet_out == j.successor:
                result.append(j.predecessor)
        return result

class Nfunctor_instance(Obj):
    def __init__(self, nfunctor, **kwargs):
        self.nfunctor = nfunctor
        self.name = self.name
        self.nf_id = kwargs["nf_id"]

    def __getattr__(self, name):
        try:
            return getattr(self.nfunctor, name)
        except AttributeError as e:
            raise AttributeError("Child' object has no attribute '%s'" % name)


class Nfunctor_relationship_instance(Obj):
    def __init__(self, **kwargs):
        self.predecessor = kwargs["predecessor"]
        self.successor = kwargs["successor"]
        self.path_id = kwargs["path_id"]
        self.tuple = (self.predecessor, self.successor)

    def reduce(self, nri):
        # self.nfunctor_relationship.successor =
        pass

    def is_in(self, action):
        if action in self.tuple:
            return True
        else:
            return False

    def __str__(self):
        temp = (self.predecessor, self.successor)
        return str(temp)


class NF_Instance(Obj):
    def __init__(self, nf, **kwargs):
        self.nf = nf
        # self.nf_id = kwargs["nf_id"] # to denote the sequence number in the service function chain
        self.nfunctor_instance_list = []
        self.nfunctor_relationship_instance_list = []
        for nfunctor in self.nfunctor_list:
            new_nfunctor_instance = Nfunctor_instance(nfunctor,nf_id = 0)
            self.nfunctor_instance_list.append(new_nfunctor_instance)

        for i in self.nfunctor_instance_list:
            for j in self.nfunctor_instance_list:
                nfunctor_1 = i.nfunctor
                nfunctor_2 = j.nfunctor
                for k in self.nfunctor_relationship_list:
                    if k.predecessor == nfunctor_1 and k.successor == nfunctor_2:
                        new_nfunctor_relationship_instance = Nfunctor_relationship_instance(predecessor=i, successor=j, path_id = k.path_id)
                        self.nfunctor_relationship_instance_list.append(new_nfunctor_relationship_instance)

    def __getattr__(self, name):
        try:
            return getattr(self.nf, name)
        except AttributeError as e:
            raise AttributeError("Child' object has no attribute '%s'" % name)

    def packet_in(self):
        for i in self.nfunctor_instance_list:
            if i.action.name == "packet_in":
                packet_in = i
        return packet_in

    def packet_out(self):
        for i in self.nfunctor_instance_list:
            if i.action.name == "packet_out":
                packet_out = i
        return packet_out

    def first_action_instances(self):
        result = []
        for i in self.nfunctor_instance_list:
            if i.action.name == "packet_in":
                packet_in_instance = i
        for j in self.nfunctor_relationship_instance_list:
            if packet_in_instance == j.predecessor:
                result.append(j.successor)
        return result

    def last_action_instances(self):
        result = []
        for i in self.nfunctor_instance_list:
            if i.action.name == "packet_out":
                packet_in_instance = i
        for j in self.nfunctor_relationship_instance_list:
            if packet_in_instance == j.successor:
                result.append(j.predecessor)
        return result

    def merge(self, nf_instance):
        nfi_1 = self
        nfi_2 = nf_instance
        packet_out = nfi_1.packet_out()
        packet_in = nfi_2.packet_in()
        actions_1 = nfi_1.last_action_instances()
        actions_2 = nfi_2.first_action_instances()
        # get all the related relation of action_1
        # get all the related relation of action_2
        # remove all the relations
        # join the new relation
        merged_nfunctor_instance_set = []
        merged_relation_set = []
        relation_set_previous = []
        for i in nfi_1.nfunctor_relationship_instance_list:
            merged_relation_set.append(i)
            if actions_1[0] == i.predecessor:
                relation_set_previous.append(i)
                merged_relation_set.remove(i)
            # if (packet_out == i.successor):
            #     merged_relation_set.remove(i)

        relation_set_next = []
        for i in nfi_2.nfunctor_relationship_instance_list:
            merged_relation_set.append(i)
            if actions_2[0] == i.successor:
                relation_set_next.append(i)
                merged_relation_set.remove(i)

        for i in nfi_1.nfunctor_instance_list:
            merged_nfunctor_instance_set.append(i)

        for i in nfi_2.nfunctor_instance_list:
            merged_nfunctor_instance_set.append(i)

        merged_nfunctor_instance_set.remove(packet_out)
        merged_nfunctor_instance_set.remove(packet_in)

        joined_relation = []
        # print(relation_set_previous)
        # print(relation_set_next)
        for i in relation_set_previous:
            for j in relation_set_next:
                previous = i.predecessor
                successor = j.successor
                new_relation_instance = Nfunctor_relationship_instance(predecessor=previous, successor=successor)
                joined_relation.append(new_relation_instance)
        merged_relation_set = merged_relation_set + joined_relation

        self.nfunctor_instance_list = merged_nfunctor_instance_set
        self.nfunctor_relationship_instance_list = merged_relation_set

    def plot(self):
        action_n = len(self.nfunctor_instance_list)
        dependency_graph = Graph(directed=True)
        visual_style = {}
        visual_style["vertex_label"] = []
        dependency_graph.add_vertices(action_n)
        for a_i_e in self.nfunctor_instance_list:
            visual_style["vertex_label"].append(a_i_e.name)

        for i in self.nfunctor_relationship_instance_list:
            index1 = self.nfunctor_instance_list.index(i.predecessor)
            index2 = self.nfunctor_instance_list.index(i.successor)
            dependency_graph.add_edges([(index1, index2)])

        bbox = (500, 500)
        l = dependency_graph.layout("auto")
        return plot(dependency_graph, layout=l, vertex_size=20, bbox=bbox, **visual_style,
                    margin=100)

