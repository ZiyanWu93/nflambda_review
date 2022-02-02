class Command:
    def __init__(self, command_str, argument_list=[]):
        self.space = " "
        self.enter = "\n"
        self.command = command_str
        self.argument_list = argument_list

    def with_argument(self, argument_list):
        return Command(self.command, argument_list)

    def apply(self):
        result = self.command
        for arg in self.argument_list:
            result = result + self.space + arg
        # print(result)
        return (result + self.enter).encode()

import os
import socket
import time
command_config = Command("config")
command_mempool = Command("mempool")
command_em = Command("em")
command_nf = Command("nf")
command_nfunctor = Command("nfunctor")
command_port = Command("port")
command_dump = Command("dump")


class NFlambda:
    NFLAMBDA_IP = 'ip_address'
    NFLAMBDA_PORT = 8093  # Port to listen on (non-privileged ports are > 1023)

    def __init__(self, start=True):
        if (start == True):
            os.system(
                '''ssh aum "python /home/anon/PycharmProjects/nsdi_tools/main.py"'''
            )
            #             time.sleep(1)
            os.system(
                '''ssh aum "nohup echo "Cat931008" | sudo -S /home/anon/CLionProjects/NF_Platform/build/nf_p > foo.out 2> foo.err < /dev/null &"'''
            )
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.settimeout(10)
        self.ip = self.NFLAMBDA_IP
        self.port = self.NFLAMBDA_PORT
        # Connect the socket to the port where the server is listening
        server_address = (self.ip, self.port)


    #         print('Connecting To %s Port %s' % server_address)
    #         while True:
    #             try:            #Cant get it to make connection after retrying
    #                 self.s.connect((self.ip, self.port))       #always throws Con Refused when tryed
    #                 print("Connected")
    #                 break
    #             except socket.error:
    #                 pass

    def send_command(self, command):
        try:
            self.s.sendall(command.apply())
            data = self.s.recv(1024)
            return data.decode()

        except:
            server_address = (self.ip, self.port)
            #print('Connecting To %s Port %s' % server_address)
            while True:
                try:  #Cant get it to make connection after retrying
                    self.s.connect(
                        (self.ip,
                         self.port))  #always throws Con Refused when tryed
                    #print("Connected")
                    self.s.sendall(command.apply())
                    data = self.s.recv(1024)
                    return data.decode()
                    break
                except socket.error:
                    pass

    def __del__(self):
        self.s.close()
        #print("connection closed for {}".format(self.__class__))

nflambda = NFlambda(start=True)

nflambda.send_command(command_config.with_argument(["nf", "gd"]))
nflambda.send_command(command_config.with_argument(["data_complexity", "5"]))
nflambda.send_command(command_config.with_argument(["control_complexity", "20"]))
nflambda.send_command(command_config.with_argument(["control_frequency", "100"]))
nflambda.send_command(command_mempool.with_argument(["create"]))
nflambda.send_command(command_em.with_argument(["create"]))
nflambda.send_command(command_em.with_argument(["assign", str(1), "data"]))
nflambda.send_command(command_em.with_argument(["assign", str(3), "data"]))
nflambda.send_command(command_em.with_argument(["assign", str(5), "data"]))
nflambda.send_command(command_em.with_argument(["assign",str(7),"control"]))
nflambda.send_command(command_em.with_argument(["edge",str(7),str(1)]))
nflambda.send_command(command_em.with_argument(["edge",str(7),str(3)]))
nflambda.send_command(command_em.with_argument(["edge",str(7),str(5)]))
nflambda.send_command(command_port.with_argument(["init"]))
nflambda.send_command(command_nfunctor.with_argument(["create"]))
nflambda.send_command(command_nfunctor.with_argument(["run"]))
result2 = (float)(nflambda.send_command(command_dump.with_argument(["stat"])))
time.sleep(1)
result1 = (float)(nflambda.send_command(command_dump.with_argument(["stat"])))
print((result1 - result2) * (84 / 64.0) / (10**9))
print("done")