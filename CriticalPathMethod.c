/*
Team Report:

Name: Nicholas Lasagna
Course: CS 2413 - Data Structures
Assignment: Critical Path Method Solver
  
Contribution Summary:
I, Nicholas Lasagna, have completed this assignment with my partner, Manuel Perez. I wrote debugged logic errors, and tested the solution for various input files. My implementation includes:
- A modular design separating file input, graph construction, topological sort, and timing calculations.
- A breadth-first approach combining topological sorting and earliest/latest start time propagation.
- Added some more comments explaining each function and within those functions.

- Within this the use of an adjacency list for graph representation as per the assignment requirement, along with implementation of a cleanup function.
- Output formatting for tabular display of earliest/latest times, slack, and the critical path.
- Full comments explaining all functions, data structures, and logic used.
- Compliance with every rubric item, including a clean interface for file changes and readable code structure.

Changes by Manuel Perez Gil:
1. Renamed struct field `t` to `dest` for clearer indication of destination node.
2. Introduced `printActivityList()` to list activities read from the file.
3. Enhanced comments in each function to explain its purpose.
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
    int  src;
    int  dest; // renamed from `t` for clarity
    int  dur;
} Activity;

typedef struct Edge {
    int to, w;
    struct Edge *nxt;
} Edge;

// Global storage for activities and graph
static Activity act[MAX_ACT];
static Edge* g[MAX_NODE + 1] = {0};
static int indeg[MAX_NODE + 1] = {0};
static int earliest[MAX_NODE + 1] = {0};
static int latest[MAX_NODE + 1] = {0};
static int slack[MAX_NODE + 1] = {0};
static int parent[MAX_NODE + 1] = {0};
static int topo[MAX_NODE + 1] = {0};

static int actCnt = 0;
static int nodeCnt = 0;
static int topoLen = 0;

// Print list of activities as read from file
static void printActivityList(void) {
    printf("\nActivities (ID, Name, Src, Dest, Dur):\n");
    for (int i = 0; i < actCnt; i++)
        printf(" %c, %s, %d -> %d, %d\n", act[i].id, act[i].name, act[i].src, act[i].dest, act[i].dur);
}

// Add directed, weighted edge to adjacency list and track indegree
static void addEdge(int u, int v, int w) {
    Edge *e = malloc(sizeof *e);
    if (!e) {
        fprintf(stderr, "Error: memory allocation failed for edge %d->%d\n", u, v);
        exit(EXIT_FAILURE);
    }
    e->to = v;
    e->w = w;
    e->nxt = g[u];
    g[u] = e;
    indeg[v]++;
    // update node count to include both endpoints
    if (u > nodeCnt)
        nodeCnt = u;
    if (v > nodeCnt)
        nodeCnt = v;
}

// Read file of activities; build graph and populate act array
static void readFile(const char *fname) {
    FILE *fp = fopen(fname, "r");
    if (!fp) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }
    // first number is count of activities
    if (fscanf(fp, "%d", &actCnt) != 1) {
        fprintf(stderr, "Invalid activity count\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    // this reads each activity line and add its edge
    for (int i = 0; i < actCnt; i++) {
        if (fscanf(fp, " %c %63s %d %d %d",
                   &act[i].id,
                   act[i].name,
                   &act[i].src,
                   &act[i].dest,
                   &act[i].dur) != 5) {
            fprintf(stderr, "Error reading line %d\n", i + 2);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        addEdge(act[i].src, act[i].dest, act[i].dur);
    }
    fclose(fp);
}

// Forward pass: compute earliest start times and record topological order
static void forwardPass(void) {
    int q[MAX_NODE + 1], head = 0, tail = 0;
    int indegCopy[MAX_NODE + 1];
    memcpy(indegCopy, indeg, sizeof indegCopy);

    // Initialize queue with zero-indegree nodes
    for (int v = 1; v <= nodeCnt; v++) {
        earliest[v] = 0;
        parent[v] = 0;
        if (indegCopy[v] == 0)
            q[tail++] = v;
    }

    topoLen = 0;
    while (head < tail) {
        int u = q[head++];
        topo[topoLen++] = u;
        for (Edge *e = g[u]; e; e = e->nxt) {
            int cand = earliest[u] + e->w;
            if (cand > earliest[e->to]) {
                earliest[e->to] = cand;
                parent[e->to] = u;
            }
            if (--indegCopy[e->to] == 0)
                q[tail++] = e->to;
        }
    }
}

// Backward pass: compute latest start times and slack
static void backwardPass(void) {
    int projectLen = earliest[nodeCnt];
    // initialize latest[] to project length
    for (int v = 1; v <= nodeCnt; v++)
        latest[v] = projectLen;
    // traverse topo in reverse
    for (int i = topoLen - 1; i >= 0; i--) {
        int u = topo[i];
        for (Edge *e = g[u]; e; e = e->nxt) {
            int cand = latest[e->to] - e->w;
            if (cand < latest[u])
                latest[u] = cand;
        }
    }
    // slack = latest - earliest for each node
    for (int v = 1; v <= nodeCnt; v++)
        slack[v] = latest[v] - earliest[v];
}

// Print results table and critical path
static void printResults(void) {
    printf("\n%-6s %-18s %-18s %-10s\n", "Node", "Earliest", "Latest", "Slack");
    printf("------------------------------------------------\n");
    for (int v = 1; v <= nodeCnt; v++)
        printf("%-6d %-18d %-18d %-10d\n", v, earliest[v], latest[v], slack[v]);

    int path[MAX_NODE + 1], len = 0;
    for (int cur = nodeCnt; cur != 0; cur = parent[cur]) path[len++] = cur;
    printf("\nCritical path: ");
    for (int i = len - 1; i >= 0; i--) printf("%d%s", path[i], i ? " -> " : "\n");
    printf("Project length: %d\n", earliest[nodeCnt]);
}

// Free all allocated edges in graph
static void cleanup(void) {
    for (int u = 1; u <= nodeCnt; u++) {
        Edge *e = g[u];
        while (e) {
            Edge *next = e->nxt;
            free(e);
            e = next;
        }
        g[u] = NULL;
    }
}

int main(void) {
    // load data, display activities, run CPM, cleanup
    readFile(FILENAME);
    printActivityList(); // debug
    forwardPass(); // computer earliest times
    backwardPass(); // latest times
    printResults(); // output
    cleanup(); // free from malloc
    return 0;
}