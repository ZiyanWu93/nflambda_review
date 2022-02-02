#include "graph.h"

// Create a node
struct node *createNode(int v)
{
    struct node *newNode = malloc(sizeof(struct node));
    newNode->vertex = v;
    newNode->next = NULL;
    return newNode;
}

// Create a graph
struct Graph *createAGraph(int vertices)
{
    struct Graph *graph = malloc(sizeof(struct Graph));
    graph->numVertices = vertices;

    graph->adjLists = malloc(vertices * sizeof(struct node *));

    int i;
    for (i = 0; i < vertices; i++)
    {
        graph->adjLists[i] = NULL;
        graph->property_list[i] = UNKNOWN;
    }
    return graph;
}

// Add edge
void addEdge(struct Graph *graph, int s, int d)
{
    // Add edge from s to d
    struct node *temp = graph->adjLists[s];
    while (temp)
    {
        if (temp->vertex == d)
        {
            return;
        }
        else
        {
            temp = temp->next;
        }
    }
    struct node *newNode = createNode(d);
    newNode->next = graph->adjLists[s];
    graph->adjLists[s] = newNode;

    // Add edge from d to s
}

void assign_property(struct Graph *graph, int s, enum Property property)
{
    graph->property_list[s] = property;
}

// Print the graph
void printGraph(struct Graph *graph)
{
    int v;
    for (v = 0; v < graph->numVertices; v++)
    {
        struct node *temp = graph->adjLists[v];
        if (graph->property_list[v] == CONTROL)
        {
            printf("\n control %d: ", v);
        }
        else if (graph->property_list[v] == DATA)
        {
            continue;
            printf("\n data %d: ", v);
        }
        else if (graph->property_list[v] == UNKNOWN)
        {
            continue;
            printf("\n unknown %d: ", v);
        }
        while (temp)
        {

            if (graph->property_list[temp->vertex] == CONTROL)
            {
                printf("control %d ", temp->vertex);
            }
            else if (graph->property_list[temp->vertex] == DATA)
            {
                printf("data %d ", temp->vertex);
            }
            else if (graph->property_list[temp->vertex] == UNKNOWN)
            {
                // continue;
                printf("unknown %d ", temp->vertex);
            }
            temp = temp->next;
        }
        printf("\n");
    }
}

void add_control_data(struct Graph *graph, int control_node, struct Set *data_set)
{
    assign_property(graph, control_node, CONTROL);
    struct Set *set_copied = set_copy(data_set);
    struct Set_iterator *s_2 = set_iterator_create(set_copied);
    int index = 0;
    while (s_2->index != 100)
    {
        index = s_2->index;
        if (set_copied->element[index] == true)
        {
            assign_property(graph, index, DATA);
            addEdge(graph, control_node, index);
        }
        set_iterator_next(s_2);
    }
    printf("done\n");
}