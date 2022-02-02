import pysftp
import os
import pickle
import pandas as pd

def configure_pcm_cat_class(class_id, way_list = []):
    # way 0 => 1
    # way 1 => 2
    # way 3 => 5

    way_mask = 0
    for i in way_list:
        way_mask = way_mask + 2 ** i

    inner_command  = "sudo allocation_app_l3cat {class_id} {way_mask}".format(class_id = class_id, way_mask = hex(way_mask))
    # print(inner_command)


    os.system(
        '''ssh ip_address "{inner_command}" '''.format(inner_command = inner_command)
    )
    return


def configure_pcm_association(class_id, core_id):

    inner_command  = "sudo association_app {class_id} {core_id}".format(class_id = class_id, core_id = core_id)

    os.system(
        '''ssh ip_address "{inner_command}" '''.format(inner_command = inner_command)
    )
    return
