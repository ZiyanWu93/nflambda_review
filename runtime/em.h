#ifndef EM_H
#define EM_H
#include "common/defined.h"
#include "common/set.h"
#include "common/debug.h"
#include "common.h"

struct execution_model_parameter
{
    unsigned int data_num;
    unsigned int ctl_num;
    
    struct Core_list *core_list;
    struct Set *data_set;
    struct Set *control_set;
    struct Graph **SFG;
    struct Execution_model* p_em;
};

struct Execution_model
{
    enum NF_TYPE NF_Type[100];
    unsigned int data_num;
    unsigned int ctl_num;
    struct Set *data_set;
    struct Set *control_set;
    struct Graph *SFG;
};

void setup_execution_model(struct execution_model_parameter *em_p);

#endif /* EM */
