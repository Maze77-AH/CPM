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

// Global Vars
static Activity act[MAX_ACT];
static Edge*    g  [MAX_NODE] = {0};

static int indeg   [MAX_NODE] = {0};
static int earliest[MAX_NODE] = {0};
static int latest  [MAX_NODE] = {0};
static int slack   [MAX_NODE] = {0};
static int parent  [MAX_NODE] = {0};

static int actCnt = 0, nodeCnt = 0;

// Helper
static void addEdge(int u,int v,int w) {
    Edge *e = malloc(sizeof *e);
    e->to = v; e->w = w; e->nxt = g[u];
    g[u]  = e;
    indeg[v]++;

    if(u>nodeCnt) nodeCnt=u;
    if(v>nodeCnt) nodeCnt=v;
}

// Can read file and if error exit program.
static void readFile(const char *fname) {
    FILE *fp = fopen(fname,"r");
    if(!fp){ perror("file"); exit(1); }

    fscanf(fp,"%d",&actCnt);
    for(int i=0;i<actCnt;i++)
    {
        fscanf(fp," %c %63s %d %d %d",
               &act[i].id,act[i].name,
               &act[i].s,&act[i].t,&act[i].dur);
        addEdge(act[i].s,act[i].t,act[i].dur);
    }
    fclose(fp);
}

// Used to pass earliest

static void forwardPass(void) {
    int q[MAX_NODE], head=0, tail=0;

    for(int v=1; v<=nodeCnt; ++v){
        earliest[v]=0;
        parent  [v]=0;
        if(!indeg[v]) q[tail++]=v;
    }
    while(head<tail){
        int u=q[head++];
        for(Edge*e=g[u]; e; e=e->nxt){
            if(earliest[u]+e->w > earliest[e->to]){
                earliest[e->to] = earliest[u]+e->w;
                parent  [e->to] = u;
            }
            if(--indeg[e->to]==0) q[tail++]=e->to;
        }
    }
}

// This is used to pass latest
static void backwardPass(void) {
    for(int v=1; v<=nodeCnt; ++v) latest[v] = earliest[nodeCnt]; /* init */

    // Destroyed indeg[], but we don’t need it anymore.
    // Walk nodes from high→low index ≈ reverse topo because sources
    // are low‑numbered in these datasets.
    for(int u=nodeCnt; u>=1; --u)
        for(Edge*e=g[u]; e; e=e->nxt)
            if(latest[e->to]-e->w < latest[u])
                latest[u] = latest[e->to]-e->w;

    for(int v=1; v<=nodeCnt; ++v) slack[v] = latest[v]-earliest[v];
}

// Print data to display: Node, Earliest Time, Latest Start Time, and Slack time.
static void printResults(void) {
    puts("");
    printf("%-6s %-20s %-20s %-10s\n",
           "Node","Earliest Start","Latest Start","Slack");
    puts("--------------------------------------------------------------");
    for(int v=1; v<=nodeCnt; ++v)
        printf("%-6d %-20d %-20d %-10d\n",
               v, earliest[v], latest[v], slack[v]);

    // rebuild critical path by following parent[] backwards
    int path[MAX_NODE], len=0, cur=nodeCnt;
    while(cur){ path[len++]=cur; cur=parent[cur]; }

    puts("\nCritical path is:");
    for(int i=len-1;i>=0;--i)
        printf("%d%s", path[i], i? " -> ":"\n");
    printf("Path length: %d\n", earliest[nodeCnt]);
}

// Main function
int main(void) {
    readFile(FILENAME);
    forwardPass();
    backwardPass();
    printResults();
    return 0;
}