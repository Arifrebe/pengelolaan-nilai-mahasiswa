#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "akademik.h"

MataKuliah *g_mk = NULL; int g_mk_n = 0;
Edge *g_edge = NULL;     int g_edge_n = 0;
static int mk_cap = 0, edge_cap = 0;

/* ---------- Queue circular (Modul 6) ---------- */
typedef struct { int *d; int cap, head, tail, count; } Queue;
static void q_init(Queue *q, int cap) { q->d = malloc(cap * sizeof(int)); q->cap = cap; q->head = q->tail = q->count = 0; }
static void q_free(Queue *q) { free(q->d); }
static int  q_empty(const Queue *q) { return q->count == 0; }
static void q_push(Queue *q, int x) { if (q->count == q->cap) return; q->d[q->tail] = x; q->tail = (q->tail + 1) % q->cap; q->count++; }
static int  q_pop(Queue *q) { int x = q->d[q->head]; q->head = (q->head + 1) % q->cap; q->count--; return x; }

/* ---------- Node graph: mata kuliah (array dinamis) ---------- */
int mk_cari(const char *kode) {
    for (int i = 0; i < g_mk_n; i++)
        if (strcasecmp(g_mk[i].kode, kode) == 0) return i;
    return -1;
}

int mk_tambah(const char *kode, const char *nama, int sks) {
    if (mk_cari(kode) >= 0) return -1;
    if (g_mk_n == mk_cap) {
        int baru = mk_cap ? mk_cap * 2 : 16;
        MataKuliah *p = realloc(g_mk, baru * sizeof(MataKuliah));
        if (!p) return -2;
        g_mk = p; mk_cap = baru;
    }
    snprintf(g_mk[g_mk_n].kode, sizeof g_mk[0].kode, "%s", kode);
    snprintf(g_mk[g_mk_n].nama, sizeof g_mk[0].nama, "%s", nama);
    g_mk[g_mk_n].sks = sks;
    return g_mk_n++;
}

/* ---------- Edge graph: syarat -> mk ---------- */
void graph_tambah_edge(int syarat, int mk) {
    if (g_edge_n == edge_cap) {
        int baru = edge_cap ? edge_cap * 2 : 16;
        Edge *p = realloc(g_edge, baru * sizeof(Edge));
        if (!p) return;
        g_edge = p; edge_cap = baru;
    }
    g_edge[g_edge_n].syarat = syarat;
    g_edge[g_edge_n].mk = mk;
    g_edge_n++;
}

static int prasyarat_terpenuhi(const char *nim, int mk) {
    for (int i = 0; i < g_edge_n; i++)
        if (g_edge[i].mk == mk && !nilai_lulus(nim, g_edge[i].syarat)) return 0;
    return 1;
}

/* BFS level demi level mulai dari mata kuliah tanpa prasyarat.
 * Setiap MK yang belum lulus & prasyaratnya terpenuhi masuk KRS (maks MAX_SKS). */
int krs_rekomendasi(const char *nim, int *out) {
    int n = g_mk_n, cnt = 0, total = 0;
    if (n == 0) return 0;
    int *indeg = calloc(n, sizeof(int)), *visited = calloc(n, sizeof(int));
    Queue q; q_init(&q, n);
    for (int i = 0; i < g_edge_n; i++) indeg[g_edge[i].mk]++;
    for (int i = 0; i < n; i++) if (indeg[i] == 0) { visited[i] = 1; q_push(&q, i); }

    while (!q_empty(&q)) {
        int u = q_pop(&q);
        if (!nilai_lulus(nim, u) && prasyarat_terpenuhi(nim, u) && total + g_mk[u].sks <= MAX_SKS) {
            out[cnt++] = u;
            total += g_mk[u].sks;
        }
        for (int i = 0; i < g_edge_n; i++)
            if (g_edge[i].syarat == u && !visited[g_edge[i].mk]) {
                visited[g_edge[i].mk] = 1;
                q_push(&q, g_edge[i].mk);
            }
    }
    q_free(&q); free(indeg); free(visited);
    return cnt;
}
