/* =========================================
   1. KONFIGURASI API
========================================= */

// Alamat server backend C
const API_BASE = "http://localhost:8000/api";


/* =========================================
   2. FUNGSI KOMUNIKASI API
========================================= */

// Mengirim request ke backend C
async function apiRequest(endpoint, options = {}) {

    const { headers, ...rest } = options;

    let response;

    try {
        response = await fetch(`${API_BASE}${endpoint}`, {
            ...rest,
            headers: {
                "Content-Type": "application/json",
                ...headers
            }
        });
    } catch (error) {
        console.error("API Error:", error);
        throw new Error("Server tidak dapat dihubungi. Pastikan backend berjalan.");
    }

    // Backend selalu membalas JSON: {success, data, message}
    let body = null;

    try {
        body = await response.json();
    } catch (error) {
        // respons bukan JSON
    }

    if (!response.ok || body?.success === false) {
        const message = body?.message || `HTTP Error: ${response.status}`;
        console.error("API Error:", message);
        throw new Error(message);
    }

    return body;
}


// Mengambil data dari respons API
function getData(response) {
    return response?.data ?? response;
}


// Mencegah teks dari API menjadi HTML berbahaya
function escapeHTML(value) {
    return String(value ?? "").replace(/[&<>"']/g, char => ({
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#039;"
    })[char]);
}


// Menampilkan pesan kosong
function emptyRow(columns, message) {
    return `
        <tr>
            <td colspan="${columns}" class="text-center text-muted py-4">
                ${escapeHTML(message)}
            </td>
        </tr>
    `;
}


/* =========================================
   3. NAVIGASI SIDEBAR
========================================= */

function showPage(pageId) {

    // Sembunyikan seluruh halaman
    document.querySelectorAll(".page-section").forEach(section => {
        section.classList.add("d-none");
    });

    // Tampilkan halaman yang dipilih
    const selectedPage = document.getElementById(pageId);

    if (!selectedPage) return;

    selectedPage.classList.remove("d-none");

    // Atur navigasi aktif
    document.querySelectorAll(".menu-item").forEach(item => {
        item.classList.remove("active");
    });

    const activeMenu = document.querySelector(
        `.menu-item[href="#${pageId}"]`
    );

    if (activeMenu) {
        activeMenu.classList.add("active");
    }

    // Muat data sesuai halaman
    switch (pageId) {
        case "dashboard":
            loadDashboard();
            break;

        case "mahasiswa":
            loadMahasiswa();
            break;

        case "nilai":
            loadNilai();
            break;

        case "matakuliah":
            loadMatakuliah();
            break;

        case "prasyarat":
            renderGraph();   // gambar saat halaman terlihat (vis-network butuh container berukuran nyata)
            break;
    }

    // Perbarui URL tanpa reload
    history.replaceState(null, "", `#${pageId}`);
}


// Event navigasi sidebar
document.querySelectorAll(".menu-item").forEach(item => {

    item.addEventListener("click", function (event) {

        event.preventDefault();

        const pageId = this.getAttribute("href").substring(1);

        showPage(pageId);

    });

});


/* =========================================
   4. DASHBOARD
========================================= */

async function loadDashboard() {

    try {

        const response = await apiRequest("/dashboard");

        const data = getData(response);

        document.getElementById("totalMahasiswa").textContent =
            data.total_mahasiswa ?? 0;

        document.getElementById("totalMatkul").textContent =
            data.total_matkul ?? 0;

        document.getElementById("totalNilai").textContent =
            data.total_nilai ?? 0;

        document.getElementById("totalPrasyarat").textContent =
            data.total_prasyarat ?? 0;

        // Tampilkan beberapa mahasiswa terbaru
        const students = data.mahasiswa_terbaru ?? [];

        renderDashboardStudents(students);

    } catch (error) {

        console.warn("Dashboard belum terhubung dengan backend.");

    }

}


// Render mahasiswa di dashboard
function renderDashboardStudents(students) {

    const table = document.getElementById("dashboardStudentTable");

    if (!students.length) {
        table.innerHTML = emptyRow(4, "Belum ada data mahasiswa.");
        return;
    }

    table.innerHTML = students.map(student => `

        <tr>
            <td>${escapeHTML(student.nim)}</td>

            <td>
                <strong>${escapeHTML(student.nama)}</strong>
            </td>

            <td>${escapeHTML(student.prodi)}</td>

            <td>
                <span class="badge bg-success-subtle text-success">
                    ${escapeHTML(student.status ?? "Aktif")}
                </span>
            </td>
        </tr>

    `).join("");

}


/* =========================================
   5. DATA MAHASISWA
========================================= */

async function loadMahasiswa() {

    const table = document.getElementById("studentTable");

    table.innerHTML = emptyRow(5, "Memuat data mahasiswa...");

    try {

        const response = await apiRequest("/mahasiswa");

        const students = getData(response);

        renderMahasiswa(students);

    } catch (error) {

        table.innerHTML = emptyRow(
            5,
            `Gagal memuat data: ${error.message}`
        );

    }

}


// Menampilkan data mahasiswa
function renderMahasiswa(students) {

    const table = document.getElementById("studentTable");

    if (!Array.isArray(students) || !students.length) {
        table.innerHTML = emptyRow(5, "Belum ada data mahasiswa.");
        return;
    }

    table.innerHTML = students.map(student => `

        <tr>
            <td>${escapeHTML(student.nim)}</td>

            <td>
                <strong>${escapeHTML(student.nama)}</strong>
            </td>

            <td>${escapeHTML(student.prodi)}</td>

            <td>
                <span class="badge bg-success-subtle text-success">
                    ${escapeHTML(student.status ?? "Aktif")}
                </span>
            </td>

            <td>
                <button
                    class="btn btn-sm btn-outline-primary"
                    onclick="searchMahasiswa('${escapeHTML(student.nim)}')"
                    title="Lihat data">
                    <i class="bi bi-search"></i>
                </button>
            </td>
        </tr>

    `).join("");

}


// Pencarian mahasiswa berdasarkan NIM
async function searchMahasiswa(nim) {

    const table = document.getElementById("studentTable");

    table.innerHTML = emptyRow(5, "Mencari mahasiswa...");

    try {

        const response = await apiRequest(
            `/mahasiswa?nim=${encodeURIComponent(nim)}`
        );

        const result = getData(response);

        const students = Array.isArray(result)
            ? result
            : result
                ? [result]
                : [];

        renderMahasiswa(students);

    } catch (error) {

        table.innerHTML = emptyRow(
            5,
            error.message
        );

    }

}


// Tombol cari
document.getElementById("btnCariMahasiswa")
    .addEventListener("click", function () {

        const nim = document.getElementById("searchNim").value.trim();

        // Kolom kosong = tampilkan semua mahasiswa
        if (!nim) {
            loadMahasiswa();
            return;
        }

        searchMahasiswa(nim);

    });


// Cari ketika menekan Enter
document.getElementById("searchNim")
    .addEventListener("keydown", function (event) {

        if (event.key === "Enter") {
            document.getElementById("btnCariMahasiswa").click();
        }

    });


// Tambah mahasiswa
document.getElementById("btnTambahMahasiswa")
    .addEventListener("click", async function () {

        const nim = prompt("Masukkan NIM mahasiswa:");

        if (nim === null || !nim.trim()) return;

        const nama = prompt("Masukkan nama mahasiswa:");

        if (nama === null || !nama.trim()) return;

        const prodi = prompt("Masukkan program studi:");

        if (prodi === null || !prodi.trim()) return;

        try {

            await apiRequest("/mahasiswa", {
                method: "POST",
                body: JSON.stringify({
                    nim: nim.trim(),
                    nama: nama.trim(),
                    prodi: prodi.trim()
                })
            });

            alert("Mahasiswa berhasil ditambahkan!");

            loadMahasiswa();

        } catch (error) {

            alert(`Gagal menambahkan mahasiswa: ${error.message}`);

        }

    });


/* =========================================
   6. PENGELOLAAN NILAI
========================================= */

async function loadNilai() {

    const table = document.getElementById("gradeTable");

    table.innerHTML = emptyRow(5, "Memuat data nilai...");

    try {

        const response = await apiRequest("/nilai");

        const grades = getData(response);

        renderNilai(grades);

    } catch (error) {

        table.innerHTML = emptyRow(
            5,
            `Gagal memuat data nilai: ${error.message}`
        );

    }

}


// Render tabel nilai
function renderNilai(grades) {

    const table = document.getElementById("gradeTable");

    if (!Array.isArray(grades) || !grades.length) {
        table.innerHTML = emptyRow(5, "Belum ada data nilai.");
        return;
    }

    table.innerHTML = grades.map(grade => `

        <tr>
            <td>${escapeHTML(grade.nim)}</td>

            <td>
                <strong>${escapeHTML(grade.nama)}</strong>
            </td>

            <td>
                ${escapeHTML(grade.matakuliah)}
                <small class="text-muted">${escapeHTML(grade.kode_matkul ?? "")}</small>
            </td>

            <td>
                <strong>${escapeHTML(grade.nilai)}</strong>
            </td>

            <td>
                <span class="badge bg-primary-subtle text-primary">
                    ${escapeHTML(grade.grade)}
                </span>
            </td>
        </tr>

    `).join("");

}


// Sorting nilai melalui backend C
document.getElementById("btnUrutkanNilai")
    .addEventListener("click", async function () {

        const button = this;

        button.disabled = true;

        try {

            const response = await apiRequest(
                "/nilai/sort?order=desc"
            );

            const grades = getData(response);

            renderNilai(grades);

        } catch (error) {

            alert(`Gagal mengurutkan nilai: ${error.message}`);

        } finally {

            button.disabled = false;

        }

    });


/* =========================================
   7. DATA MATA KULIAH
========================================= */

async function loadMatakuliah() {

    const container = document.getElementById("courseList");

    container.innerHTML = `
        <div class="col-12 text-center text-muted py-4">
            Memuat mata kuliah...
        </div>
    `;

    try {

        const response = await apiRequest("/matakuliah");

        const courses = getData(response);

        renderMatakuliah(courses);

    } catch (error) {

        container.innerHTML = `
            <div class="col-12 text-center text-muted py-4">
                Gagal memuat mata kuliah: ${escapeHTML(error.message)}
            </div>
        `;

    }

}


// Render daftar mata kuliah
function renderMatakuliah(courses) {

    const container = document.getElementById("courseList");

    if (!Array.isArray(courses) || !courses.length) {
        container.innerHTML = `
            <div class="col-12 text-center text-muted py-4">
                Belum ada data mata kuliah.
            </div>
        `;
        return;
    }

    container.innerHTML = courses.map(course => `

        <div class="col-md-6 col-xl-4">

            <div class="stat-card">

                <div class="stat-icon blue">
                    <i class="bi bi-book-half"></i>
                </div>

                <div>
                    <p>${escapeHTML(course.kode ?? "Mata Kuliah")}</p>

                    <h6 class="fw-bold mb-1">
                        ${escapeHTML(course.nama)}
                    </h6>

                    <small>
                        ${escapeHTML(course.sks ?? 0)} SKS
                    </small>

                    <div>
                        <small class="text-muted">
                            Prasyarat: ${
                                Array.isArray(course.prasyarat) && course.prasyarat.length
                                    ? course.prasyarat.map(escapeHTML).join(", ")
                                    : "-"
                            }
                        </small>
                    </div>
                </div>

            </div>

        </div>

    `).join("");

}


// Tambah mata kuliah
document.getElementById("btnTambahMatkul")
    .addEventListener("click", async function () {

        const kode = prompt("Masukkan kode mata kuliah:");

        if (kode === null || !kode.trim()) return;

        const nama = prompt("Masukkan nama mata kuliah:");

        if (nama === null || !nama.trim()) return;

        const sks = Number(prompt("Masukkan jumlah SKS:"));

        if (!Number.isInteger(sks) || sks <= 0) {
            alert("Jumlah SKS harus berupa bilangan bulat positif.");
            return;
        }

        const prasyaratInput = prompt(
            "Kode prasyarat (pisahkan dengan koma, kosongkan jika tidak ada):"
        );

        if (prasyaratInput === null) return;

        const prasyarat = prasyaratInput
            .split(",")
            .map(item => item.trim())
            .filter(Boolean);

        try {

            await apiRequest("/matakuliah", {
                method: "POST",
                body: JSON.stringify({
                    kode: kode.trim(),
                    nama: nama.trim(),
                    sks: sks,
                    prasyarat: prasyarat
                })
            });

            alert("Mata kuliah berhasil ditambahkan!");

            loadMatakuliah();

        } catch (error) {

            alert(`Gagal menambahkan mata kuliah: ${error.message}`);

        }

    });


/* =========================================
   8. REKOMENDASI KRS
========================================= */

document.getElementById("btnCekKrs")
    .addEventListener("click", async function () {

        const nim = document.getElementById("krsNim").value.trim();

        const resultContainer = document.getElementById("krsResult");

        if (!nim) {
            alert("Masukkan NIM mahasiswa terlebih dahulu.");
            return;
        }

        resultContainer.innerHTML = `
            <p class="text-muted">Memeriksa prasyarat mata kuliah...</p>
        `;

        try {

            const response = await apiRequest(
                `/krs/rekomendasi?nim=${encodeURIComponent(nim)}`
            );

            const courses = getData(response);

            renderKRS(courses);

        } catch (error) {

            resultContainer.innerHTML = `
                <p class="text-danger">
                    ${escapeHTML(error.message)}
                </p>
            `;

        }

    });


// Menampilkan rekomendasi KRS
function renderKRS(courses) {

    const container = document.getElementById("krsResult");

    if (!Array.isArray(courses) || !courses.length) {

        container.innerHTML = `
            <div class="alert alert-info">
                Tidak ada rekomendasi mata kuliah.
            </div>
        `;

        return;
    }

    container.innerHTML = `

        <h6 class="fw-bold mb-3">
            Mata Kuliah yang Direkomendasikan
        </h6>

        <div class="row g-3">

            ${courses.map(course => `

                <div class="col-md-6">

                    <div class="stat-card">

                        <div class="stat-icon green">
                            <i class="bi bi-check-circle-fill"></i>
                        </div>

                        <div>
                            <h6 class="fw-bold mb-1">
                                ${escapeHTML(course.nama)}
                            </h6>

                            <small>
                                ${escapeHTML(course.kode ?? "")}
                                &middot; ${escapeHTML(course.sks ?? 0)} SKS
                            </small>

                            <div class="mt-2">
                                <span class="badge bg-success-subtle text-success">
                                    Memenuhi Prasyarat
                                </span>
                            </div>
                        </div>

                    </div>

                </div>

            `).join("")}

        </div>

    `;

}

// render graph prasyarat mata kuliah
async function renderGraph() {
    const container = document.getElementById("graphContainer");

    // Library vis-network dimuat dari internet (CDN)
    if (typeof vis === "undefined") {
        container.innerHTML = "<p class=\"text-danger p-3\">Library graph (vis-network) tidak termuat. Periksa koneksi internet.</p>";
        return;
    }

    // Ambil data graph dari backend C
    let graph;
    try {
        graph = getData(await apiRequest("/matakuliah/graph"));
    } catch (error) {
        container.innerHTML = `<p class="text-danger p-3">Gagal memuat graph prasyarat: ${escapeHTML(error.message)}</p>`;
        return;
    }

    if (!graph?.nodes?.length) {
        container.innerHTML = "<p class=\"text-muted p-3\">Belum ada data mata kuliah.</p>";
        return;
    }

    container.innerHTML = "";          // hapus ikon/teks placeholder
    container.style.display = "block";
    container.style.height = "460px";

    const nodes = new vis.DataSet(
        graph.nodes.map(item => ({
            id: item.kode,
            label: `${item.kode}\n${item.nama}`,
            shape: "box",
            color: {
                background: "#DBEAFE",
                border: "#2563EB"
            },
            font: {
                color: "#1E3A8A",
                size: 14
            },
            margin: 12
        }))
    );

    const edges = new vis.DataSet(
        graph.edges.map(item => ({
            from: item.from,
            to: item.to,
            arrows: "to"
        }))
    );

    new vis.Network(
        container,
        { nodes, edges },

        {
            layout: {
                hierarchical: {
                    enabled: true,
                    direction: "UD",
                    sortMethod: "directed",
                    levelSeparation: 120,
                    nodeSpacing: 180
                }
            },
            physics: {
                enabled: false
            },
            interaction: {
                dragNodes: true,
                zoomView: true,
                dragView: true
            },
            edges: {
                color: "#64748B",
                arrows: {
                    to: {
                        enabled: true,
                        scaleFactor: 0.8
                    }
                },
                smooth: true
            }
        }

    );
}


/* =========================================
   9. INISIALISASI APLIKASI
========================================= */

document.addEventListener("DOMContentLoaded", function () {

    const initialPage = location.hash.replace("#", "");

    const validPages = [
        "dashboard",
        "mahasiswa",
        "nilai",
        "matakuliah",
        "prasyarat",
        "krs"
    ];

    showPage(
        validPages.includes(initialPage)
            ? initialPage
            : "dashboard"
    );

});