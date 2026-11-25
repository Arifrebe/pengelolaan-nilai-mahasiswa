# Kontrak API - Sistem Perencanaan Akademik

Server: `http://localhost:8000` | Prefix: `/api` | JSON | CORS aktif (termasuk preflight OPTIONS)

Respons sukses: `{"success":true,"data":...,"message":"Berhasil"}`
Respons gagal  : `{"success":false,"data":null,"message":"..."}` dengan status HTTP 400/404/405/409/500.
Data kosong selalu `[]`. NIM dan kode matkul bertipe string.

| Method | Endpoint | Keterangan |
|---|---|---|
| GET | /api/dashboard | `total_mahasiswa, total_matkul, total_nilai, total_prasyarat, mahasiswa_terbaru[]` (5 terakhir) |
| GET | /api/mahasiswa | Daftar mahasiswa: `nim, nama, prodi, status` |
| GET | /api/mahasiswa?nim=123 | Cari NIM (binary search); 404 jika tidak ada. Data berupa array 1 elemen |
| POST | /api/mahasiswa | Body `{nim, nama, prodi}`; 409 jika NIM sudah ada |
| GET | /api/nilai | Baris: `nim, nama, kode_matkul, matakuliah, nilai, grade` |
| GET | /api/nilai/sort?order=desc | Urut nilai (insertion sort); `order=asc` untuk menaik |
| POST | /api/nilai | Body `{nim, kode_matkul, nilai}`; 409 jika sudah ada |
| PUT | /api/nilai | Body sama; 404 jika belum ada |
| GET | /api/matakuliah | `kode, nama, sks, prasyarat[]` |
| POST | /api/matakuliah | Body `{kode, nama, sks, prasyarat?:["IK101"]}` (**tambahan**, dipakai tombol Tambah Matkul) |
| GET | /api/matakuliah/graph | `nodes[{kode,nama,sks}]`, `edges[{from,to}]` (**from = prasyarat, to = mata kuliah**) |
| GET | /api/krs/rekomendasi?nim=123 | `[{kode,nama,sks}]` hasil BFS+Queue, maks 24 SKS |

Grade: A >= 85, B >= 75, C >= 60, D >= 50, E < 50. Lulus jika nilai >= 60.
Data tersimpan otomatis di folder `backend/data/*.csv`.

Catatan: halaman prasyarat (vis-network) memuat graph dari `GET /api/matakuliah/graph`; data mock di frontend sudah tidak dipakai.
