#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "akademik.h"

Nilai *g_nilai = NULL;
int g_nilai_n = 0;
static int cap = 0;

int nilai_cari(const char *nim, int mk) {                /* sequential search */
    for (int i = 0; i < g_nilai_n; i++)
        if (g_nilai[i].mk == mk && strcmp(g_nilai[i].nim, nim) == 0) return i;
    return -1;
}

int nilai_tambah(const char *nim, int mk, double v) {
    if (nilai_cari(nim, mk) >= 0) return -1;
    if (g_nilai_n == cap) {
        int baru = cap ? cap * 2 : 16;
        Nilai *p = realloc(g_nilai, baru * sizeof(Nilai));
        if (!p) return -2;
        g_nilai = p; cap = baru;
    }
    snprintf(g_nilai[g_nilai_n].nim, sizeof g_nilai[0].nim, "%s", nim);
    g_nilai[g_nilai_n].mk = mk;
    g_nilai[g_nilai_n].nilai = v;
    g_nilai_n++;
    return 0;
}

int nilai_ubah(const char *nim, int mk, double v) {
    int i = nilai_cari(nim, mk);
    if (i < 0) return -1;
    g_nilai[i].nilai = v;
    return 0;
}

int nilai_lulus(const char *nim, int mk) {
    int i = nilai_cari(nim, mk);
    return i >= 0 && g_nilai[i].nilai >= NILAI_LULUS;
}

const char *grade_huruf(double v) {
    if (v >= 85) return "A";
    if (v >= 75) return "B";
    if (v >= 60) return "C";
    if (v >= 50) return "D";
    return "E";
}

/* Insertion sort (stabil). desc=1: tertinggi -> terendah */
void nilai_urut(Nilai *a, int n, int desc) {
    for (int i = 1; i < n; i++) {
        Nilai key = a[i];
        int j = i - 1;
        while (j >= 0 && (desc ? a[j].nilai < key.nilai : a[j].nilai > key.nilai)) {
            a[j + 1] = a[j]; j--;
        }
        a[j + 1] = key;
    }
}
