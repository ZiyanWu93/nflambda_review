#!/usr/bin/env python
# coding: utf-8

# # Dependencies

# In[438]:


# dependency
import pandas as pd
import time
from time import sleep
from tqdm.notebook import tqdm
import signal
import matplotlib.pyplot as plt


# ## remote launch a program

# In[5]:


passwd = "Cat931008"


# In[9]:


# remotely launch the platform
import os
os.system(
    '''ssh aum "nohup echo "{}" | sudo -S /home/anon/CLionProjects/NF_Platform/build/nf_p > foo.out 2> foo.err < /dev/null &"'''.format(passwd)
)


# ## remote kill the program

# In[349]:


# remotely kills the platform
#ssh user@remote.host nohup python scriptname.py &
os.system(
    '''ssh aum "python /home/anon/PycharmProjects/nsdi_tools/main.py"''')


# ## get_microarchitecture_snapshot()

# In[442]:


import subprocess


# EXEC  : instructions per nominal CPU cycle
#IPC   : instructions per CPU cycle
#FREQ  : relation to nominal CPU frequency='unhalted clock ticks'/'invariant timer ticks' (includes Intel Turbo Boost)
# AFREQ : relation to nominal CPU frequency while in active state (not in power-saving C state)='unhalted clock ticks'/'invariant timer ticks while in C0-state'  (includes Intel Turbo Boost)
# L3MISS: L3 (read) cache misses
# L3HIT : L3 (read) cache hit ratio (0.00-1.00)
# L3MPI : number of L3 (read) cache misses per instruction
# L2MPI : number of L2 (read) cache misses per instruction
# READ  : bytes read from main memory controller (in GBytes)
# WRITE : bytes written to main memory controller (in GBytes)
# LOCAL : ratio of local memory requests to memory controller in %
# L3OCC : L3 occupancy (in KBytes)
# TEMP  : Temperature reading in 1 degree Celsius relative to the TjMax temperature (thermal headroom): 0 corresponds to the max temperature
# energy: Energy in Joules
    
    
def get_microarchitecture_snapshot():
    ssh = subprocess.Popen(['ssh', 'anon@aum', 'cat', '/home/anon/pcm/test.dat'],
                           stdout=subprocess.PIPE)
    lines = []
    for line in ssh.stdout:
        x = [i.decode('ascii') for i in line.split()]
        lines.append(x)

    dic = {}
    for line in lines[1:]:
        if(line[0]!='TOTAL'):
            key = int(line[0])
        else:
            key = 'TOTAL'
        dic[key] = {}
        for index,j in enumerate(lines[0]):
            dic[key][j] = line[index]
    return dic    


# In[6]:


get_microarchitecture_snapshot()


# ## dump_configuration

# In[439]:


import copy

def new_core_set():
    return {i for i in rand_range(0,48,1)}

def get_core_from_socket(core_set, socket):
    for i in core_set:
        if i%2 == socket:
            core_set.remove(i)
            return i
    return None

def get_data_control_cores(n_data_socket_0, n_control_socket_0,
                           n_data_socket_1, n_control_socket_1):
    dic = {}
    dic["data_0"] = set()
    dic["control_0"] = set()
    dic["data_1"] = set()
    dic["control_1"] = set()

    core_set = new_core_set()
    for i in rand_range(n_data_socket_0):
        dic["data_0"].add(get_core_from_socket(core_set, 0))

    for i in rand_range(n_control_socket_0):
        dic["control_0"].add(get_core_from_socket(core_set, 0))

    for i in rand_range(n_data_socket_1):
        dic["data_1"].add(get_core_from_socket(core_set, 1))

    for i in rand_range(n_control_socket_1):
        dic["control_1"].add(get_core_from_socket(core_set, 1))

    return dic


def get_cores_socket1(n_data_socket_1, n_control_socket_1):
    core_set = new_core_set()

    temp1 = []
    for i in rand_range(n_data_socket_1):
        temp1.append(get_core_from_socket(core_set, 1))

    temp2 = []
    for i in rand_range(n_control_socket_1):
        temp2.append(get_core_from_socket(core_set, 1))

    return [temp1,temp2]


def get_group(data_set,control_set):
    control_set = copy.deepcopy(control_set)
    data_set = copy.deepcopy(data_set)

    n_control_set = len(control_set)
    n_data_set = len(data_set)
    
    num_group = len(control_set)
    n_dfunctor_per_group = len(data_set) // len(control_set)
    remainder = len(data_set) % len(control_set)
    result = []
    for i in rand_range(num_group):
        temp = []
        temp.append(control_set.pop())
        for j in rand_range(n_dfunctor_per_group):
            temp.append(data_set.pop())
        result.append(temp)

    for i in rand_range(remainder):
        result[i].append(data_set.pop())

    return result


def generate_configuration_socket_1(core_n_list, control_n_list):
    result = []
    for num_core in core_n_list:
        for control_n in (control_n_list):
            if ((num_core - control_n) < control_n):
                continue

            temp = get_data_control_cores(0,
                                          0,
                                          n_data_socket_1=(num_core -
                                                           control_n),
                                          n_control_socket_1=control_n)
            result.append(get_group(temp["control_1"], temp["data_1"]))

    return result

def print_level(s,level):
    pre = ""
    for i in rand_range(level):
        pre = pre + "  "
    print("{}{}".format(pre, s))
    
def dump_group(group):
    print_level("control: {}".format(group[0]),2)
    print_level("data: {}".format(group[1:]),2)

def dump_plan(plan):
    print("group number: {}".format(len(plan)))
    i = 1
    for group in plan:
        print_level("group: {}".format(i),1)
        dump_group(group)
        i+=1
    
def dump_configuration(conf):
    print("how many: {}".format(len(conf)))
    i = 1
    for plan in conf:
        print("plan {}".format(i))
        dump_plan(plan)
        i+=1
        
        
        import copy

def new_core_set():
    return {i for i in rand_range(0,48,1)}

def get_core_from_socket(core_set, socket):
    for i in core_set:
        if i%2 == socket:
            core_set.remove(i)
            return i
    return None

def c(n_data_socket_0, n_control_socket_0,
                           n_data_socket_1, n_control_socket_1):
    dic = {}
    dic["data_0"] = set()
    dic["control_0"] = set()
    dic["data_1"] = set()
    dic["control_1"] = set()

    core_set = new_core_set()
    for i in rand_range(n_data_socket_0):
        dic["data_0"].add(get_core_from_socket(core_set, 0))

    for i in rand_range(n_control_socket_0):
        dic["control_0"].add(get_core_from_socket(core_set, 0))

    for i in rand_range(n_data_socket_1):
        dic["data_1"].add(get_core_from_socket(core_set, 1))

    for i in rand_range(n_control_socket_1):
        dic["control_1"].add(get_core_from_socket(core_set, 1))

    return dic


def get_cores_socket1(n_data_socket_1, n_control_socket_1):
    core_set = new_core_set()

    temp1 = []
    for i in rand_range(n_data_socket_1):
        temp1.append(get_core_from_socket(core_set, 1))

    temp2 = []
    for i in rand_range(n_control_socket_1):
        temp2.append(get_core_from_socket(core_set, 1))

    return [temp1,temp2]


def get_group(data_set,control_set):
    control_set = copy.deepcopy(control_set)
    data_set = copy.deepcopy(data_set)

    n_control_set = len(control_set)
    n_data_set = len(data_set)
    
    num_group = len(control_set)
    n_dfunctor_per_group = len(data_set) // len(control_set)
    remainder = len(data_set) % len(control_set)
    result = []
    for i in rand_range(num_group):
        temp = []
        temp.append(control_set.pop())
        for j in rand_range(n_dfunctor_per_group):
            temp.append(data_set.pop())
        result.append(temp)

    for i in rand_range(remainder):
        result[i].append(data_set.pop())

    return result

def group_data_n(group):
    return len(group) -1

def groups_data_n(groups):
    temp = 0
    for group in groups:
        temp = temp + group_data_n(group)
    return temp

# each element represents an execution model, which is a list of group
def generate_configuration_socket_1(core_n_list, control_n_list):
    result = []
    for num_core in core_n_list:
        for control_n in (control_n_list):
            if ((num_core - control_n) < control_n):
                continue

            temp = get_data_control_cores(0,
                                          0,
                                          n_data_socket_1=(num_core -
                                                           control_n),
                                          n_control_socket_1=control_n)
            result.append(get_group(temp["data_1"], temp["control_1"]))

    return result

def print_level(s,level):
    pre = ""
    for i in rand_range(level):
        pre = pre + "  "
    print("{}{}".format(pre, s))
    
def dump_group(group):
    print_level("control: {}".format(group[0]),2)
    print_level("data: {}".format(group[1:]),2)
    
def dump_plan(plan):
    print("group number: {}".format(len(plan)))
    i = 1
    for group in plan:
        print_level("group: {}".format(i),1)
        dump_group(group)
        i+=1
    
def dump_configuration(conf):
    print("how many: {}".format(len(conf)))
    i = 1
    for plan in conf:
        print("plan {}".format(i))
        dump_plan(plan)
        i+=1
        
conf = generate_configuration_socket_1([2,4,6,8,12],[2,4])

#dump_configuration(conf)
# get_data_control_cores(0,0,10,2)
# get_group({1, 3, 5, 7, 9, 11, 13, 15, 17, 19},{21, 23})
# get_cores_socket1(10,2)
conf


# In[440]:


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


# In[441]:


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
command_coordinator = Command("coordinator")

# Register an handler for the timeout
# def handler(signum, frame):
#     raise Exception("end of time")
      
# signal.signal(signal.SIGALRM, handler)


class NFlambda:
    NFLAMBDA_IP = 'ip_address'
    NFLAMBDA_PORT = 8093  # Port to listen on (non-privileged ports are > 1023)
    counter = 0
    
    def __init__(self, start=True):
        if (start == True):
            os.system(
                '''ssh aum "python /home/anon/PycharmProjects/nsdi_tools/main.py"'''
            )
            time.sleep(0.2)
            os.system(
                '''ssh aum "nohup echo "Cat931008" | sudo -S /home/anon/CLionProjects/NF_Platform/build/nf_p > foo.out 2> foo.err < /dev/null &"'''
            )
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
                     self.port))  #always throws Con Refused when tryed

                #print("Connected")
                #self.s.sendall(command.apply())
                #self.s.settimeout(10)
                data = self.s.recv(1024)
                self.counter = 0
                result = True
            except Exception as e:
                print(e)
                time.sleep(1)
                count = count + 1
                if(count==5):
                    raise ValueError('dont know __init__')
                    
    def send_command(self, command):
        count = 0
        try:
            
            self.s.sendall(command.apply())
            data = self.s.recv(1024)
            while(data==None):
                data = self.s.recv(1024)
                count = count + 1
                time.sleep(1)
                if(count==10):
                    raise ValueError('10 seconds with no response')
                
            return data.decode()

        except:
            result = None
            while result is None:
                try:
                    # connect
                    self.s.connect(
                        (self.ip,
                         self.port))  #always throws Con Refused when tryed

                    #print("Connected")
                    self.s.sendall(command.apply())
                    #self.s.settimeout(10)
                    data = self.s.recv(1024)
                    result = True
                    return data.decode()
                    
                except:
                    time.sleep(1)
                    count = count + 1
                    if(count==5):
                        raise ValueError('dont know send_command')

    def __del__(self):
#         os.system('''ssh aum "python /home/anon/PycharmProjects/nsdi_tools/main.py"''')
        self.s.shutdown(socket.SHUT_RDWR)
        #print("connection closed for {}".format(self.__class__))


# ## nflambda configure group 

# In[443]:


def dump_group(group):
    print_level("control: {}".format(group[0]),2)
    print_level("data: {}".format(group[1:]),2)
    
def nflambda_configure_group(nflambda,group):
    nflambda.send_command(command_em.with_argument(["assign", str(group[0]), "control"]))
    for i in group[1:]:
        nflambda.send_command(command_em.with_argument(["assign", str(i), "data"]))
        
    for i in group[1:]:
        nflambda.send_command(command_em.with_argument(["edge",str(group[0]),str(i)]))


# # FWD

# In[ ]:


nflambda = NFlambda(start=True)

nflambda.send_command(command_config.with_argument(["nf", "fwd"]))
nflambda.send_command(command_mempool.with_argument(["create"]))
nflambda.send_command(command_em.with_argument(["create"]))
for i in rand_range(1, 48, 2):
    nflambda.send_command(command_em.with_argument(["assign", str(i), "data"]))

nflambda.send_command(command_port.with_argument(["init"]))
nflambda.send_command(command_nfunctor.with_argument(["create"]))
nflambda.send_command(command_nfunctor.with_argument(["run"]))
result1 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
time.sleep(1)
result2 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
print((result2 - result1) * (84 / 64) / (10**9))


# # Generic decomposed

# In[143]:


def exp_generic_decomposed(configuration):
    try:
        (groups,flow_n,frequency) = configuration
        nflambda = NFlambda(start=True)
        nflambda.send_command(command_config.with_argument(["nf", "gd"]))
        nflambda.send_command(command_config.with_argument(["num_flow", str(flow_n)]))
        nflambda.send_command(command_config.with_argument(["control_frequency", str(frequency)]))
        nflambda.send_command(command_config.with_argument(["data_complexity", "5"]))
        nflambda.send_command(command_config.with_argument(["control_complexity", "20"]))
        nflambda.send_command(command_mempool.with_argument(["create"]))
        nflambda.send_command(command_em.with_argument(["create"]))

        for group in groups:
            nflambda_configure_group(nflambda,group)

        nflambda.send_command(command_port.with_argument(["init"]))
        nflambda.send_command(command_nfunctor.with_argument(["create"]))
        nflambda.send_command(command_nfunctor.with_argument(["run"]))
        result2 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
        time.sleep(0.5)
        result1 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
        return (result1 - result2) * 2 * (84 / 64) / (10**9)
    except:
        raise


# ## the impact of frequency on the performance
# relationship with the microarchitectur

# In[142]:


#core_n_list = [2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24]
core_n_list = [24]
control_n_list = [1, 2, 3, 4] # number of control actors
#frequency_list = [5,10,20,50,100] # frequency of interactions
flow_n_list = [1000000]
frequency_list = [1,5,10,20,30,40,50,60,70,80,90,100] # frequency of interactions
# nflambda_configure_group(nflambda,group)
import itertools

configuration = []
for (n_core, n_control,flow_n, frequency) in itertools.product(core_n_list, control_n_list, flow_n_list, frequency_list):
#     print(n_core, n_control, flow_n, frequency)
    n_data = n_core - n_control
    if(n_data<=n_control):
        continue
    to_be_allocated_cores = get_cores_socket1(n_data,n_control)
    data_core_set = to_be_allocated_cores[0]
    control_core_set = to_be_allocated_cores[1]
#     print(data_core_set,control_core_set)
    groups = get_group(data_core_set,control_core_set)
    for i in groups:
        pass
        
    configuration.append((groups,flow_n,frequency))

configuration


# In[144]:


i = 0
result = []
for i in tqdm(rand_range(len(configuration))):
    temp = None
    while temp is None:
        try:
            temp = exp_generic_decomposed(configuration[i])
            snapshot = get_microarchitecture_snapshot()
            result.append((temp,snapshot))
        except Exception as e: 
            print(e)


# In[145]:


result[::][1]


# In[147]:


core_socket_1 = [i for i in rand_range(1,49,2)]
result_2 = []
i = 0
for r in result:
    snapshot = r[1]
    data_cores = []
    for group in configuration[i][0]:
        data_cores = data_cores + group[1:]
    L3MPI = 0
    L2MPI = 0
    for key in snapshot:
        if(key in data_cores):
            L3MPI = L3HIT + float(snapshot[key]['L3MPI'])
            L2MPI = L2MPI + float(snapshot[key]['L2MPI'])
    result_2.append((configuration[i][0],len(configuration[i][0]),configuration[i][1],configuration[i][2], L3MPI/len(data_cores), L2MPI/len(data_cores)))
    i=i+1

df = pd.DataFrame(result_2 ,columns=['groups','control_n','flow_n', 'frequency',"L3MPI","L2MPI"]) 
df


# In[136]:


len(result)


# In[ ]:





# In[ ]:





# # Monolithic monitor

# In[ ]:


def flatten_list(_2d_list):
    flat_list = []
    # Iterate through the outer list
    for element in _2d_list:
        if type(element) is list:
            # If the element is of type list, iterate through the sublist
            for item in element:
                flat_list.append(item)
        else:
            flat_list.append(element)
    return flat_list


# In[ ]:


x = [1,2,4,6,8,10,12,14,16,18,20,22,24]
y = [1000,3000,5000,10000,50000,100000,200000,400000,50000,200000, 400000, 600000, 800000,1000000]
X = [x for i in y]
Y = [[i for j in x] for i in y]

np_x = np.array(X)
np_y = np.array(Y)

flattened_X = np_x.flatten()
flattened_Y = np_y.flatten()
flattened_Z = np.array([])

snapshots = []
for i in rand_range(len(flattened_X)):
    nflambda = NFlambda(start=True)
    nflambda.send_command(command_config.with_argument(["nf", "m_mn"]))
    nflambda.send_command(command_config.with_argument(["entry_size_shared", str(flattened_Y[i])]))
    nflambda.send_command(command_mempool.with_argument(["create"]))
    nflambda.send_command(command_em.with_argument(["create"]))
    temp = 1
    for j in rand_range(flattened_X[i]):
        nflambda.send_command(command_em.with_argument(["assign", str(temp), "data"]))
        temp = temp +2
    nflambda.send_command(command_port.with_argument(["init"]))
    nflambda.send_command(command_nfunctor.with_argument(["create"]))
    nflambda.send_command(command_nfunctor.with_argument(["run"]))
    result2 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
    time.sleep(1)
    result1 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
    flattened_Z = np.append(flattened_Z,(result1 - result2) * (84 / 64) / (10**9))
    snapshot = get_microarchitecture_snapshot()
    snapshots.append(snapshot)
    
Z = flattened_Z.reshape(np_x.shape)
np.save("X_1", X)
np.save("Y_1", Y)
np.save("Z_1", Z)


# ## relationship between the LLC
# as the shared state size increase, the LLC miss rate increases by a lot

# In[71]:


x = [24]
y = [1000,3000,5000,10000,50000,100000,200000,400000,50000,200000, 400000, 600000, 800000,1000000]
X = [x for i in y]
Y = [[i for j in x] for i in y]

np_x = np.array(X)
np_y = np.array(Y)

flattened_X = np_x.flatten()
flattened_Y = np_y.flatten()
flattened_Z = np.array([])

snapshots = []
for i in rand_range(len(flattened_X)):
    nflambda = NFlambda(start=True)
    nflambda.send_command(command_config.with_argument(["nf", "m_mn"]))
    nflambda.send_command(command_config.with_argument(["entry_size_shared", str(flattened_Y[i])]))
    nflambda.send_command(command_mempool.with_argument(["create"]))
    nflambda.send_command(command_em.with_argument(["create"]))
    temp = 1
    for j in rand_range(flattened_X[i]):
        nflambda.send_command(command_em.with_argument(["assign", str(temp), "data"]))
        temp = temp +2
    nflambda.send_command(command_port.with_argument(["init"]))
    nflambda.send_command(command_nfunctor.with_argument(["create"]))
    nflambda.send_command(command_nfunctor.with_argument(["run"]))
    time.sleep(0.3)
    result2 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
    time.sleep(1)
    result1 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
    flattened_Z = np.append(flattened_Z,(result1 - result2) * (84 / 64) / (10**9))
    snapshot = get_microarchitecture_snapshot()
    snapshots.append(snapshot)
    print(i)
    
Z = flattened_Z.reshape(np_x.shape)
# np.save("X_1", X)
# np.save("Y_1", Y)
# np.save("Z_1", Z)


# In[73]:


Z


# In[74]:


core_socket_1 = [i for i in rand_range(1,49,2)]
    
for snapshot in snapshots:
    L3HIT = 0
    for key in snapshot:
        if(key in core_socket_1):
            L3HIT = L3HIT + float(snapshot[key]['L3MPI'])
    print(L3HIT/24)
#    print(i['TOTAL']['L3HIT'])


# In[75]:


core_socket_1 = [i for i in rand_range(1,49,2)]
result = []
i = 0
for snapshot in snapshots:
    L3MPI = 0
    L2MPI = 0
    for key in snapshot:
        if(key in core_socket_1):
            L3MPI = L3HIT + float(snapshot[key]['L3MPI'])
            L2MPI = L2MPI + float(snapshot[key]['L2MPI'])
    result.append((y[i], L3MPI/24, L2MPI/24))
    i=i+1

df = pd.DataFrame(result, columns=['entry_size', "L3MPI","L2MPI"]) 
df
#    print(i['TOTAL']['L3HIT'])


# In[96]:


fig = plt.figure(dpi=300)
ax = df.plot(x='entry_size', y='L3MPI',figsize=(4,4),ax = plt.gca(),legend=False)
ax.set_ylabel("L3MPI")
ax.set_xlabel("number of entries")
xlabels = ['{}'.format((int)(x)) + 'K' for x in ax.get_xticks()/1000]
ax.set_xticklabels(xlabels)
plt.show()


# In[97]:


fig = plt.figure(dpi=300)
ax = df.plot(x='entry_size', y='L2MPI',figsize=(4,4),ax = plt.gca(),legend=False)
ax.set_ylabel("L2MPI")
ax.set_xlabel("number of entries")
xlabels = ['{}'.format((int)(x)) + 'K' for x in ax.get_xticks()/1000]
ax.set_xticklabels(xlabels)
plt.show()


# # Decomposed monitor

# ## performance

# In[ ]:


core_n_list = [2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24]
control_n_list = [1, 2, 3, 4] # number of control actors
flow_n_list = [1000, 3000, 5000, 10000, 50000, 100000, 200000,
     400000, 50000, 200000, 400000, 600000, 800000, 1000000] # number of flows
frequency_list = [5,10,20,50,100] # frequency of interactions


# In[ ]:


# nflambda_configure_group(nflambda,group)
import itertools

configuration = []
for (n_core, n_control, flow_n, frequency) in itertools.product(core_n_list, control_n_list, flow_n_list, frequency_list):
#     print(n_core, n_control, flow_n, frequency)
    n_data = n_core - n_control
    if(n_data<=n_control):
        continue
    to_be_allocated_cores = get_cores_socket1(n_data,n_control)
    data_core_set = to_be_allocated_cores[0]
    control_core_set = to_be_allocated_cores[1]
#     print(data_core_set,control_core_set)
    groups = get_group(data_core_set,control_core_set)
    for i in groups:
        pass
        
    configuration.append((groups, flow_n, frequency))

configuration


# In[ ]:


i = 0

result = []
def exp_decomposed_monitor(configuration):
    try:
        (groups,flow_n,frequency) = configuration
        nflambda = NFlambda(start=True)
        nflambda.send_command(command_config.with_argument(["nf", "d_mn"]))
        nflambda.send_command(command_config.with_argument(["num_flow", str(flow_n)]))
        nflambda.send_command(command_config.with_argument(
            ["interaction_counter", str(frequency)]))
        nflambda.send_command(command_mempool.with_argument(["create"]))
        nflambda.send_command(command_em.with_argument(["create"]))

        for group in groups:
            nflambda_configure_group(nflambda,group)

        nflambda.send_command(command_port.with_argument(["init"]))
        nflambda.send_command(command_nfunctor.with_argument(["create"]))
        nflambda.send_command(command_nfunctor.with_argument(["run"]))
        result2 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
        time.sleep(0.5)
        result1 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
        return (result1 - result2) * 2 * (84 / 64) / (10**9)
    except:
        raise
        

for i in tqdm(rand_range(len(configuration))):
    temp = None
    while temp is None:
        try:
            temp = exp_decomposed_monitor(configuration[i])
            result.append(temp)
        except:
            print(i)

# flattened_Z = np.append(flattened_Z,(result1 - result2) * (84 / 64) / (10**9))


# In[ ]:


import pickle
# with open('decomposed_monitor.pkl', 'wb') as f:
#     pickle.dump(result, f)
    
with open('decomposed_monitor.pkl', 'rb') as f:
    mynewlist = pickle.load(f)


# In[ ]:


def flatten(t):
    return [item for sublist in t for item in sublist]


# In[ ]:


result = []
for i in rand_range(len(configuration)):
    temp = [len(flatten(configuration[i][0])), configuration[i][1]] + [mynewlist[i]]
    result.append(temp)
result


# In[ ]:


coordinate = []
for i in rand_range(len(configuration)):
    temp = [len(flatten(configuration[i][0]))] + [configuration[i][1]]
    coordinate.append(temp)
coordinate = [list(x) for x in set(tuple(x) for x in coordinate)]
coordinate


# In[ ]:


def maximum_element(l):
    temp = l[0]
    for i in l:
        if i >temp:
            temp = i
    if(temp>95):
        return 95
    return temp


# In[ ]:


r = []
for i in coordinate:
    temp = []
    for j in result:
        if i == j[:2]:
            temp.append(j[2])
    if(temp!=[]):
        r.append(i+[maximum_element(temp)])
r


# In[ ]:


d = {}
for i in r:
    d[tuple(i[:2])] = i[2]
d


# In[ ]:


x = list(set([i[0] for i in r]))
y = list(set([i[1] for i in r]))


# In[ ]:


X = [x for i in y]
Y = [[i for j in x] for i in y]


# In[ ]:


np_x = np.array(X)
np_y = np.array(Y)

flattened_X = np_x.flatten()
flattened_Y = np_y.flatten()
flattened_Z = np.array([])


# In[ ]:


for i in rand_range(len(flattened_X)):
    flattened_Z = np.append(flattened_Z,d[tuple([flattened_X[i],flattened_Y[i]])])


# In[ ]:


Z = flattened_Z.reshape(np_x.shape)


# In[ ]:


np.save("X_2", X)
np.save("Y_2", Y)
np.save("Z_2", Z)


# ## the impact of frequency on the performance (X)
# relationship with the microarchitectur

# In[5]:


core_n_list = [24]
control_n_list = [1, 2, 3, 4,5,6,7,8] # number of control actors
flow_n_list = [10000] # number of flows
num_entry_list = [10000]
frequency_list = [1,2,3,4,5,6,7,8,9,10] # frequency of interactions
# frequency_list = [1] # frequency of interactions


# In[14]:


import itertools
def generate_dm_configuration(core_n_list,control_n_list,flow_n_list,frequency_list):
    configuration = []
    for (n_core, n_control, flow_n, frequency) in itertools.product(core_n_list, control_n_list, flow_n_list, frequency_list):
    #     print(n_core, n_control, flow_n, frequency)
        n_data = n_core - n_control
        if(n_data<=n_control):
            continue
        to_be_allocated_cores = get_cores_socket1(n_data,n_control)
        data_core_set = to_be_allocated_cores[0]
        control_core_set = to_be_allocated_cores[1]
    #     print(data_core_set,control_core_set)
        groups = get_group(data_core_set,control_core_set)
        for i in groups:
            pass
        configuration.append((groups, flow_n, frequency))
    return configuration


# In[148]:


def exp_decomposed_monitor(configuration):
    try:
        (groups,flow_n,frequency) = configuration
        nflambda = NFlambda(start=True)
        nflambda.send_command(command_config.with_argument(["nf", "d_mn"]))
        nflambda.send_command(command_config.with_argument(["num_flow", str(flow_n)]))
        nflambda.send_command(command_config.with_argument(
            ["interaction_counter", str(frequency)]))
        nflambda.send_command(command_mempool.with_argument(["create"]))
        nflambda.send_command(command_em.with_argument(["create"]))

        for group in groups:
            nflambda_configure_group(nflambda,group)

        nflambda.send_command(command_port.with_argument(["init"]))
        nflambda.send_command(command_nfunctor.with_argument(["create"]))
        nflambda.send_command(command_nfunctor.with_argument(["run"]))
        result2 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
        time.sleep(1)
        result1 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
        return (result1 - result2)  * (84 / 64.0) / (10**9)
    except:
        raise
        
def exp_wrapper(configuration):
    result = []
    snapshots = []
    for i in tqdm(rand_range(len(configuration))):
        temp = None
        while temp is None:
            try:
                temp = exp_decomposed_monitor(configuration[i])
                snapshot = get_microarchitecture_snapshot()
                snapshots.append(snapshot)
                result.append(temp)
            except Exception as e: 
                print(e)
    return (result,snapshots)
        
# flattened_Z = np.append(flattened_Z,(result1 - result2) * (84 / 64) / (10**9))


# In[164]:


# result, configuration, snapshots
def calculate(snapshots, result, configuration):
    core_socket_1 = [i for i in rand_range(1,49,2)]
    result_2 = []
    i = 0
    for snapshot in snapshots:
        L3MPI = 0
        L2MPI = 0
        L3HIT = 0

        data_cores = []
        for group in configuration[i][0]:
            data_cores = data_cores + group[1:]

        for key in snapshot:
            if(key in data_cores):
                L3MPI = L3MPI + float(snapshot[key]['L3MPI'])
                L2MPI = L2MPI + float(snapshot[key]['L2MPI'])
                L3HIT = L2MPI + float(snapshot[key]['L3HIT'])
        result_2.append((result[i],configuration[i][0], len(configuration[i][0]),configuration[i][1],configuration[i][2],L3MPI/len(data_cores), L2MPI/len(data_cores),L3HIT/len(data_cores)))
        i=i+1
    df = pd.DataFrame(result_2, columns=['throughput','groups','control_n','flow_n','frequency',"L3MPI","L2MPI",'L3HIT'])
    df['data_n'] = df.apply(lambda row: groups_data_n(row.groups), axis=1)
    return df

def mn_lot(df):
    
    # df
    #    print(i['TOTAL']['L3HIT'])

    fig, ax = plt.subplots(figsize=(10,4))
    for key, grp in df.groupby(['control_n']):
        ax.plot(grp['frequency'], grp['L3MPI'], label=key)

    ax.set_xlabel("number of packets invoking control actions")
    ax.set_ylabel("L3MPI")

    ax.legend()
    plt.show()

    fig, ax = plt.subplots(figsize=(10,4))
    for key, grp in df.groupby(['control_n']):
        ax.plot(grp['frequency'], grp['L2MPI'], label=key)

    # plt.ylim([0.002, 0.007])
    ax.legend()
    plt.show()


    fig, ax = plt.subplots(figsize=(10,4))
    for key, grp in df.groupby(['control_n']):
        ax.plot(grp['frequency'], grp['throughput'], label=key)

    # plt.ylim([0.002, 0.007])
    ax.legend()
    plt.show()


    fig, ax = plt.subplots(figsize=(10,4))
    for key, grp in df.groupby(['control_n']):
        ax.plot(grp['frequency'], grp['L3HIT'], label=key)

    # plt.ylim([0.002, 0.007])
    ax.legend()
    plt.show()


# In[147]:


core_n_list = [2,4,6,8,10,12,14,16,18,20,22,24]
control_n_list = [1,2,4,6,8,10] # number of control actors
flow_n_list = [10000,20000,40000,80000,160000,320000,640000,1280000] # number of flows
frequency_list = [40*i for i in rand_range(1,5)] # frequency of interactions
# frequency_list = [1] # frequency of interactions
# nflambda_configure_group(nflambda,group)
configuration = generate_dm_configuration(core_n_list,control_n_list,flow_n_list,frequency_list)
configuration
result, snapshots = exp_wrapper(configuration)


# In[183]:


core_n_list = [2,4,6,8,10,12,14,16,18,20,22,24]
control_n_list = [1] # number of control actors
flow_n_list = [1280000] # number of flows
frequency_list = [i for i in rand_range(1,10)] # frequency of interactions
# frequency_list = [1] # frequency of interactions
# nflambda_configure_group(nflambda,group)
configuration = generate_dm_configuration(core_n_list,control_n_list,flow_n_list,frequency_list)
configuration
result, snapshots = exp_wrapper(configuration)


# In[185]:


df = df.append(calculate(snapshots, result, configuration))


# In[187]:


for key, grp in df.groupby(['control_n','flow_n','frequency']):
#     print(key)
#     print(grp)
    if(key[0]==1 and key[1] == 1280000):
        fig, ax = plt.subplots(figsize=(10,4))
        ax.plot(grp['data_n'], grp['throughput'], label=key)
        ax.legend()
        plt.show()

# plt.ylim([0.002, 0.007])


# In[ ]:





# In[ ]:





# In[ ]:





# In[ ]:





# In[ ]:





# In[ ]:





# ## Traffic Generator

# In[444]:


nflambda = NFlambda(start=False)

nflambda.send_command(command_config.with_argument(["nf", "d_tf"]))
nflambda.send_command(command_mempool.with_argument(["create"]))
nflambda.send_command(command_em.with_argument(["create"]))
nflambda.send_command(command_em.with_argument(["assign", str(1), "data"]))
nflambda.send_command(command_em.with_argument(["assign", str(3), "control"]))
nflambda.send_command(command_em.with_argument(["edge",str(1),str(3)]))
nflambda.send_command(command_port.with_argument(["init"]))
nflambda.send_command(command_nfunctor.with_argument(["create"]))


# In[445]:


nflambda.send_command(command_nfunctor.with_argument(["run"]))


# In[446]:


for i in rand_range(100000):
    nflambda.send_command(command_coordinator.with_argument(["send"]))


# In[269]:


result1 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
time.sleep(1)
result2 = (int)(nflambda.send_command(command_dump.with_argument(["stat"])))
print((result2 - result1) * (84 / 64) / (10**9))


# In[ ]:





# # 3-D plot example

# In[ ]:





# In[ ]:





# In[ ]:





# In[ ]:


NFlambda.counter


# In[ ]:


try:
    signal.alarm(10)
    sleep(10)
except:
    print("handler")


# In[ ]:




