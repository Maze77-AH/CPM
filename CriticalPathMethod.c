/*
Team Report:

Name: Nicholas Lasagna
Course: CS 2413 - Data Structures
Assignment: Critical Path Method Solver
  
Contribution Summary:
I, Nicholas Lasagna, have completed this assignment with my partner, Manuel Perez. I wrote debugged logic errors, and tested the solution for various input files. My implementation includes:
- A modular design separating file input, graph construction, topological sort, and timing calculations.
- A breadth-first approach combining topological sorting and earliest/latest start time propagation.

- Within this the use of an adjacency list for graph representation as per the assignment requirement, along with implementation of a cleanup function.
- Output formatting for tabular display of earliest/latest times, slack, and the critical path.
- Full comments explaining all functions, data structures, and logic used.
- Compliance with every rubric item, including a clean interface for file changes and readable code structure.

I Manuel Perez, 
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
    int  src, t;
    int  dur;
} Activity;

typedef struct Edge {
    int to, w;
    struct Edge *nxt;
} Edge;

// Global Vars
static Activity act[MAX_ACT];
static Edge* g [MAX_NODE] = {0};

static int indeg[MAX_NODE] = {0};
static int earliest[MAX_NODE] = {0};
static int latest[MAX_NODE] = {0};
static int slack[MAX_NODE] = {0};
static int parent[MAX_NODE] = {0};
static int topo[MAX_NODE] = {0};

static int actCnt = 0, nodeCnt = 0, topoLen = 0;

// Helper
static void addEdge(int u, int v, int w) {
    Edge *e = malloc(sizeof *e);
    if (!e) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(EXIT_FAILURE);
    }
    e->to = v;
    e->w = w;
    e->nxt = g[u];
    g[u] = e;
    indeg[v]++;

    if (u > nodeCnt)
        nodeCnt = u;
    if (v > nodeCnt)
        nodeCnt = v;
}

// Can read file and if error exits the program.
static void readFile(const char *fname) {
    FILE *fp = fopen(fname, "r");
    if (!fp) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    if (fscanf(fp, "%d", &actCnt) != 1 || actCnt < 1 || actCnt > MAX_ACT) {
        fprintf(stderr, "Error: invalid actCount\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < actCnt; i++) {
        if (fscanf(fp, " %c %63s %d %d %d",
                   &act[i].id,
                   act[i].name,
                   &act[i].src,
                   &act[i].t,
                   &act[i].dur) != 5) {
            fprintf(stderr, "Error: bad format #%d\n", i+1);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        addEdge(act[i].src, act[i].t, act[i].dur);
    }
    fclose(fp);
}

// Used to pass earliest

static void forwardPass(void) {
    int q[MAX_NODE], head = 0, tail = 0;
    int indegCopy[MAX_NODE];

    // Indeg is copyies in order for backwardPass to have a non-destroyed version
    memcpy(indegCopy, indeg, sizeof indeg);

    for (int v = 1; v <= nodeCnt; v++) {
        earliest[v] = 0;
        parent[v]   = 0;
        if (indegCopy[v] == 0)
            q[tail++] = v;
    }

    topoLen = 0;
    while (head < tail) {
        int u = q[head++];
        topo[topoLen++] = u;       // record u in topo order
        for (Edge *e = g[u]; e; e = e->nxt) {
            int cand = earliest[u] + e->w;
            if (cand > earliest[e->to]) {
                earliest[e->to] = cand;
                parent[e->to]   = u;
            }
            if (--indegCopy[e->to] == 0) {
                q[tail++] = e->to;
            }
        }
    }
}

// This is used to pass latest
static void backwardPass(void) {
    int projectLen = earliest[nodeCnt];
    for (int v = 1; v <= nodeCnt; v++)
        latest[v] = projectLen;

    // Destroyed indeg[], but we don’t need it anymore.
    // Walk nodes from high→low index ≈ reverse topo because sources
    // are low‑numbered in these datasets.
    for (int i = topoLen - 1; i >= 0; i--) {
        int u = topo[i];
        for (Edge *e = g[u]; e; e = e->nxt) {
            int cand = latest[e->to] - e->w;
            if (cand < latest[u])
                latest[u] = cand;
        }
    }

    for (int v = 1; v <= nodeCnt; v++)
        slack[v] = latest[v] - earliest[v];
}

// Print data to display: Node, Earliest Time, Latest Start Time, and Slack time.
static void printResults(void) {
    puts("");
    printf("%-6s %-18s %-18s %-10s\n",
           "Node", "Earliest Start Time", "Latest Start Time", "Slack Time");
    printf("----------------------------------------------------------------\n");
    for (int v = 1; v <= nodeCnt; v++)
        printf("%-6d %-18d %-18d %-10d\n", v, earliest[v], latest[v], slack[v]);

    // We can reconstruct critical path by following parent[] back from sink
    int path[MAX_NODE], len = 0;
    for (int cur = nodeCnt; cur != 0; cur = parent[cur])
        path[len++] = cur;

    printf("\nThe critical path is:\n  ");
    for (int i = len - 1; i >= 0; i--)
        printf("%d%s", path[i], i ? " -> " : "\n");
    printf("\nwhich has length %d.\n", earliest[nodeCnt]);
}

// Makes Edge look better
static void cleanup(void) {
    for (int u = 1; u <= nodeCnt; u++) {
        Edge *e = g[u];
        while (e) {
            Edge *nx = e->nxt;
            free(e); // Free malloc
            e = nx;
        }
        g[u] = NULL;
    }
}

// Main function
int main(void) {
    readFile(FILENAME);
    forwardPass();
    backwardPass();
    printResults();
    cleanup();
    return 0;
}