/* Antarmuka terminal Sistem Akademik - pengganti server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "akademik.h"

/* ================= Input ================= */
static void baca_baris(const char *label, char *out, size_t sz) {
    char buf[256];
    printf("%s: ", label);
    fflush(stdout);
    if (!fgets(buf, sizeof buf, stdin)) {                /* EOF (Ctrl+D / Ctrl+Z) */
        printf("\nKeluar.\n");
        exit(0);
    }
    if (!strchr(buf, '\n')) {                            /* buang sisa baris yang terlalu panjang */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
    buf[strcspn(buf, "\r\n")] = 0;
    for (char *p = buf; *p; p++)                         /* koma merusak format CSV */
        if (*p == ',' || *p == ';' || *p == '\t') *p = ' ';
    char *awal = buf + strspn(buf, " ");
    size_t n = strlen(awal);
    while (n && awal[n - 1] == ' ') awal[--n] = 0;
    if (n >= sz) printf("  (dipotong menjadi %d karakter)\n", (int)sz - 1);
    snprintf(out, sz, "%s", awal);
}

/* Teks wajib: tidak boleh kosong */
static int baca_wajib(const char *label, char *out, size_t sz) {
    baca_baris(label, out, sz);
    if (out[0]) return 1;
    printf("Gagal: %s wajib diisi.\n", label);
    return 0;
}

static int baca_angka(const char *label, double min, double max, double *v) {
    char buf[32], *end;
    baca_baris(label, buf, sizeof buf);
    *v = strtod(buf, &end);
    if (buf[0] && *end == 0 && *v >= min && *v <= max) return 1;   /* nan ikut ditolak */
    printf("Gagal: masukkan angka %g-%g.\n", min, max);
    return 0;
}

static int baca_mahasiswa(void) {                        /* indeks mahasiswa atau -1 */
    char nim[16];
    if (!baca_wajib("NIM", nim, sizeof nim)) return -1;
    int i = mhs_cari(nim, NULL);                         /* binary search */
    if (i < 0) printf("Mahasiswa tidak ditemukan.\n");
    return i;
}

/* ================= Mahasiswa ================= */
static void judul_mhs(void) { printf("\n%-15s %-30s %s\n", "NIM", "Nama", "Prodi"); }
static void baris_mhs(const Mahasiswa *m) { printf("%-15s %-30s %s\n", m->nim, m->nama, m->prodi); }

static void dashboard(void) {
    printf("\nMahasiswa: %d | Mata kuliah: %d | Nilai: %d | Prasyarat: %d\n",
           g_mhs_n, g_mk_n, g_nilai_n, g_edge_n);
    printf("\n5 mahasiswa terbaru:");
    judul_mhs();
    int batas = 0, ada_batas = 0;
    for (int k = 0; k < 5; k++) {
        int best = -1;
        for (int i = 0; i < g_mhs_n; i++)
            if ((!ada_batas || g_mhs[i].urut < batas) && (best < 0 || g_mhs[i].urut > g_mhs[best].urut)) best = i;
        if (best < 0) break;
        baris_mhs(&g_mhs[best]);
        batas = g_mhs[best].urut;
        ada_batas = 1;
    }
}

static void daftar_mhs(void) {
    if (!g_mhs_n) { printf("Belum ada data.\n"); return; }
    judul_mhs();
    for (int i = 0; i < g_mhs_n; i++) baris_mhs(&g_mhs[i]);
}

static void cari_mhs(void) {
    int i = baca_mahasiswa();
    if (i < 0) return;
    judul_mhs();
    baris_mhs(&g_mhs[i]);
}

static void tambah_mhs(void) {
    char nim[16], nama[64], prodi[48];
    if (!baca_wajib("NIM", nim, sizeof nim)) return;
    if (mhs_cari(nim, NULL) >= 0) { printf("Gagal: NIM sudah terdaftar.\n"); return; }
    if (!baca_wajib("Nama", nama, sizeof nama)) return;
    baca_baris("Prodi (boleh kosong)", prodi, sizeof prodi);
    if (!prodi[0]) strcpy(prodi, "-");
    if (mhs_tambah(nim, nama, prodi) != 0) { printf("Gagal: memori habis.\n"); return; }
    persist_save();
    printf("Mahasiswa berhasil ditambahkan.\n");
}

/* ================= Nilai ================= */
static void cetak_nilai(const Nilai *a, int n) {
    if (!n) { printf("Belum ada data.\n"); return; }
    printf("\n%-15s %-20s %-7s %-30s %6s %s\n", "NIM", "Nama", "Kode", "Mata Kuliah", "Nilai", "Grade");
    for (int i = 0; i < n; i++) {
        int m = mhs_cari(a[i].nim, NULL);
        printf("%-15s %-20.20s %-7s %-30.30s %6g %s\n", a[i].nim, m >= 0 ? g_mhs[m].nama : "-",
               g_mk[a[i].mk].kode, g_mk[a[i].mk].nama, a[i].nilai, grade_huruf(a[i].nilai));
    }
}

static void urut_nilai(void) {
    char pilih[8];
    baca_baris("Urutan: 1=tertinggi, 2=terendah [1]", pilih, sizeof pilih);
    if (!g_nilai_n) { printf("Belum ada data.\n"); return; }
    Nilai *salinan = malloc(g_nilai_n * sizeof(Nilai));
    if (!salinan) { printf("Gagal: memori habis.\n"); return; }
    memcpy(salinan, g_nilai, g_nilai_n * sizeof(Nilai));
    nilai_urut(salinan, g_nilai_n, strcmp(pilih, "2") != 0);   /* insertion sort pada salinan */
    cetak_nilai(salinan, g_nilai_n);
    free(salinan);
}

static void tulis_nilai(int ubah) {
    int m = baca_mahasiswa();
    if (m < 0) return;
    char kode[12];
    if (!baca_wajib("Kode mata kuliah", kode, sizeof kode)) return;
    int mk = mk_cari(kode);
    if (mk < 0) { printf("Mata kuliah tidak ditemukan.\n"); return; }
    int ada = nilai_cari(g_mhs[m].nim, mk) >= 0;
    if (ubah && !ada) { printf("Nilai belum ada, gunakan menu tambah nilai.\n"); return; }
    if (!ubah && ada) { printf("Nilai sudah ada, gunakan menu ubah nilai.\n"); return; }
    double v;
    if (!baca_angka("Nilai (0-100)", 0, 100, &v)) return;
    int r = ubah ? nilai_ubah(g_mhs[m].nim, mk, v) : nilai_tambah(g_mhs[m].nim, mk, v);
    if (r != 0) { printf("Gagal menyimpan nilai.\n"); return; }
    persist_save();
    printf("Nilai berhasil %s (grade %s).\n", ubah ? "diubah" : "ditambahkan", grade_huruf(v));
}

/* ================= Mata kuliah & KRS ================= */
static void daftar_mk(void) {
    printf("\n%-7s %-30s %3s  %s\n", "Kode", "Nama", "SKS", "Prasyarat");
    for (int i = 0; i < g_mk_n; i++) {
        printf("%-7s %-30.30s %3d  ", g_mk[i].kode, g_mk[i].nama, g_mk[i].sks);
        int n = 0;
        for (int e = 0; e < g_edge_n; e++)
            if (g_edge[e].mk == i) printf("%s%s", n++ ? " " : "", g_mk[g_edge[e].syarat].kode);
        printf("%s\n", n ? "" : "-");
    }
}

static void tambah_mk(void) {
    char kode[12], nama[48], teks[200];
    double sks;
    if (!baca_wajib("Kode", kode, sizeof kode)) return;
    if (strchr(kode, ' ')) { printf("Gagal: kode tidak boleh mengandung spasi.\n"); return; }
    if (mk_cari(kode) >= 0) { printf("Gagal: kode sudah ada.\n"); return; }
    if (!baca_wajib("Nama", nama, sizeof nama)) return;
    if (!baca_angka("SKS (1-10)", 1, 10, &sks)) return;
    if (sks != (int)sks) { printf("Gagal: SKS harus bilangan bulat.\n"); return; }

    baca_baris("Kode prasyarat, pisahkan dengan spasi (boleh kosong)", teks, sizeof teks);
    int syarat[16], ns = 0;
    for (char *p = strtok(teks, " "); p; p = strtok(NULL, " ")) {
        int idx = mk_cari(p);
        if (idx < 0) { printf("Gagal: kode prasyarat %s tidak dikenal.\n", p); return; }
        int dobel = 0;
        for (int i = 0; i < ns; i++) if (syarat[i] == idx) dobel = 1;
        if (dobel) continue;
        if (ns == 16) { printf("Gagal: maksimal 16 prasyarat.\n"); return; }
        syarat[ns++] = idx;
    }
    int baru = mk_tambah(kode, nama, (int)sks);
    if (baru < 0) { printf("Gagal: memori habis.\n"); return; }
    for (int i = 0; i < ns; i++) graph_tambah_edge(syarat[i], baru);
    persist_save();
    printf("Mata kuliah berhasil ditambahkan.\n");
}

static void graph_prasyarat(void) {
    printf("\nGraph prasyarat (prasyarat -> mata kuliah):\n");
    if (!g_edge_n) printf("  (belum ada)\n");
    for (int e = 0; e < g_edge_n; e++) {
        const MataKuliah *a = &g_mk[g_edge[e].syarat], *b = &g_mk[g_edge[e].mk];
        printf("  %s %s -> %s %s\n", a->kode, a->nama, b->kode, b->nama);
    }
}

static void krs(void) {
    int m = baca_mahasiswa();
    if (m < 0) return;
    int *out = malloc((g_mk_n ? g_mk_n : 1) * sizeof(int));
    if (!out) { printf("Gagal: memori habis.\n"); return; }
    int n = krs_rekomendasi(g_mhs[m].nim, out), total = 0;   /* BFS + queue */
    printf("\nRekomendasi KRS untuk %s (%s):\n", g_mhs[m].nama, g_mhs[m].nim);
    if (!n) printf("Tidak ada mata kuliah yang dapat direkomendasikan.\n");
    for (int i = 0; i < n; i++) {
        printf("  %d. %-7s %-30s %d SKS\n", i + 1, g_mk[out[i]].kode, g_mk[out[i]].nama, g_mk[out[i]].sks);
        total += g_mk[out[i]].sks;
    }
    if (n) printf("Total: %d SKS (maks %d)\n", total, MAX_SKS);
    free(out);
}

/* ================= Menu utama ================= */
int main(void) {
    persist_init();
    for (;;) {
        printf("\n========== SISTEM AKADEMIK ==========\n"
               " 1. Dashboard\n"
               " 2. Daftar mahasiswa\n"
               " 3. Cari mahasiswa\n"
               " 4. Tambah mahasiswa\n"
               " 5. Daftar nilai\n"
               " 6. Urutkan nilai\n"
               " 7. Tambah nilai\n"
               " 8. Ubah nilai\n"
               " 9. Daftar mata kuliah\n"
               "10. Tambah mata kuliah\n"
               "11. Graph prasyarat\n"
               "12. Rekomendasi KRS\n"
               " 0. Keluar\n");
        char pilih[8], *end;
        baca_baris("Pilih menu", pilih, sizeof pilih);
        long p = strtol(pilih, &end, 10);
        if (!pilih[0] || *end) p = -1;
        switch (p) {
            case 0:  printf("Keluar.\n"); return 0;
            case 1:  dashboard(); break;
            case 2:  daftar_mhs(); break;
            case 3:  cari_mhs(); break;
            case 4:  tambah_mhs(); break;
            case 5:  cetak_nilai(g_nilai, g_nilai_n); break;
            case 6:  urut_nilai(); break;
            case 7:  tulis_nilai(0); break;
            case 8:  tulis_nilai(1); break;
            case 9:  daftar_mk(); break;
            case 10: tambah_mk(); break;
            case 11: graph_prasyarat(); break;
            case 12: krs(); break;
            default: printf("Pilihan tidak valid.\n");
        }
    }
}
