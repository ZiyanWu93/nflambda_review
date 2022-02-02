#ifndef COMMON_GRAPH_H
#define COMMON_GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include "set.h"
enum Property
{
    UNKNOWN,
    CONTROL,
    DATA
};

struct node
{
    int vertex;
    struct node *next;
};

struct Graph
{
    int numVertices;
    struct node **adjLists;
    enum Property property_list[100];
};
struct node *createNode(int v);
struct Graph *createAGraph(int vertices);
void addEdge(struct Graph *graph, int s, int d);
void assign_property(struct Graph *graph, int s, enum Property property);
void printGraph(struct Graph *graph);
void add_control_data(struct Graph *graph, int control_node, struct Set *data_set);
#endif /* COMMON_GRAPH */
