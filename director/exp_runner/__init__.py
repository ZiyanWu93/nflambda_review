from em import *
from nflambda import *

def dictionary_average(dictionary,core_list):
    result = {}
    total = 0
    for i in core_list:
        total = total  + (int)(dictionary[i]["L3OCC"])
    result["average_L3OCC"] = total/len(core_list)
    total = 0
    for i in core_list:
        total = total  + (int)(dictionary[i]["L3MISS"])
    result["average_L3MISS"] = total/len(core_list)
    total = 0
    for i in core_list:
        total = total  + (int)(dictionary[i]["L2MISS"])
    result["average_L2MISS"] = total/len(core_list)
    total = 0
    for i in core_list:
        total = total  + (float)(dictionary[i]["L3MPI"])
    result["average_L3MPI"] = total/len(core_list)
    total = 0
    for i in core_list:
        total = total  + (float)(dictionary[i]["L2MPI"])
    result["average_L2MPI"] = total/len(core_list)
    return result

#merge dictionary as state
def result_dic_result(result):
    key_list = result[0].keys()
    new_dict = {}
    for i in key_list:
        for j in result:
            new_dict.setdefault(i,[]).append(j[i])
    return new_dict

def exp1_rand_size_throughput(rand_range):
    e_m = execution_model(Sfc([nf_generic_decomposed]), [[24, 24, 24]], decouple=False)
    # layout = e_m.execution_graph.layout("tree")
    # g = e_m.execution_graph
    # plot(g, layout=layout, vertex_size=20, bbox=(300, 300), vertex_label=e_m.label_list, vertex_color=e_m.vertex_color,
    #      margin=50, shape="rectangle")
    # plot(g, layout=layout, vertex_size=20,bbox=(500, 500),vertex_label = e_m.label_list, margin=100,shape="rectangle")
    parameter_list = []
    p_1 = Parameter()
    p_1.d["state_size"] = 1000000
    p_1.d["data_state"] = rand_range
    p_1.d["control_state"] = rand_range
    nflambda = NFlambda(start=True)
    nflambda.deployment(e_m.deploy_action(), e_m.deploy_state(), e_m.deploy_paths(), p_1)
    nflambda.start_port()
    nflambda.run()
    result = nflambda.get_snapshot(sampling_time=1)
    dic_temp = {}
    dic_temp["state size"] = rand_range
    dic_temp["throughput"] = result[0]
    return dic_temp | dictionary_average(result[1], [i for i in range(1,49,2)])

def exp2_rand_size_throughput(rand_range):
    e_m = execution_model(Sfc([nf_generic_decomposed,nf_generic_decomposed]), [[24, 24, 24], [24, 24, 24]], decouple=False)
    # layout = e_m.execution_graph.layout("tree")
    # g = e_m.execution_graph
    # plot(g, layout=layout, vertex_size=20, bbox=(300, 300), vertex_label=e_m.label_list, vertex_color=e_m.vertex_color,
    #      margin=50, shape="rectangle")
    # plot(g, layout=layout, vertex_size=20,bbox=(500, 500),vertex_label = e_m.label_list, margin=100,shape="rectangle")
    parameter_list = []
    p_1 = Parameter()
    p_1.d["rand_range"] = rand_range
    p_1.d["state_size"] = 1000000
    p_1.d["data_state"] = rand_range
    p_1.d["control_state"] = rand_range
    nflambda = NFlambda(start=True)
    nflambda.deployment(e_m.deploy_action(), e_m.deploy_state(), e_m.deploy_paths(), p_1)
    nflambda.start_port()
    nflambda.run()
    result = nflambda.get_snapshot(sampling_time=1)
    dic_temp = {}
    dic_temp["state size"] = rand_range
    dic_temp["throughput"] = result[0]
    return dic_temp | dictionary_average(result[1], [i for i in range(1,49,2)])

def exp_frequency_state_size_throughput(frequency,control_state_size):
    e_m = execution_model(Sfc([nf_generic_decomposed]), [[24, 24, 24]], decouple=False)
    # layout = e_m.execution_graph.layout("tree")
    # g = e_m.execution_graph
    # plot(g, layout=layout, vertex_size=20, bbox=(300, 300), vertex_label=e_m.label_list, vertex_color=e_m.vertex_color,
    #      margin=50, shape="rectangle")
    # plot(g, layout=layout, vertex_size=20,bbox=(500, 500),vertex_label = e_m.label_list, margin=100,shape="rectangle")
    parameter_list = []
    p_1 = Parameter()
    p_1.d["control_frequency"] = frequency
    p_1.d["state_size"] = 1000000
    p_1.d["data_state"] = 10000
    p_1.d["control_state"] = control_state_size
    nflambda = NFlambda(start=True)
    nflambda.deployment(e_m.deploy_action(), e_m.deploy_state(), e_m.deploy_paths(), p_1)
    nflambda.start_port()
    nflambda.run()
    result = nflambda.get_snapshot(sampling_time=1)
    dic_temp = {}
    dic_temp["control state size"] = control_state_size
    dic_temp["frequency"] = frequency
    dic_temp["throughput"] = result[0]
    return dic_temp | dictionary_average(result[1], [i for i in range(1,49,2)])