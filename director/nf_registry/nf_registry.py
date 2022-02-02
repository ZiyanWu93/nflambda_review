from em import State, Action, Nf
import yaml


class NF_registry:
    def __init__(self):
        self.action_table = {}
        self.state_table = {}
        self.nf_table = {}
        self.register(Action("packet_in", ["system"], [], None, registration_id=0))
        self.register(Action("packet_out", ["system"], [], None, registration_id=1))
        self.register(Action("drop", ["system"], [], None, registration_id=2))

    def read_file(self, filename):

        with open(filename, "r") as stream:
            try:
                # print(yaml.safe_load(stream))
                temp = yaml.safe_load(stream)
            except yaml.YAMLError as exc:
                print(exc)
        nf_name = temp["name"]

        for i in temp["state"]:
            state_name = i["name"]
            registration_id = i["registration_id"]
            self.register(State(state_name, i["properties"], registration_id))

        action_list = []
        for j in temp["action"]:
            action_name = j["name"]
            if action_name == "packet_in":
                action_list.append(self.action_table[action_name])
                continue

            if action_name not in ["packet_out", "drop"]:
                registration_id = int(j["registration_id"])
                state_name = j["state"]
                state = self.state_table[state_name]
                properties = j["properties"]
                if properties is None:
                    properties = ""
                dependency_list = []
                dependencies = []
                if action_name in temp["edges"]:
                    dependencies = temp["edges"][action_name]

                for temp_action_name in dependencies:
                    dependency_list.append(self.action_table[temp_action_name])
                new_action = Action(action_name, properties, dependency_list, state, registration_id=registration_id)
                self.register(new_action)
                action_list.append(new_action)
            else:
                action = self.action_table[action_name]
                dependency_list = []
                dependencies = []
                if action_name in temp["edges"]:
                    dependencies = temp["edges"][action_name]

                for temp_action_name in dependencies:
                    dependency_list.append(self.action_table[temp_action_name])
                action.dependencies_action_list = action.dependencies_action_list + dependency_list
                action_list.append(self.action_table[action_name])

        paths = []
        for des in temp["edges"]:
            for source in temp["edges"][des]:
              paths.append((source,des))

        new_nf = Nf(nf_name, action_list=action_list, paths = paths)
        self.register(new_nf)

    def register(self, obj):
        if type(obj) is State:
            self.state_table[obj.name] = obj

        if type(obj) is Action:
            self.action_table[obj.name] = obj

        if type(obj) is Nf:
            self.nf_table[obj.name] = obj
