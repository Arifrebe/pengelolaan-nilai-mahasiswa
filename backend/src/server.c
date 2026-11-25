/* Server HTTP (Mongoose) + endpoint JSON - membungkus struktur data C */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include "mongoose.h"
#include "akademik.h"

#define CORS "Access-Control-Allow-Origin: *\r\n" \
             "Access-Control-Allow-Methods: GET, POST, PUT, OPTIONS\r\n" \
             "Access-Control-Allow-Headers: Content-Type\r\n"
#define HDR  "Content-Type: application/json\r\n" CORS

static char g_frontend[PATH_MAX] = "../frontend";

/* ================= String builder JSON ================= */
typedef struct { char *b; size_t n, cap; } Sb;

static void sb_fmt(Sb *s, const char *fmt, ...) {
    va_list ap;
    for (;;) {
        va_start(ap, fmt);
        int w = vsnprintf(s->b + s->n, s->cap - s->n, fmt, ap);
        va_end(ap);
        if (w >= 0 && (size_t)w < s->cap - s->n) { s->n += (size_t)w; return; }
        s->cap = s->cap ? s->cap * 2 : 1024;
        s->b = realloc(s->b, s->cap);
    }
}

static void sb_str(Sb *s, const char *v) {               /* string JSON ter-escape */
    sb_fmt(s, "\"");
    for (; *v; v++) {
        unsigned char ch = (unsigned char)*v;
        if (ch == '"' || ch == '\\') sb_fmt(s, "\\%c", ch);
        else if (ch < 0x20) sb_fmt(s, "\\u%04x", ch);
        else sb_fmt(s, "%c", ch);
    }
    sb_fmt(s, "\"");
}

static void sb_mhs(Sb *s, const Mahasiswa *m) {
    sb_fmt(s, "{\"nim\":"); sb_str(s, m->nim);
    sb_fmt(s, ",\"nama\":"); sb_str(s, m->nama);
    sb_fmt(s, ",\"prodi\":"); sb_str(s, m->prodi);
    sb_fmt(s, ",\"status\":\"Aktif\"}");
}

static void sb_nilai(Sb *s, const Nilai *n) {
    int im = mhs_cari(n->nim, NULL);
    sb_fmt(s, "{\"nim\":"); sb_str(s, n->nim);
    sb_fmt(s, ",\"nama\":"); sb_str(s, im >= 0 ? g_mhs[im].nama : "-");
    sb_fmt(s, ",\"kode_matkul\":"); sb_str(s, g_mk[n->mk].kode);
    sb_fmt(s, ",\"matakuliah\":"); sb_str(s, g_mk[n->mk].nama);
    sb_fmt(s, ",\"nilai\":%g,\"grade\":\"%s\"}", n->nilai, grade_huruf(n->nilai));
}

static void sb_mk_ringkas(Sb *s, int i) {
    sb_fmt(s, "{\"kode\":"); sb_str(s, g_mk[i].kode);
    sb_fmt(s, ",\"nama\":"); sb_str(s, g_mk[i].nama);
    sb_fmt(s, ",\"sks\":%d}", g_mk[i].sks);
}

/* ================= Balasan standar ================= */
static void reply(struct mg_connection *c, int status, int ok, const char *data, const char *msg) {
    Sb s = {0};
    sb_fmt(&s, "{\"success\":%s,\"data\":%s,\"message\":", ok ? "true" : "false", data ? data : "null");
    sb_str(&s, msg);
    sb_fmt(&s, "}");
    mg_http_reply(c, status, HDR, "%s", s.b);
    free(s.b);
}
static void fail(struct mg_connection *c, int status, const char *msg) { reply(c, status, 0, NULL, msg); }

/* ================= Parsing input ================= */
static void bersihkan(char *s) {                         /* hindari karakter pemisah CSV, trim */
    for (char *p = s; *p; p++) if (*p == ',' || *p == ';' || *p == '\n' || *p == '\r' || *p == '\t') *p = ' ';
    size_t n = strlen(s);
    while (n && s[n - 1] == ' ') s[--n] = 0;
    size_t lead = strspn(s, " ");
    if (lead) memmove(s, s + lead, strlen(s + lead) + 1);
}

static int json_str(struct mg_str body, const char *path, char *out, size_t sz) {
    out[0] = 0;
    struct mg_str t = mg_json_get_tok(body, path);
    if (t.buf == NULL || t.len == 0) return 0;
    if (t.buf[0] == '"') {
        char *v = mg_json_get_str(body, path);
        if (!v) return 0;
        snprintf(out, sz, "%s", v);
        free(v);
    } else if (t.len == 4 && memcmp(t.buf, "null", 4) == 0) {
        return 0;
    } else {
        snprintf(out, sz, "%.*s", (int)t.len, t.buf);
    }
    bersihkan(out);
    return out[0] != 0;
}

static int json_num(struct mg_str body, const char *path, double *v) {
    char buf[32], *end;
    if (!json_str(body, path, buf, sizeof buf)) return 0;
    *v = strtod(buf, &end);
    return *end == 0;
}

static int query_var(struct mg_http_message *hm, const char *name, char *out, size_t sz) {
    int n = mg_http_get_var(&hm->query, name, out, sz);
    if (n <= 0) { out[0] = 0; return 0; }
    bersihkan(out);
    return out[0] != 0;
}

/* ================= Handler endpoint ================= */
static void h_dashboard(struct mg_connection *c) {
    Sb s = {0};
    sb_fmt(&s, "{\"total_mahasiswa\":%d,\"total_matkul\":%d,\"total_nilai\":%d,\"total_prasyarat\":%d,\"mahasiswa_terbaru\":[",
           g_mhs_n, g_mk_n, g_nilai_n, g_edge_n);
    int prev = INT_MAX;
    for (int k = 0; k < 5; k++) {                        /* 5 mahasiswa terakhir ditambahkan */
        int best = -1;
        for (int i = 0; i < g_mhs_n; i++)
            if (g_mhs[i].urut < prev && (best < 0 || g_mhs[i].urut > g_mhs[best].urut)) best = i;
        if (best < 0) break;
        if (k) sb_fmt(&s, ",");
        sb_mhs(&s, &g_mhs[best]);
        prev = g_mhs[best].urut;
    }
    sb_fmt(&s, "]}");
    reply(c, 200, 1, s.b, "Berhasil");
    free(s.b);
}

static void h_mhs_get(struct mg_connection *c, struct mg_http_message *hm) {
    char nim[16];
    Sb s = {0};
    if (query_var(hm, "nim", nim, sizeof nim)) {         /* binary search */
        int i = mhs_cari(nim, NULL);
        if (i < 0) { fail(c, 404, "Data tidak ditemukan"); return; }
        sb_fmt(&s, "["); sb_mhs(&s, &g_mhs[i]); sb_fmt(&s, "]");
    } else {
        sb_fmt(&s, "[");
        for (int i = 0; i < g_mhs_n; i++) { if (i) sb_fmt(&s, ","); sb_mhs(&s, &g_mhs[i]); }
        sb_fmt(&s, "]");
    }
    reply(c, 200, 1, s.b, "Berhasil");
    free(s.b);
}

static void h_mhs_post(struct mg_connection *c, struct mg_http_message *hm) {
    char nim[16], nama[64], prodi[48];
    if (!json_str(hm->body, "$.nim", nim, sizeof nim) || !json_str(hm->body, "$.nama", nama, sizeof nama)) {
        fail(c, 400, "NIM dan nama wajib diisi"); return;
    }
    if (!json_str(hm->body, "$.prodi", prodi, sizeof prodi)) snprintf(prodi, sizeof prodi, "-");
    int r = mhs_tambah(nim, nama, prodi);
    if (r == -1) { fail(c, 409, "NIM sudah terdaftar"); return; }
    if (r < 0)   { fail(c, 500, "Memori habis"); return; }
    persist_save();
    Sb s = {0};
    sb_mhs(&s, &g_mhs[mhs_cari(nim, NULL)]);
    reply(c, 201, 1, s.b, "Mahasiswa berhasil ditambahkan");
    free(s.b);
}

static void h_nilai_get(struct mg_connection *c) {
    Sb s = {0};
    sb_fmt(&s, "[");
    for (int i = 0; i < g_nilai_n; i++) { if (i) sb_fmt(&s, ","); sb_nilai(&s, &g_nilai[i]); }
    sb_fmt(&s, "]");
    reply(c, 200, 1, s.b, "Berhasil");
    free(s.b);
}

static void h_nilai_sort(struct mg_connection *c, struct mg_http_message *hm) {
    char order[8];
    int desc = 1;
    if (query_var(hm, "order", order, sizeof order) && strcasecmp(order, "asc") == 0) desc = 0;
    Nilai *salinan = malloc((g_nilai_n ? g_nilai_n : 1) * sizeof(Nilai));
    memcpy(salinan, g_nilai, g_nilai_n * sizeof(Nilai));
    nilai_urut(salinan, g_nilai_n, desc);                /* insertion sort pada salinan */
    Sb s = {0};
    sb_fmt(&s, "[");
    for (int i = 0; i < g_nilai_n; i++) { if (i) sb_fmt(&s, ","); sb_nilai(&s, &salinan[i]); }
    sb_fmt(&s, "]");
    reply(c, 200, 1, s.b, desc ? "Nilai diurutkan dari tertinggi" : "Nilai diurutkan dari terendah");
    free(s.b); free(salinan);
}

/* POST = tambah nilai baru, PUT = ubah nilai yang sudah ada */
static void h_nilai_tulis(struct mg_connection *c, struct mg_http_message *hm, int ubah) {
    char nim[16], kode[12];
    double v;
    if (!json_str(hm->body, "$.nim", nim, sizeof nim) || !json_str(hm->body, "$.kode_matkul", kode, sizeof kode)
        || !json_num(hm->body, "$.nilai", &v)) { fail(c, 400, "nim, kode_matkul, dan nilai wajib diisi"); return; }
    if (v < 0 || v > 100) { fail(c, 400, "Nilai harus 0-100"); return; }
    if (mhs_cari(nim, NULL) < 0) { fail(c, 404, "Mahasiswa tidak ditemukan"); return; }
    int mk = mk_cari(kode);
    if (mk < 0) { fail(c, 404, "Mata kuliah tidak ditemukan"); return; }
    int r = ubah ? nilai_ubah(nim, mk, v) : nilai_tambah(nim, mk, v);
    if (r == -1) { fail(c, ubah ? 404 : 409, ubah ? "Nilai belum ada, gunakan POST" : "Nilai sudah ada, gunakan PUT"); return; }
    if (r < 0)   { fail(c, 500, "Memori habis"); return; }
    persist_save();
    Sb s = {0};
    sb_nilai(&s, &g_nilai[nilai_cari(nim, mk)]);
    reply(c, ubah ? 200 : 201, 1, s.b, ubah ? "Nilai berhasil diubah" : "Nilai berhasil ditambahkan");
    free(s.b);
}

static void sb_mk_penuh(Sb *s, int i) {
    sb_fmt(s, "{\"kode\":"); sb_str(s, g_mk[i].kode);
    sb_fmt(s, ",\"nama\":"); sb_str(s, g_mk[i].nama);
    sb_fmt(s, ",\"sks\":%d,\"prasyarat\":[", g_mk[i].sks);
    int first = 1;
    for (int e = 0; e < g_edge_n; e++)
        if (g_edge[e].mk == i) { if (!first) sb_fmt(s, ","); sb_str(s, g_mk[g_edge[e].syarat].kode); first = 0; }
    sb_fmt(s, "]}");
}

static void h_mk_get(struct mg_connection *c) {
    Sb s = {0};
    sb_fmt(&s, "[");
    for (int i = 0; i < g_mk_n; i++) { if (i) sb_fmt(&s, ","); sb_mk_penuh(&s, i); }
    sb_fmt(&s, "]");
    reply(c, 200, 1, s.b, "Berhasil");
    free(s.b);
}

static void h_mk_post(struct mg_connection *c, struct mg_http_message *hm) {
    char kode[12], nama[48], tmp[160];
    double sks;
    if (!json_str(hm->body, "$.kode", kode, sizeof kode) || !json_str(hm->body, "$.nama", nama, sizeof nama)
        || !json_num(hm->body, "$.sks", &sks) || sks < 1 || sks > 10) {
        fail(c, 400, "kode, nama, dan sks (1-10) wajib diisi"); return;
    }
    if (mk_cari(kode) >= 0) { fail(c, 409, "Kode mata kuliah sudah ada"); return; }

    int syarat[16], ns = 0;                               /* prasyarat opsional: array atau teks */
    struct mg_str t = mg_json_get_tok(hm->body, "$.prasyarat");
    char kodes[16][12];
    int nk = 0;
    if (t.buf && t.len && t.buf[0] == '[') {
        for (int i = 0; i < 16; i++) {
            char path[32];
            snprintf(path, sizeof path, "$.prasyarat[%d]", i);
            if (!json_str(hm->body, path, kodes[nk], sizeof kodes[0])) break;
            nk++;
        }
    } else if (json_str(hm->body, "$.prasyarat", tmp, sizeof tmp)) {
        for (char *p = strtok(tmp, " "); p && nk < 16; p = strtok(NULL, " ")) snprintf(kodes[nk++], sizeof kodes[0], "%s", p);
    }
    for (int i = 0; i < nk; i++) {
        int idx = mk_cari(kodes[i]);
        if (idx < 0) { fail(c, 400, "Kode prasyarat tidak dikenal"); return; }
        syarat[ns++] = idx;
    }
    int baru = mk_tambah(kode, nama, (int)sks);
    if (baru < 0) { fail(c, 500, "Memori habis"); return; }
    for (int i = 0; i < ns; i++) graph_tambah_edge(syarat[i], baru);
    persist_save();
    Sb s = {0};
    sb_mk_penuh(&s, baru);
    reply(c, 201, 1, s.b, "Mata kuliah berhasil ditambahkan");
    free(s.b);
}

static void h_graph(struct mg_connection *c) {
    Sb s = {0};
    sb_fmt(&s, "{\"nodes\":[");
    for (int i = 0; i < g_mk_n; i++) { if (i) sb_fmt(&s, ","); sb_mk_ringkas(&s, i); }
    sb_fmt(&s, "],\"edges\":[");
    for (int e = 0; e < g_edge_n; e++) {                  /* from = prasyarat, to = mata kuliah */
        if (e) sb_fmt(&s, ",");
        sb_fmt(&s, "{\"from\":"); sb_str(&s, g_mk[g_edge[e].syarat].kode);
        sb_fmt(&s, ",\"to\":");   sb_str(&s, g_mk[g_edge[e].mk].kode);
        sb_fmt(&s, "}");
    }
    sb_fmt(&s, "]}");
    reply(c, 200, 1, s.b, "Berhasil");
    free(s.b);
}

static void h_krs(struct mg_connection *c, struct mg_http_message *hm) {
    char nim[16];
    if (!query_var(hm, "nim", nim, sizeof nim)) { fail(c, 400, "Parameter nim wajib diisi"); return; }
    if (mhs_cari(nim, NULL) < 0) { fail(c, 404, "Data tidak ditemukan"); return; }
    int *out = malloc((g_mk_n ? g_mk_n : 1) * sizeof(int));
    int n = krs_rekomendasi(nim, out);
    Sb s = {0};
    sb_fmt(&s, "[");
    for (int i = 0; i < n; i++) { if (i) sb_fmt(&s, ","); sb_mk_ringkas(&s, out[i]); }
    sb_fmt(&s, "]");
    reply(c, 200, 1, s.b, n ? "Berhasil" : "Tidak ada mata kuliah yang dapat direkomendasikan");
    free(s.b); free(out);
}

/* ================= Routing ================= */
static int uri_is(struct mg_http_message *hm, const char *p) {
    return hm->uri.len == strlen(p) && memcmp(hm->uri.buf, p, hm->uri.len) == 0;
}
static int is_m(struct mg_http_message *hm, const char *m) {
    return hm->method.len == strlen(m) && memcmp(hm->method.buf, m, hm->method.len) == 0;
}

static void route(struct mg_connection *c, struct mg_http_message *hm) {
    printf("%.*s %.*s\n", (int)hm->method.len, hm->method.buf, (int)hm->uri.len, hm->uri.buf);

    if (is_m(hm, "OPTIONS")) { mg_http_reply(c, 204, CORS, ""); return; }   /* preflight CORS */

    if (uri_is(hm, "/api/dashboard") && is_m(hm, "GET"))            h_dashboard(c);
    else if (uri_is(hm, "/api/mahasiswa")) {
        if (is_m(hm, "GET")) h_mhs_get(c, hm);
        else if (is_m(hm, "POST")) h_mhs_post(c, hm);
        else fail(c, 405, "Method tidak diizinkan");
    }
    else if (uri_is(hm, "/api/nilai/sort") && is_m(hm, "GET"))      h_nilai_sort(c, hm);
    else if (uri_is(hm, "/api/nilai")) {
        if (is_m(hm, "GET")) h_nilai_get(c);
        else if (is_m(hm, "POST")) h_nilai_tulis(c, hm, 0);
        else if (is_m(hm, "PUT")) h_nilai_tulis(c, hm, 1);
        else fail(c, 405, "Method tidak diizinkan");
    }
    else if (uri_is(hm, "/api/matakuliah/graph") && is_m(hm, "GET")) h_graph(c);
    else if (uri_is(hm, "/api/matakuliah")) {
        if (is_m(hm, "GET")) h_mk_get(c);
        else if (is_m(hm, "POST")) h_mk_post(c, hm);
        else fail(c, 405, "Method tidak diizinkan");
    }
    else if (uri_is(hm, "/api/krs/rekomendasi") && is_m(hm, "GET")) h_krs(c, hm);
    else if (hm->uri.len >= 4 && memcmp(hm->uri.buf, "/api", 4) == 0) fail(c, 404, "Endpoint tidak ditemukan");
    else {                                                            /* file frontend statis */
        struct mg_http_serve_opts opts = {0};
        opts.root_dir = g_frontend;
        mg_http_serve_dir(c, hm, &opts);
    }
}

static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) route(c, (struct mg_http_message *)ev_data);
}

int main(int argc, char **argv) {
    char *rel = argc > 1 ? argv[1] : "../frontend";
    if (!realpath(rel, g_frontend)) {                    /* Mongoose menolak path berisi ".." */
        fprintf(stderr, "Folder frontend tidak ditemukan: %s\n", rel);
        return 1;
    }
    persist_init();
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    if (!mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL)) {
        fprintf(stderr, "Gagal membuka port 8000 (sudah dipakai?)\n");
        return 1;
    }
    printf("Server berjalan: http://localhost:8000  (frontend: %s)\n", g_frontend);
    printf("Mahasiswa=%d MataKuliah=%d Nilai=%d\n", g_mhs_n, g_mk_n, g_nilai_n);
    for (;;) mg_mgr_poll(&mgr, 100);
    mg_mgr_free(&mgr);
    return 0;
}
