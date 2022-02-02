#include "em.h"

//void setup_execution_model(struct execution_model_parameter *em_p) {
//    DEBUG_PRINT("setup_execution_model started\n");
//    *(em_p->SFG) = createAGraph(em_p->core_list->core_number);
//    set_reset(em_p->control_set);
//    set_reset(em_p->data_set);
//
//    int control_count = 0;
//    int data_count = 0;
//    for (int i = 1; i <= em_p->core_list->core_number; i++) {
//        if(control_count == em_p->ctl_num && data_count == em_p->data_num){
//            break;
//        }
//        if (control_count < em_p->ctl_num) {
//            // assign_property(c->execution_model.SFG, c->core_list.core_id[i], CONTROL);
//            if (em_p->core_list->core_id[i - 1] % 2 == 0) {
//                set_insert(em_p->control_set, em_p->core_list->core_id[i - 1]);
//                control_count++;
//            } else {
//                set_insert(em_p->data_set, em_p->core_list->core_id[i - 1]);
//                printf("add %d\n", em_p->core_list->core_id[i - 1]);
//                data_count++;
//            }
//            continue;
//        } else {
//            // assign_property(c->execution_model.SFG, c->core_list.core_id[i], DATA);
//            set_insert(em_p->data_set, em_p->core_list->core_id[i - 1]);
//            printf("add %d\n", em_p->core_list->core_id[i - 1]);
//            data_count++;
//        }
//    }
//    printf("%d\n", data_count);
//    int num_split = em_p->control_set->num;
//    struct Set **data_set_splitted = set_split(em_p->data_set, num_split);
//    int i = 0;
//    struct Set_iterator *s = set_iterator_create(em_p->control_set);
//    while (s->index != 100) {
//        if (set_iterator_get(s) == true) {
//            add_control_data(*(em_p->SFG), s->index, data_set_splitted[i]);
//            i++;
//        }
//        set_iterator_next(s);
//    }
//    printGraph(*(em_p->SFG));
//    DEBUG_PRINT("setup_execution_model done\n");
//}
//

void setup_execution_model(struct execution_model_parameter *em_p)
{
    DEBUG_PRINT("setup_execution_model started\n");
    *(em_p->SFG) = createAGraph(em_p->core_list->core_number);
    set_reset(em_p->control_set);
    set_reset(em_p->data_set);

    for (int i = 1; i <= em_p->core_list->core_number; i++)
    {
        if (i <= em_p->ctl_num)
        {
            // assign_property(c->execution_model.SFG, c->core_list.core_id[i], CONTROL);zz
            set_insert(em_p->control_set, em_p->core_list->core_id[i - 1]);
        }
        else if (i <= em_p->ctl_num + em_p->data_num)
        {
            // assign_property(c->execution_model.SFG, c->core_list.core_id[i], DATA);
            set_insert(em_p->data_set, em_p->core_list->core_id[i - 1]);
        }
    }
    int num_split = em_p->control_set->num;
    struct Set **data_set_splitted = set_split(em_p->data_set, num_split);
    int i = 0;
    struct Set_iterator *s = set_iterator_create(em_p->control_set);
    while (s->index != 100)
    {
        if (set_iterator_get(s) == true)
        {
            add_control_data(*(em_p->SFG), s->index, data_set_splitted[i]);
            i++;
        }
        set_iterator_next(s);
    }
    printGraph(*(em_p->SFG));
    DEBUG_PRINT("setup_execution_model done\n");
}
