#ifndef AKADEMIK_H
#define AKADEMIK_H

#define NILAI_LULUS 60.0   /* nilai minimal lulus (C) */
#define MAX_SKS     24     /* batas SKS per KRS */

/* ---------- Tipe data ---------- */
typedef struct { char nim[16]; char nama[64]; char prodi[48]; int urut; } Mahasiswa;
typedef struct { char kode[12]; char nama[48]; int sks; } MataKuliah;
typedef struct { int syarat; int mk; } Edge;          /* syarat -> mk (syarat adalah prasyarat mk) */
typedef struct { char nim[16]; int mk; double nilai; } Nilai;

/* ---------- Modul 2: array dinamis + Modul 3: searching (mahasiswa.c) ---------- */
extern Mahasiswa *g_mhs; extern int g_mhs_n;
int  mhs_cari(const char *nim, int *posisi);          /* binary search; -1 jika tidak ada */
int  mhs_tambah(const char *nim, const char *nama, const char *prodi);
int  mhs_tambah_u(const char *nim, const char *nama, const char *prodi, int urut);

/* ---------- Nilai: array dinamis + sorting (nilai.c) ---------- */
extern Nilai *g_nilai; extern int g_nilai_n;
int  nilai_cari(const char *nim, int mk);
int  nilai_tambah(const char *nim, int mk, double v); /* -1 jika sudah ada */
int  nilai_ubah(const char *nim, int mk, double v);   /* -1 jika belum ada */
int  nilai_lulus(const char *nim, int mk);
const char *grade_huruf(double v);
void nilai_urut(Nilai *a, int n, int desc);           /* insertion sort */

/* ---------- Modul 8 Graph + Modul 6 Queue (graph.c) ---------- */
extern MataKuliah *g_mk; extern int g_mk_n;
extern Edge *g_edge; extern int g_edge_n;
int  mk_cari(const char *kode);
int  mk_tambah(const char *kode, const char *nama, int sks);
void graph_tambah_edge(int syarat, int mk);
int  krs_rekomendasi(const char *nim, int *out);      /* BFS + queue; isi out[] dgn indeks MK */

/* ---------- Penyimpanan CSV (persist.c) ---------- */
void persist_init(void);                              /* muat CSV, atau isi data contoh */
void persist_save(void);

#endif
