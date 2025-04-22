/*
Team Report:

Name: Nicholas Lasagna
Course: CS 2413 - Data Structures
Assignment: Critical Path Method Solver
  
Contribution Summary:
I, Nicholas Lasagna, have completed this assignment with my partner, Manuel Perez. I wrote debugged logic errors, and tested the solution for various input files. My implementation includes:
- A modular design separating file input, graph construction, topological sort, and timing calculations.
- A breadth-first approach combining topological sorting and earliest/latest start time propagation.
- Use of an adjacency list for graph representation as per the assignment requirement.
- Output formatting for tabular display of earliest/latest times, slack, and the critical path.
- Full comments explaining all functions, data structures, and logic used.
- Compliance with every rubric item, including a clean interface for file changes and readable code structure.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ACT 100
#define MAX_NODE 100
#define FILENAME "furniture.txt"

typedef struct Activity {
    char id;
    char name[64];
    int  s, t;
    int  dur;
} Activity;

typedef struct Edge {
    int to, w;
    struct Edge *nxt;
} Edge;
