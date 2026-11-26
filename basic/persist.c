#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define BUAT_FOLDER(p) _mkdir(p)
#else
#define BUAT_FOLDER(p) mkdir(p, 0755)
#endif
#include "akademik.h"

#define D "data/"

static void seed(void) {
    const char *mk[][3] = {
        {"IK101","Algoritma dan Pemrograman","3"}, {"IK102","Matematika Diskrit","3"},
        {"IK201","Struktur Data","3"}, {"IK202","Basis Data","3"}, {"IK203","Aljabar Linear","3"},
        {"IK301","Analisis Algoritma","3"}, {"IK302","Pemrograman Web","3"},
        {"IK303","Kecerdasan Buatan","3"}, {"IK401","Machine Learning","3"},
        {"IK402","Rekayasa Perangkat Lunak","3"}};
    for (int i = 0; i < 10; i++) mk_tambah(mk[i][0], mk[i][1], atoi(mk[i][2]));
    int e[][2] = {{0,2},{0,3},{1,4},{2,5},{1,5},{3,6},{2,7},{4,7},{7,8},{3,9},{6,9}};
    for (int i = 0; i < 11; i++) graph_tambah_edge(e[i][0], e[i][1]);

    mhs_tambah("2023001", "Raihan", "Ilmu Komputer");
    mhs_tambah("2023002", "Sinta", "Ilmu Komputer");
    mhs_tambah("2023003", "Budi", "Ilmu Komputer");
    struct { const char *nim; int mk; double v; } n[] = {
        {"2023001",0,88},{"2023001",1,78},{"2023001",2,82},{"2023001",3,70},
        {"2023002",0,92},{"2023002",1,55},{"2023002",3,85},{"2023003",0,65}};
    for (int i = 0; i < 8; i++) nilai_tambah(n[i].nim, n[i].mk, n[i].v);
    persist_save();
}

void persist_save(void) {
    BUAT_FOLDER("data");
    FILE *f;
    if ((f = fopen(D "mahasiswa.csv", "w"))) {
        for (int i = 0; i < g_mhs_n; i++)
            fprintf(f, "%s,%s,%s,%d\n", g_mhs[i].nim, g_mhs[i].nama, g_mhs[i].prodi, g_mhs[i].urut);
        fclose(f);
    }
    if ((f = fopen(D "matakuliah.csv", "w"))) {
        for (int i = 0; i < g_mk_n; i++) fprintf(f, "%s,%s,%d\n", g_mk[i].kode, g_mk[i].nama, g_mk[i].sks);
        fclose(f);
    }
    if ((f = fopen(D "prasyarat.csv", "w"))) {
        for (int i = 0; i < g_edge_n; i++)
            fprintf(f, "%s,%s\n", g_mk[g_edge[i].syarat].kode, g_mk[g_edge[i].mk].kode);
        fclose(f);
    }
    if ((f = fopen(D "nilai.csv", "w"))) {
        for (int i = 0; i < g_nilai_n; i++)
            fprintf(f, "%s,%s,%g\n", g_nilai[i].nim, g_mk[g_nilai[i].mk].kode, g_nilai[i].nilai);
        fclose(f);
    }
}

void persist_init(void) {
    FILE *f = fopen(D "matakuliah.csv", "r");
    if (!f) { seed(); return; }
    char line[256];
    while (fgets(line, sizeof line, f)) {
        char *k = strtok(line, ",\n"), *nm = strtok(NULL, ",\n"), *s = strtok(NULL, ",\n");
        if (k && nm && s) mk_tambah(k, nm, atoi(s));
    }
    fclose(f);
    if ((f = fopen(D "prasyarat.csv", "r"))) {
        while (fgets(line, sizeof line, f)) {
            char *a = strtok(line, ",\n"), *b = strtok(NULL, ",\n");
            int x = a ? mk_cari(a) : -1, y = b ? mk_cari(b) : -1;
            if (x >= 0 && y >= 0) graph_tambah_edge(x, y);
        }
        fclose(f);
    }
    if ((f = fopen(D "mahasiswa.csv", "r"))) {
        while (fgets(line, sizeof line, f)) {
            char *a = strtok(line, ",\n"), *b = strtok(NULL, ",\n"), *c = strtok(NULL, ",\n"), *u = strtok(NULL, ",\n");
            if (a && b && c) mhs_tambah_u(a, b, c, u ? atoi(u) : 0);
        }
        fclose(f);
    }
    if ((f = fopen(D "nilai.csv", "r"))) {
        while (fgets(line, sizeof line, f)) {
            char *a = strtok(line, ",\n"), *b = strtok(NULL, ",\n"), *v = strtok(NULL, ",\n");
            int m = b ? mk_cari(b) : -1;
            if (a && m >= 0 && v) nilai_tambah(a, m, atof(v));
        }
        fclose(f);
    }
}
