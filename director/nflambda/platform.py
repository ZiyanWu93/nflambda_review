import os
import socket
import time
from typing import List

from em import deployment_model
from em.nf_model import Parameter, Action_instance_execution, State_instance_execution
from . import get_microarchitecture_snapshot
from .command import *


class NFlambda:
    NFLAMBDA_IP = 'ip_address'
    NFLAMBDA_PORT = 8093  # Port to listen on (non-privileged ports are > 1023)
    counter = 0
    running_core = []
    data_cores = []
    data_queue_map = {}

    def __init__(self, start=True):
        # assert set(data_cores) <= set(running_core)
        if (start == True):
            os.system(
                '''ssh ip_address "nohup sudo python /home/anon/PycharmProjects/nsdi_tools/main.py"'''
            )
            time.sleep(0.2)
            os.system(
                '''ssh ip_address "nohup sudo /home/anon/CLionProjects/NF_Platform/build/nf_p > foo.out 2> foo.err < /dev/null &"'''
            )
            time.sleep(0.2)
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.ip = self.NFLAMBDA_IP
        self.port = self.NFLAMBDA_PORT
        # Connect the socket to the port where the server is listening
        server_address = (self.ip, self.port)
        count = 0
        result = None
        time.sleep(1)
        while result is None:
            try:
                # connect

                self.s.connect(
                    (self.ip,
                     self.port))  # always throws Con Refused when tryed
                data = self.s.recv(1024)
                self.counter = 0
                result = True
            except Exception as e:
                print(e)
                time.sleep(1)
                count = count + 1
                if (count == 5):
                    raise ValueError('dont know __init__')

        self.parameter = Parameter()

    def send_command(self, command):
        count = 0
        try:

            self.s.sendall(command.apply())
            data = self.s.recv(1024)
            while (data == None):
                data = self.s.recv(1024)
                count = count + 1
                time.sleep(1)
                if (count == 10):
                    raise ValueError('10 seconds with no response')

            return data.decode()

        except:
            result = None
            while result is None:
                try:
                    # connect
                    self.s.connect(
                        (self.ip,
                         self.port))  # always throws Con Refused when tryed

                    # print("Connected")
                    self.s.sendall(command.apply())
                    # self.s.settimeout(10)
                    data = self.s.recv(1024)
                    result = True
                    return data.decode()

                except:
                    time.sleep(1)
                    count = count + 1
                    if (count == 5):
                        raise ValueError('dont know send_command')

    def start_port(self):
        self.send_command(command_port.with_argument(["init"]))


    def start_pcm(self):
        os.system(
            '''ssh ip_address "cd /home/anon/project/pcm && nohup sudo /home/anon/project/pcm/pcm-raw.x 0.5 -el /home/anon/project/pcm/event_list/l_1.txt -csv=/tmp/log/output.csv > foo.out 2> foo.err < /dev/null &"'''
        )
        return

    def kill_pcm(self):
        os.system(
            '''ssh anon@ip_address "python /home/anon/PycharmProjects/nsdi_tools/kill_pcm.py"'''
        )
        return

    def run(self):
        #
        self.start_port()
        self.configure_rss()
        self.send_command(command_worker.with_argument(["run"]))

    # called after the state is created
    # register action in the action_table, associate it with its
    # configure worker's first action if it is the first action after packet_in
    # configure the paths
    def worker_first_action(self, worker_id, action_id, starting_bucket, num_bucket):
        self.send_command(command_worker.with_argument(
            ["first_action", str(worker_id), str(action_id), str(starting_bucket), str(num_bucket)]))

    def install_simple_path(self, state_id_1, action_id_2, local_path_id):
        self.send_command(command_state.with_argument(
            ["install_simple_path", str(state_id_1), str(action_id_2), str(local_path_id)]))

    def create_action(self, a_i_e: Action_instance_execution, action_index, state_index):
        # print(str(a_i_e.action_instance.action.registration_id), str(action_index),
        #      str(state_index), str(a_i_e.core_id))
        self.send_command(command_action.with_argument(
            ["create", str(a_i_e.action_instance.action.registration_id), str(action_index),
             str(state_index), str(a_i_e.core_id)]))

    def create_packet_out_action(self, action_index, core_id):
        self.send_command(command_action.with_argument(
            ["create_packet_out", str(action_index), str(core_id)]))

    def migrate_action(self, a_i_e, action_index):
        self.send_command(command_action.with_argument(["migrate", str(action_index), str(a_i_e.core_id)]))

    def create_state(self, s_i_e: State_instance_execution, index):
        self.send_command(
            command_state.with_argument(["create", str(s_i_e.state_instance.state.registration_id), str(index)]))

    def state_num_flows(self, state_id,num_flows):
        self.send_command(
            command_state.with_argument(["data_state", str(state_id), str(num_flows)]))

    def configure_rss_reta(self, starting_bucket, num_bucket, core_id, action_id):
        self.send_command(
            command_config.with_argument(
                ["rss_reta_config", str(starting_bucket), str(num_bucket), str(core_id), str(action_id)]))

    def configure_rss(self):
        self.send_command(command_config.with_argument(["rss"]))

    def dump_worker(self):
        self.send_command(command_dump.with_argument(["worker_stat"]))

    def configure_parameter(self, parameter):
        self.send_command(command_config.with_argument(["parameter", parameter.to_string()]))

    def configure_state(self, state_key, state_id, val):
        return self.send_command(command_state.with_argument([str(state_key), str(state_id), str(val)]))

    def dump_message_num(self, state_id):
        return self.send_command(command_state.with_argument(["message_num", str(state_id)]))

    def deployment(self, d_m: deployment_model):

        self.d_m = d_m
        n_packet_in = d_m.scaling_plan_list[0]  # number

        a_i_e_list = d_m.action_instance_execution_list
        s_i_e_list = d_m.state_instance_execution_list

        # install states
        for s_i_e in s_i_e_list:
            self.create_state(s_i_e, s_i_e_list.index(s_i_e))

        # install actions
        for a_i_e in a_i_e_list:
            if (a_i_e.action_instance.action.name not in ["packet_in", "packet_out", "drop"]):
                self.create_action(a_i_e, d_m.action_index(a_i_e), d_m.state_index(a_i_e.state_instance_execution))
                # print(a_i_e, d_m.action_index(a_i_e), d_m.state_index(a_i_e.state_instance_execution))
            if (a_i_e.action_instance.action.name in ["packet_out"]):
                self.create_packet_out_action(d_m.action_index(a_i_e), d_m.placement_function[a_i_e_list.index(a_i_e)])


        n_packet_in = d_m.scaling_plan_list[0]  # number
        num_buckets = 512 / n_packet_in
        starting_bucket = 0
        for i in range(n_packet_in):
            a_i_e = d_m.action_instance_execution_list[i]
            a_i_e.starting_bucket = starting_bucket
            a_i_e.num_buckets = (int)(num_buckets)
            starting_bucket = starting_bucket + num_buckets
            self.configure_rss_reta(a_i_e.starting_bucket, a_i_e.num_buckets, d_m.placement_function[i], i)

        # configure each worker's first action
        for i in range(n_packet_in):
            a_i_e = d_m.action_instance_execution_list[i]
            for temp_a_i_e in a_i_e_list:
                if a_i_e in temp_a_i_e.dependency_list:
                    action_id = a_i_e_list.index(temp_a_i_e)
                    self.worker_first_action(d_m.placement_function[i], action_id, a_i_e.starting_bucket,
                                                 a_i_e.num_buckets)
                    # print(d_m.placement_function[i], action_id, a_i_e.starting_bucket, a_i_e.num_buckets)

        # install paths among the functions
        for a_i_e in a_i_e_list:
            if (a_i_e.action_instance.action.is_packet_in()):
                continue
            for temp_a_i_e in a_i_e_list:
                if a_i_e in temp_a_i_e.dependency_list:
                    print(a_i_e_list.index(a_i_e),a_i_e_list.index(temp_a_i_e))
                    state_id_1 = s_i_e_list.index(a_i_e.state_instance_execution)
                    action_id_2 = a_i_e_list.index(temp_a_i_e)
                    # if in the same nf
                    if a_i_e.action_instance.nf_id == temp_a_i_e.action_instance.nf_id:
                        nf = a_i_e.action_instance.action.nf
                        local_path_id = nf.get_path_id(a_i_e.action_instance.action, temp_a_i_e.action_instance.action)
                        # print(local_path_id)
                        self.install_simple_path(state_id_1, action_id_2, local_path_id)
                        # print(a_i_e,temp_a_i_e)
                    if a_i_e.action_instance.nf_id == temp_a_i_e.action_instance.nf_id - 1:
                        nf = a_i_e.action_instance.action.nf
                        local_path_id = nf.get_path_id(a_i_e.action_instance.action, nf.action_list[-1])
                        self.install_simple_path(state_id_1, action_id_2, local_path_id)
                        # print(a_i_e.state_instance_execution)

    def exp_generic_decomposed(self):
        pass

    def get_snapshot(self, sampling_time):
        time.sleep(sampling_time)
        v1 = (int)(self.send_command(command_dump.with_argument(["stat"])))
        time.sleep(sampling_time)
        v2 = (int)(self.send_command(command_dump.with_argument(["stat"])))
        snapshot = get_microarchitecture_snapshot()
        result = {}
        result["throughput"] = (v2 - v1) / 1000000000 * 84 / 64 / sampling_time
        return result | snapshot

    def kill_platform(self):
        os.system('''ssh aum "python /home/anon/PycharmProjects/nsdi_tools/main.py"''')
        return

    def __del__(self):
        #         os.system('''ssh aum "python /home/anon/PycharmProjects/nsdi_tools/main.py"''')
        self.s.shutdown(socket.SHUT_RDWR)
        # print("connection closed for {}".format(self.__class__))
