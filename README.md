# Sistem Perencanaan Akademik dan Pengelolaan Nilai Mahasiswa

Proyek Struktur Data. Semua logika program ditulis dalam **bahasa C**. Website hanya menjadi
tampilan: ia meminta data ke server C lewat API JSON.

Panduan ini ditulis untuk pemula yang biasa memakai CMD atau PowerShell di Windows.
Ikuti dari Bagian 2 secara berurutan.

---

## Daftar isi
1. Gambaran singkat
2. Persiapan di Windows (sekali saja)
3. Memindahkan proyek ke Ubuntu
4. Menjalankan server
5. Memakai website
6. Menambah atau mengubah nilai
7. Skenario demo untuk presentasi
8. Menghentikan dan menjalankan lagi
9. Penyimpanan data
10. Struktur folder
11. Pemecahan masalah
12. Kamus istilah

---

## 1. Gambaran singkat

**Fitur dan struktur data yang dipakai**

| Fitur | Struktur data / algoritma |
|-------|---------------------------|
| Tambah dan tampilkan mahasiswa | Array dinamis (`realloc`) |
| Catat dan ubah nilai | Array dinamis |
| Cari mahasiswa berdasarkan NIM | Binary search |
| Urutkan nilai tertinggi ke terendah | Insertion sort |
| Daftar mata kuliah dan graph prasyarat | Array dinamis + Graph |
| Rekomendasi mata kuliah untuk KRS | BFS dengan Queue (circular) |

**Aturan**
- Lulus jika nilai **60 ke atas**.
- Grade: A (85 ke atas), B (75 ke atas), C (60 ke atas), D (50 ke atas), E (di bawah 50).
- Rekomendasi KRS: mata kuliah yang belum lulus, **semua prasyaratnya sudah lulus**, maksimal **24 SKS**.

**Cara kerjanya**
```
Browser (website)  --permintaan JSON-->  Server C (port 8000)  -->  struktur data C
                   <--------hasil-------                       -->  file CSV (penyimpanan)
```
Server C berjalan di komputermu sendiri. Tidak perlu internet, kecuali untuk menggambar graph
(Bagian 5).

---

## 2. Persiapan di Windows (sekali saja)

### 2.1 Kenapa tidak bisa langsung di CMD atau PowerShell?
Server C ini memakai fungsi khas Linux. Windows punya fitur bernama **WSL** (Windows Subsystem
for Linux) yang menjalankan Linux kecil bernama **Ubuntu** di dalam Windows. Kamu tetap memakai
komputer Windows seperti biasa, hanya terminalnya yang berbeda untuk proyek ini.

Perbedaan perintah yang sering dipakai:

| Tujuan | CMD / PowerShell | Terminal Ubuntu |
|--------|------------------|-----------------|
| Lihat isi folder | `dir` | `ls` |
| Pindah folder | `cd folder` | `cd folder` |
| Naik satu folder | `cd ..` | `cd ..` |
| Lihat folder sekarang | `cd` | `pwd` |
| Salin file | `copy` | `cp` |
| Hapus file | `del` | `rm` |
| Hapus folder | `rmdir /s` | `rm -r` |
| Bersihkan layar | `cls` | `clear` |

Catatan: Linux **membedakan huruf besar dan kecil** (`Proyek` dan `proyek` dianggap berbeda), dan
alamat folder memakai garis miring biasa `/`, bukan `\`.

### 2.2 Pasang WSL
1. Klik tombol Start, ketik **PowerShell**, klik kanan hasilnya, pilih **Run as administrator**.
2. Ketik perintah ini lalu tekan Enter:
   ```
   wsl --install
   ```
3. Tunggu sampai selesai, lalu **restart komputer**.
4. Setelah menyala, jendela Ubuntu biasanya terbuka sendiri dan meminta kamu membuat akun Linux
   (jika tidak terbuka, ketik **Ubuntu** di menu Start dan buka):
   - `Enter new UNIX username`: ketik nama bebas, huruf kecil tanpa spasi (contoh: `raihan`).
   - `New password`: ketik password lalu Enter. **Huruf yang diketik tidak muncul di layar.**
     Itu normal, bukan error. Ingat password ini, akan dipakai di langkah 2.4.
   - Ketik password sekali lagi untuk konfirmasi.

Syarat: Windows 10 versi 2004 ke atas atau Windows 11. Jika muncul pesan tentang *virtualization*
(virtualisasi), aktifkan fitur Virtualization di BIOS komputer, lalu coba lagi.

### 2.3 Buka terminal Ubuntu dari VS Code
Kamu tidak perlu membuka jendela Ubuntu terpisah. Di VS Code:
1. Buka panel terminal (menu **Terminal > New Terminal**, atau tekan `` Ctrl + ` ``).
2. Di kanan atas panel terminal ada tombol **+** dan **panah kecil (v)** di sebelahnya.
   Klik panah kecilnya, lalu pilih **Ubuntu (WSL)**.
3. Terminal berubah menjadi Linux. Tandanya, tampilannya seperti:
   ```
   raihan@NAMA-PC:~$
   ```
   Tanda `~` artinya kamu berada di folder rumahmu di Ubuntu.

Jika pilihan Ubuntu (WSL) tidak ada, pasang extension **WSL** (dari Microsoft) di VS Code, lalu
tutup dan buka VS Code lagi.

### 2.4 Pasang compiler C
Di terminal Ubuntu, jalankan:
```
sudo apt update
sudo apt install build-essential unzip
```
- `sudo` meminta password Linux dari langkah 2.2 (tidak terlihat saat diketik, lalu Enter).
- Jika ditanya `Do you want to continue? [Y/n]`, ketik `Y` lalu Enter.
- Tunggu sampai selesai.

Cek berhasil atau tidak:
```
gcc --version
make --version
```
Keduanya harus menampilkan nomor versi. Jika muncul `command not found`, ulangi langkah di atas.

---

## 3. Memindahkan proyek ke Ubuntu

Anggap file `proyek-akademik-c.zip` ada di folder **Downloads** Windows.

1. Cari nama user Windows-mu:
   ```
   ls /mnt/c/Users
   ```
   Akan muncul beberapa nama folder. Pilih yang sesuai namamu (abaikan `Public`, `Default`,
   `All Users`). Misalnya `Raihan`.
2. Salin zip ke folder rumah Ubuntu (ganti `Raihan` sesuai hasil langkah 1):
   ```
   cp /mnt/c/Users/Raihan/Downloads/proyek-akademik-c.zip ~
   ```
   Jika nama folder mengandung spasi, apit dengan tanda kutip: `"/mnt/c/Users/Nama Saya/Downloads/..."`.
3. Ekstrak lalu masuk ke folder backend:
   ```
   cd ~
   unzip proyek-akademik-c.zip
   cd proyek/backend
   ```

Pastikan benar: ketik `ls`, akan terlihat `Makefile`, `lib`, dan `src`.

---

## 4. Menjalankan server

Di folder `proyek/backend` (terminal Ubuntu), ketik:
```
make run
```
Pertama kali, `make` mengompilasi program (beberapa detik). Lalu muncul:
```
Server berjalan: http://localhost:8000  (frontend: ...)
Mahasiswa=3 MataKuliah=10 Nilai=8
```
Artinya server sudah hidup. **Biarkan terminal ini tetap terbuka.** Jika ditutup, server mati dan
website tidak bisa mengambil data.

Buka browser biasa di Windows (Chrome, Edge, dan lain-lain) dan masuk ke alamat:
```
http://localhost:8000
```

> Jangan membuka `index.html` dengan klik dua kali. Selalu lewat `http://localhost:8000`.

Setiap kali ada permintaan dari website, terminal akan mencetak barisnya (misalnya
`GET /api/mahasiswa`). Itu tandanya server bekerja.

---

## 5. Memakai website

Menu ada di sisi kiri halaman.

**Dashboard**
Ringkasan: jumlah mahasiswa, mata kuliah, nilai, dan prasyarat, serta mahasiswa yang baru ditambahkan.

**Data Mahasiswa**
- *Menambah:* klik **Tambah Mahasiswa**, isi NIM, nama, dan prodi. NIM yang sudah terdaftar ditolak.
- *Mencari:* ketik NIM di kolom pencarian, klik **Cari** (memakai binary search). Kosongkan kolom
  lalu cari lagi untuk menampilkan semua mahasiswa.

**Pengelolaan Nilai**
- Tabel berisi NIM, nama, mata kuliah, nilai, dan grade.
- Klik **Urutkan Nilai** untuk mengurutkan dari tertinggi ke terendah (insertion sort).
- Form tambah atau ubah nilai belum ada di halaman ini. Gunakan cara di Bagian 6.

**Mata Kuliah**
- Daftar kode, nama, dan SKS.
- Klik **Tambah Matkul** untuk menambah mata kuliah baru.

**Prasyarat Mata Kuliah**
Graph interaktif. Panah mengarah **dari mata kuliah prasyarat ke mata kuliah yang membutuhkannya**
(contoh: Algoritma dan Pemrograman -> Struktur Data). Kotak bisa digeser dan graph bisa di-zoom.
Halaman ini butuh **internet**, karena library gambar graph (vis-network) dimuat dari internet.

**Rekomendasi KRS**
Ketik NIM mahasiswa, klik **Cek**. Muncul kartu mata kuliah yang boleh diambil mahasiswa tersebut
(maksimal 24 SKS).

---

## 6. Menambah atau mengubah nilai

Form nilai belum tersedia di website, jadi pakai perintah. Server harus tetap berjalan, jadi
**buka terminal Ubuntu kedua** di VS Code (klik tombol **+** lalu pilih Ubuntu (WSL) lagi).

Tambah nilai baru:
```
curl -X POST http://localhost:8000/api/nilai -H "Content-Type: application/json" -d '{"nim":"2023003","kode_matkul":"IK102","nilai":77}'
```
Ubah nilai yang sudah ada:
```
curl -X PUT http://localhost:8000/api/nilai -H "Content-Type: application/json" -d '{"nim":"2023003","kode_matkul":"IK102","nilai":90}'
```
Arti isinya: `nim` = NIM mahasiswa, `kode_matkul` = kode mata kuliah, `nilai` = angka 0 sampai 100.

Aturan: POST hanya untuk nilai yang belum ada, PUT hanya untuk nilai yang sudah ada. Jika salah,
server menjawab pesan seperti `Nilai sudah ada, gunakan PUT`. Setelah itu muat ulang halaman
website untuk melihat hasilnya.

---

## 7. Skenario demo untuk presentasi

Data contoh bawaan:

| NIM | Nama | Sudah lulus | Rekomendasi KRS |
|-----|------|-------------|-----------------|
| 2023001 | Raihan | IK101, IK102, IK201, IK202 | IK203, IK301, IK302 |
| 2023002 | Sinta | IK101, IK202 (IK102 nilai 55, belum lulus) | IK102 (mengulang), IK201, IK302 |
| 2023003 | Budi | IK101 | IK102, IK201, IK202 |

Urutan yang disarankan:
1. Dashboard, lalu cari NIM `2023001`.
2. Buka Pengelolaan Nilai dan klik **Urutkan Nilai**.
3. Buka graph prasyarat dan jelaskan arah panah.
4. Cek KRS untuk `2023002`. IK203 dan IK301 belum muncul karena IK102 belum lulus.
5. Ubah nilai IK102 milik Sinta menjadi 80:
   ```
   curl -X PUT http://localhost:8000/api/nilai -H "Content-Type: application/json" -d '{"nim":"2023002","kode_matkul":"IK102","nilai":80}'
   ```
6. Cek KRS `2023002` lagi. Sekarang IK203 dan IK301 muncul.

Jika data sudah berubah dan ingin kembali ke data awal, lihat Bagian 9.

---

## 8. Menghentikan dan menjalankan lagi

- **Menghentikan server:** klik terminal tempat server berjalan, tekan `Ctrl + C`.
- **Menjalankan lagi di lain hari:**
  1. Buka VS Code, buka terminal **Ubuntu (WSL)**.
  2. `cd ~/proyek/backend`
  3. `make run`
  4. Buka `http://localhost:8000`.

Langkah di Bagian 2 dan 3 tidak perlu diulang.

**Membuka folder proyek di VS Code:** di terminal Ubuntu, masuk ke `~/proyek`, lalu ketik
`code .` (butuh extension WSL). Kamu juga bisa melihat file proyek dari Windows Explorer lewat
alamat `\\wsl$\Ubuntu\home\NAMA_USER_LINUX\proyek`.

---

## 9. Penyimpanan data

Data tersimpan otomatis di folder `backend/data/` (file CSV) setiap ada perubahan, dan dimuat lagi
saat server dijalankan. Menutup server tidak menghapus data.

Mengembalikan ke data contoh:
1. Hentikan server (`Ctrl + C`).
2. Jalankan `rm -r data` di folder `backend`.
3. Jalankan `make run` lagi.

---

## 10. Struktur folder

```
proyek/
├── README.md              panduan ini
├── API.md                 daftar endpoint untuk tim
├── backend/
│   ├── Makefile           resep kompilasi (dipakai oleh perintah make)
│   ├── lib/               mongoose.c, mongoose.h (library web server untuk C)
│   └── src/
│       ├── akademik.h     tipe data dan deklarasi fungsi
│       ├── mahasiswa.c    array dinamis + binary search
│       ├── nilai.c        array dinamis + insertion sort + grade
│       ├── graph.c        graph prasyarat + queue + BFS rekomendasi KRS
│       ├── persist.c      simpan dan muat CSV + data contoh
│       └── server.c       routing dan JSON API
└── frontend/              index.html, app.js, style.css (tampilan web)
```

---

## 11. Pemecahan masalah

| Yang terjadi | Penyebab dan solusi |
|--------------|---------------------|
| `wsl --install` gagal atau meminta virtualisasi | Aktifkan Virtualization di BIOS, pastikan Windows 10 versi 2004 ke atas atau Windows 11 |
| Pilihan **Ubuntu (WSL)** tidak ada di VS Code | Pasang extension WSL, atau buka Ubuntu lewat menu Start |
| `make: command not found` atau `gcc: command not found` | Ulangi `sudo apt install build-essential` (Bagian 2.4) |
| `unzip: command not found` | `sudo apt install unzip` |
| `cp: cannot stat ... No such file` | Nama user Windows atau lokasi zip salah. Cek dengan `ls /mnt/c/Users` lalu `ls /mnt/c/Users/NAMA/Downloads` |
| `Gagal membuka port 8000` | Server sebelumnya masih berjalan. Cari terminalnya, tekan `Ctrl + C`, atau tutup terminal itu |
| Halaman tampil tapi tabel kosong atau error | Server belum jalan, atau halaman dibuka lewat file langsung. Gunakan `http://localhost:8000` |
| Graph prasyarat kosong | Tidak ada internet (library graph tidak termuat), atau server mati. Muat ulang halaman |
| Halaman tidak berubah setelah data diubah | Tekan `F5` (muat ulang) |
| Ingin mengosongkan atau mengembalikan data | Lihat Bagian 9 |
| Salah ketik perintah | Tekan panah atas untuk memanggil perintah sebelumnya, perbaiki, lalu Enter |
| Cara menempel (paste) di terminal | `Ctrl + V` di VS Code, atau `Ctrl + Shift + V` di jendela Ubuntu terpisah |

---

## 12. Kamus istilah

- **Terminal / CMD / PowerShell:** jendela tempat mengetik perintah.
- **Ubuntu:** salah satu jenis Linux. Di sini dijalankan di dalam Windows lewat WSL.
- **WSL:** fitur Windows untuk menjalankan Linux tanpa menginstal komputer baru.
- **Compiler (gcc):** program yang mengubah kode C menjadi program yang bisa dijalankan.
- **make:** alat yang menjalankan resep kompilasi di file `Makefile`.
- **Server:** program yang terus berjalan dan menjawab permintaan dari browser.
- **localhost:** alamat untuk komputermu sendiri. `8000` adalah nomor port server ini.
- **API / endpoint:** alamat di server (misalnya `/api/mahasiswa`) yang dipanggil website untuk
  mengambil atau mengirim data.
- **JSON:** format teks untuk mengirim data antara server dan website.
- **CSV:** file teks sederhana tempat data disimpan.
- **BFS:** penelusuran graph level demi level memakai queue (dipakai untuk rekomendasi KRS).
- **Prasyarat:** mata kuliah yang harus lulus sebelum mengambil mata kuliah lain.

Daftar lengkap endpoint dan format datanya ada di `API.md`.
