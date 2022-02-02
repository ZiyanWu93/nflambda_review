import socket
import time

from command import Command
import errno

class Agent:
    def __init__(self, ip, port):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.ip = ip
        self.port = port
        # Connect the socket to the port where the server is listening
        server_address = (self.ip, self.port)
        print('connecting to %s port %s' % server_address)
        self.s.connect(server_address)

    def send_command_wait_respond(self, command: Command):
        # try:
        #     # Send data
        # print('sending {}{}'.format(command.command, command.argument_list))
        try:
            self.s.sendall(command.apply())
            if (command.command == "exit"):
                self.s.close()
                exit()

            if command.command == "stat":
                result = self.s.recv(1024)
                return eval(result.decode())

            if command.command == "stat_data":
                result = self.s.recv(1024)
                return eval(result.decode())

            data = self.s.recv(1024)
            if data:
                pass
                # print('received "%s"' % data.decode().replace("\n", ""))
            else:
                self.s.close()
                self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                server_address = (self.ip, self.port)
                print('connecting to %s port %s' % server_address)
                self.s.connect(server_address)

        except socket.error as e:
            if e.errno != errno.EPIPE:
                # Not a broken pipe
                raise
            self.s.close()
            self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_address = (self.ip, self.port)
            print('connecting to %s port %s' % server_address)
            self.s.connect(server_address)
            self.send_command_wait_respond(command) # try again

    def wait(self, n):
        # print("wait {} seconds".format(n))
        time.sleep(n)

    def __del__(self):
        self.s.close()
        print("connection closed for {}".format(self.__class__))


class TrafficGen(Agent):
    # command for traffic generator
    TrafficGen_IP = "127.0.0.1"
    TrafficGen_PORT = 10000
    command_play = Command("play")
    command_stop_pktgen_custom = Command("stop_pktgen_custom")
    command_stop_pktgen = Command("stop_pktgen")
    command_prepare = Command("prepare")
    command_pktgen = Command("pktgen")
    command_pktgen_speed = Command("pktgen_speed")

    def __init__(self, port=2000):
        Agent.__init__(self, self.TrafficGen_IP, port)

    def prepare(self, filename):
        self.send_command_wait_respond(
            self.command_prepare.with_argument([filename]))

    def play(self):
        self.send_command_wait_respond(self.command_play)

    def pktgen(self, filename):
        self.send_command_wait_respond(
            self.command_pktgen.with_argument([filename]))

    def stop_pktgen(self):
        self.send_command_wait_respond(self.command_stop_pktgen)

    def stop_pktgen_custom(self):
        self.send_command_wait_respond(self.command_stop_pktgen_custom)

    def pktgen_speed(self, filename, speed):
        self.send_command_wait_respond(
            self.command_pktgen_speed.with_argument([filename, speed]))

    def training(self):
        self.send_command_wait_respond(
            self.command_prepare.with_argument(["training.pcap"]))
        self.send_command_wait_respond(self.command_play)


class NFlambda(Agent):
    # Standard loopback interface address (localhost)
    NFLAMBDA_IP = 'ip_address'
    NFLAMBDA_PORT = 8093  # Port to listen on (non-privileged ports are > 1023)
    command_config = Command("config")
    command_complexity = Command("complexity")
    command_destroy = Command("destroy")
    command_init = Command("init")
    command_exit = Command("exit")
    command_clear = Command("clear")
    command_stop = Command("stop")
    command_stat = Command("stat")
    command_stat_data = Command("stat_data")
    command_start = Command("start")
    command_setup = Command("setup")
    command_switch = Command("switch")
    command_data = Command("data")
    command_dump_console = Command("dump").with_argument(["console"])
    command_dump_file = Command("dump").with_argument(["file"])
    command_dump_recover = Command("dump").with_argument(["recover"])
    command_config_GD_ASYN = command_config.with_argument(["GD_ASYN", "1"])
    command_config_GD_D_ASYN = command_config.with_argument(["GD_ASYN", "0"])
    command_config_GD_D_ASYN_STATE = command_config.with_argument(["GD_ASYN", "2"])


    def __init__(self):
        return Agent.__init__(self, self.NFLAMBDA_IP, self.NFLAMBDA_PORT)

    def data(self, num):
        self.send_command_wait_respond(
            self.command_stop.with_argument(["ETH"]))
        self.send_command_wait_respond(
            self.command_config.with_argument(["data", str(num)]))

    def get_stat(self):
        return self.send_command_wait_respond(self.command_stat)

    def init_nf(self):
        self.send_command_wait_respond(self.command_init.with_argument(["nf"]))

    def clear(self):
        for i in range(10):
            self.send_command_wait_respond(NFlambda.command_clear)

    # def get_stat_sum(self, maximum_core: int):
    #     result = []
    #     for j in range(10, 100, 500, 1000):
    #         self.change_complexity(j)
    #         for i in range(2, maximum_core + 1,2):
    #             result.append(self.get_stat(i))
    #     return result

    def start(self):
        self.send_command_wait_respond(self.command_start)

    def get_stat_mono_lb(self):
        return self.send_command_wait_respond(self.command_stat)
    # stop NF
    # destroy
    # config data number
    # setup EM
    # init nf
    # stop ETH
    # init port
    # start

    def config(self, command_list, parameter_list=[]):
        temp_command_list = []
        for command in (command_list):
            if(command == "control"):
                temp_command = self.command_config.with_argument(
                    ["control", str(parameter_list.pop(0))])
            elif(command == "switch"):
                temp_command = self.command_switch.with_argument(
                    [str(parameter_list.pop(0))])
            elif(command == "GM_data_complexity"):
                temp_command = self.command_config.with_argument(
                    ["GM_data_complexity", str(parameter_list.pop(0))])
            elif(command == "GM_control_complexity"):
                temp_command = self.command_config.with_argument(
                    ["GM_control_complexity", str(parameter_list.pop(0))])
            elif(command == "GM_frequency_control"):
                temp_command = self.command_config.with_argument(
                    ["GM_frequency_control", str(parameter_list.pop(0))])
            elif(command == "GM_control_interval"):
                temp_command = self.command_config.with_argument(
                    ["GM_control_interval", str(parameter_list.pop(0))])
            elif(command == "GD_data_complexity"):
                temp_command = self.command_config.with_argument(
                    ["GD_data_complexity", str(parameter_list.pop(0))])
            elif(command == "GD_control_complexity"):
                temp_command = self.command_config.with_argument(
                    ["GD_control_complexity", str(parameter_list.pop(0))])
            elif(command == "GD_control_frequency"):
                temp_command = self.command_config.with_argument(
                    ["GD_control_frequency", str(parameter_list.pop(0))])
            elif(command == "GD_control_size"):
                temp_command = self.command_config.with_argument(
                    ["GD_control_size", str(parameter_list.pop(0))])
            elif (command == "GD_data_size"):
                temp_command = self.command_config.with_argument(
                    ["GD_data_size", str(parameter_list.pop(0))])
            elif(command == "GM_control_size"):
                temp_command = self.command_config.with_argument(
                    ["GM_control_size", str(parameter_list.pop(0))])
            elif (command == "GM_data_size"):
                temp_command = self.command_config.with_argument(
                    ["GM_data_size", str(parameter_list.pop(0))])

            temp_command_list.append(temp_command)

        for temp_command in temp_command_list:
            self.send_command_wait_respond(temp_command)

    def change_gm_control_complexity(self, i: int):
        self.send_command_wait_respond(self.command_config.with_argument([
                                       "GM_control_complexity", str(i)]))
        return

    def change_entry(self, entry: int):
        return self.send_command_wait_respond(self.command_config.with_argument([str(entry)]))

    def exp_mono_lb(tf: TrafficGen, nflambda, trainingfile, testfile):
        tf.send_command_wait_respond(
            tf.command_prepare.with_argument([trainingfile]))
        tf.send_command_wait_respond(tf.command_play)
        tf.send_command_wait_respond(tf.command_play)
        tf.send_command_wait_respond(tf.command_stop)
        nflambda.send_command_wait_respond(
            nflambda.command_stop.with_argument(["NF"]))
        tf.send_command_wait_respond(
            tf.command_prepare.with_argument([testfile]))
        nflambda.clear()
        nflambda.send_command_wait_respond(nflambda.command_start)
        tf.send_command_wait_respond(tf.command_play)
        tf.send_command_wait_respond(tf.command_stop)
        print(nflambda.send_command_wait_respond(nflambda.command_stat))


class NFlambda_runner(Agent):
    # Standard loopback interface address (localhost)
    IP = 'ip_address'
    PORT = 3000  # Port to listen on (non-privileged ports are > 1023)
    command_run = Command("run")

    def __init__(self):
        return Agent.__init__(self, self.IP, self.PORT)

    