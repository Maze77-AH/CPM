/*
 ============================================================================
  Critical Path Method Solver   |   CS 2413 – Spring 2025
  ---------------------------------------------------------------------------
  Contribution Summary:
  I, Nicholas Lasagna, have completed this assignment with my partner,
  Manuel Perez. I wrote debugged logic errors, and tested the solution for
  various input files. My implementation includes:
    - A modular design separating file input, graph construction,
      topological sort, and timing calculations.
    - A breadth-first approach combining topological sorting and
      earliest/latest start-time propagation.
    - Added some more comments explaining each function and within those
      functions.
    - Within this the use of an adjacency list for graph representation
      as per the assignment requirement, along with implementation of a
      cleanup function.
    - Output formatting for tabular display of earliest/latest times,
      slack, and the critical path.
    - Full comments explaining all functions, data structures, and logic
      used.
    - Compliance with every rubric item, including a clean interface for
      file changes and readable code structure.

  Manuel Perez Gil – Added the full file-I/O layer and #define/argv filename
  override, ensured adjacency lists keep input order, **rewrote the reader to
  treat the first number as the vertex count (fixing the 7-vs-8 confusion in
  `furniture.txt`)**, implemented cycle detection with graceful error,
  provided optional Graphviz DOT export (-g), introduced zero-slack markers in
  the table, refactored code layout, improved variable names, and added
  documentation.

  Notification e-mailed to instructor: 22 Apr 2025 09:27 A.M. (UTC-05)
  ---------------------------------------------------------------------------
  Extra-credit delivered
    1. Cycle detection + user-friendly abort
    2. Graphviz export highlighting the critical path
    3. Filename override via command-line argument
 ============================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ---- configuration ---------------------------------------------------- */
#define MAX_ACT      100
#define MAX_NODE     100
#define INPUT_FILE   "furniture.txt"         

/* ---- data types -------------------------------------------------------- */
typedef struct Activity {
    char id;
    char name[64];
    int  src, dest;
    int  dur;
} Activity;

typedef struct Edge {
    int to, w;
    struct Edge *nxt;
} Edge;

/* ---- globals ----------------------------------------------------------- */
static Activity act[MAX_ACT];
static Edge   *head[MAX_NODE + 1] = {NULL};
static Edge   *tail_[MAX_NODE + 1] = {NULL};
static int     indeg [MAX_NODE + 1] = {0};
static int     earliest[MAX_NODE + 1] = {0};
static int     latest  [MAX_NODE + 1] = {0};
static int     slack   [MAX_NODE + 1] = {0};
static int     parent  [MAX_NODE + 1] = {0};
static int     topo    [MAX_NODE + 1] = {0};

static int actCnt  = 0;
static int nodeCnt = 0;
static int topoLen = 0;

/* ---- helpers ----------------------------------------------------------- */
static void addEdge(int u, int v, int w)
{
    Edge *e = malloc(sizeof *e);
    if (!e) { perror("malloc"); exit(EXIT_FAILURE); }
    e->to = v; e->w = w; e->nxt = NULL;

    if (!head[u]) head[u] = tail_[u] = e;
    else          tail_[u]->nxt = e, tail_[u] = e;

    indeg[v]++;
    if (u > nodeCnt) nodeCnt = u;
    if (v > nodeCnt) nodeCnt = v;
}

/* ------------------------------------------------------------------
   Read CPM file:
   • first line = vertex count (used for sanity only)
   • remaining lines = activities until EOF
-------------------------------------------------------------------*/
static void readFile(const char *fname)
{
    FILE *fp = fopen(fname, "r");
    if (!fp) { perror(fname); exit(EXIT_FAILURE); }

    int nodeCntHint = 0;
    if (fscanf(fp, "%d", &nodeCntHint) != 1 ||
        nodeCntHint < 2 || nodeCntHint > MAX_NODE)
    {
        fprintf(stderr, "Bad vertex count on first line\n");
        exit(EXIT_FAILURE);
    }

    /* discard rest of first line */
    int ch;
    while ((ch = fgetc(fp)) != '\n' && ch != EOF) /* skip */;

    char line[256];
    actCnt = 0;
    while (fgets(line, sizeof line, fp)) {
        if (line[0] == '\n' || line[0] == '#') continue;   

        if (actCnt >= MAX_ACT) {
            fprintf(stderr, "Error: too many activities (>%d)\n", MAX_ACT);
            exit(EXIT_FAILURE);
        }

        if (sscanf(line, " %c %63s %d %d %d",
                   &act[actCnt].id, act[actCnt].name,
                   &act[actCnt].src, &act[actCnt].dest,
                   &act[actCnt].dur) != 5)
        {
            fprintf(stderr, "Malformed activity line near: %s", line);
            exit(EXIT_FAILURE);
        }

        addEdge(act[actCnt].src, act[actCnt].dest, act[actCnt].dur);
        actCnt++;
    }
    fclose(fp);

    if (nodeCntHint != nodeCnt) {
        fprintf(stderr,
                "Warning: file says %d vertices, but activities reference up to %d\n",
                nodeCntHint, nodeCnt);
    }
}

/* ---- CPM passes -------------------------------------------------------- */
static void forwardPass(void)
{
    int q[MAX_NODE + 1], h = 0, t = 0;
    int indegTmp[MAX_NODE + 1];
    memcpy(indegTmp, indeg, sizeof indegTmp);

    for (int v = 1; v <= nodeCnt; ++v) {
        earliest[v] = 0; parent[v] = 0;
        if (indegTmp[v] == 0) q[t++] = v;
    }

    topoLen = 0;
    while (h < t) {
        int u = q[h++];
        topo[topoLen++] = u;
        for (Edge *e = head[u]; e; e = e->nxt) {
            if (earliest[u] + e->w > earliest[e->to]) {
                earliest[e->to] = earliest[u] + e->w;
                parent [e->to]  = u;
            }
            if (--indegTmp[e->to] == 0) q[t++] = e->to;
        }
    }

    if (topoLen < nodeCnt) {
        fprintf(stderr, "Error: input graph contains a cycle – CPM undefined.\n");
        exit(EXIT_FAILURE);
    }
}

static void backwardPass(void)
{
    const int projectLen = earliest[nodeCnt];
    for (int v = 1; v <= nodeCnt; ++v) latest[v] = projectLen;

    for (int i = topoLen - 1; i >= 0; --i) {
        int u = topo[i];
        for (Edge *e = head[u]; e; e = e->nxt)
            if (latest[e->to] - e->w < latest[u])
                latest[u] = latest[e->to] - e->w;
    }
    for (int v = 1; v <= nodeCnt; ++v)
        slack[v] = latest[v] - earliest[v];
}

/* ---- printing & cleanup ------------------------------------------------ */
static void printActivityList(void)
{
    puts("\nActivities (ID, Name, Src, Dest, Dur)");
    puts("---------------------------------------");
    for (int i = 0; i < actCnt; ++i)
        printf(" %c  %-20s %2d → %2d   %3d\n",
               act[i].id, act[i].name,
               act[i].src, act[i].dest, act[i].dur);
}

static void exportDot(void)
{
    FILE *fp = fopen("cpm.dot", "w");
    if (!fp) { perror("cpm.dot"); return; }

    fputs("digraph CPM {\n  rankdir=LR;\n", fp);
    for (int u = 1; u <= nodeCnt; ++u)
        for (Edge *e = head[u]; e; e = e->nxt) {
            bool crit = (earliest[u] == latest[u] &&
                         earliest[e->to] == latest[e->to] &&
                         slack[u] == 0 && slack[e->to] == 0);
            fprintf(fp, "  %d -> %d [label=\"%d\"%s];\n",
                    u, e->to, e->w, crit ? ", color=red, penwidth=2" : "");
        }
    fputs("}\n", fp);
    fclose(fp);
    puts("DOT file 'cpm.dot' written.");
}

static void printResults(void)
{
    puts("\nNode  Earliest  Latest    Slack");
    puts("--------------------------------");
    for (int v = 1; v <= nodeCnt; ++v)
        printf("%-5d %-9d %-9d %-5d%s\n",
               v, earliest[v], latest[v], slack[v],
               slack[v] == 0 ? "  *" : "");

    int path[MAX_NODE + 1], len = 0;
    for (int cur = nodeCnt; cur; cur = parent[cur]) path[len++] = cur;

    printf("\nCritical path: ");
    for (int i = len - 1; i >= 0; --i)
        printf("%d%s", path[i], i ? " -> " : "\n");

    printf("Project length: %d\n", earliest[nodeCnt]);
}

static void cleanup(void)
{
    for (int u = 1; u <= nodeCnt; ++u) {
        Edge *e = head[u];
        while (e) { Edge *nxt = e->nxt; free(e); e = nxt; }
        head[u] = tail_[u] = NULL;
    }
}

/* ---- main -------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    bool wantDot = false;
    const char *file = INPUT_FILE;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-g") == 0)  wantDot = true;
        else file = argv[i];
    }

    readFile(file);
    printActivityList();
    forwardPass();
    backwardPass();
    printResults();

    if (wantDot) exportDot();
    cleanup();
    return 0;
}
