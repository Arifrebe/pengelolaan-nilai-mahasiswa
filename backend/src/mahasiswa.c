#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "akademik.h"

Mahasiswa *g_mhs = NULL;
int g_mhs_n = 0;
static int cap = 0, max_urut = 0;

/* Binary search pada array yang selalu terurut menurut NIM */
int mhs_cari(const char *nim, int *posisi) {
    int lo = 0, hi = g_mhs_n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2, c = strcmp(g_mhs[mid].nim, nim);
        if (c == 0) { if (posisi) *posisi = mid; return mid; }
        if (c < 0) lo = mid + 1; else hi = mid - 1;
    }
    if (posisi) *posisi = lo;
    return -1;
}

int mhs_tambah_u(const char *nim, const char *nama, const char *prodi, int urut) {
    int pos;
    if (mhs_cari(nim, &pos) >= 0) return -1;
    if (g_mhs_n == cap) {                              /* larik dinamis: kapasitas x2 */
        int baru = cap ? cap * 2 : 8;
        Mahasiswa *p = realloc(g_mhs, baru * sizeof(Mahasiswa));
        if (!p) return -2;
        g_mhs = p; cap = baru;
    }
    memmove(&g_mhs[pos + 1], &g_mhs[pos], (g_mhs_n - pos) * sizeof(Mahasiswa));
    Mahasiswa *m = &g_mhs[pos];
    snprintf(m->nim, sizeof m->nim, "%s", nim);
    snprintf(m->nama, sizeof m->nama, "%s", nama);
    snprintf(m->prodi, sizeof m->prodi, "%s", prodi);
    m->urut = urut;
    if (urut > max_urut) max_urut = urut;
    g_mhs_n++;
    return 0;
}

int mhs_tambah(const char *nim, const char *nama, const char *prodi) {
    return mhs_tambah_u(nim, nama, prodi, max_urut + 1);
}
