/**
 * TUGAS PROYEK PEMROGRAMAN DASAR
 * Domain: Aplikasi Manajemen Klinik (Versi UI Menarik & Fitur Lengkap)
 * Fitur: 
 * - Data Pasien Lengkap (NIK, Alamat, Multi-Gejala)
 * - Sistem Poli & Harga Jasa Otomatis
 * - Transaksi Detail (Jasa + Obat-obatan)
 * - Edit Data Selektif
 * - File I/O dengan format delimiter '|'
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib> 
#include <thread>  
#include <chrono>

using namespace std;

// --- DEFINISI WARNA (ANSI CODE) ---
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"

// --- FUNGSI BANTUAN UI & STRING ---
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pauseScreen() {
    cout << "\n" << CYAN << "Tekan Enter untuk kembali..." << RESET;
    cin.ignore(); cin.get();
}

void loadingAnimation(string message) {
    cout << YELLOW << message << RESET;
    for(int i=0; i<3; i++) {
        cout << ".";
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(200));
    }
    cout << "\n";
}

// Memotong string jika terlalu panjang untuk tabel
string truncate(string str, size_t width) {
    if (str.length() > width)
        return str.substr(0, width - 3) + "...";
    return str;
}

// Menggabungkan vector string menjadi satu string dipisah koma
string joinVector(const vector<string>& vec, string delimiter) {
    string result = "";
    for (size_t i = 0; i < vec.size(); i++) {
        result += vec[i];
        if (i < vec.size() - 1) result += delimiter;
    }
    return result;
}

void printHeader() {
    clearScreen();
    cout << BOLD << CYAN;
    cout << "==============================================================================\n";
    cout << "   _  __ _  _         _  _     ___        _           _   \n";
    cout << "  | |/ /| |(_) _ _   (_)| |__ / __| ___  | |_   __ _ | |_ \n";
    cout << "  | ' < | || || ' \\  | || / / \\__ \\/ -_) | ' \\ / _` ||  _|\n";
    cout << "  |_|\\_\\|_||_||_||_| |_||_\\_\\ |___/\\___| |_||_|\\__,_| \\__|\n";
    cout << "                                                          \n";
    cout << "       SISTEM MANAJEMEN DATA PASIEN, POLI & TRANSAKSI     \n";
    cout << "==============================================================================\n" << RESET;
}

// --- CLASS PASIEN ---
class Pasien {
public:
    string nik;
    string nama;
    string alamat;
    int umur;
    vector<string> gejala; // Multi gejala
    string poli;           // Poli tujuan (Umum/Gigi/dll)
    double biayaJasa;      // Biaya dokter berdasarkan Poli
    vector<string> obat;   // Daftar obat yang diberikan
    double biayaObat;      // Total harga obat
    double totalBiaya;     // Jasa + Obat

    // Constructor Default
    Pasien() {}

    // Constructor Parameter Lengkap
    Pasien(string nik, string n, string almt, int u, vector<string> g, string p, double bj, vector<string> o, double bo, double tb) {
        this->nik = nik;
        nama = n;
        alamat = almt;
        umur = u;
        gejala = g;
        poli = p;
        biayaJasa = bj;
        obat = o;
        biayaObat = bo;
        totalBiaya = tb;
    }

    // Format penyimpanan file menggunakan '|' sebagai pemisah utama dan ',' untuk list
    string toFileFormat() {
        string strGejala = joinVector(gejala, ",");
        string strObat = joinVector(obat, ",");
        if (strObat.empty()) strObat = "-";
        
        stringstream ss;
        ss << nik << "|" << nama << "|" << alamat << "|" << umur << "|" 
           << strGejala << "|" << poli << "|" << (size_t)biayaJasa << "|" 
           << strObat << "|" << (size_t)biayaObat << "|" << (size_t)totalBiaya;
        return ss.str();
    }
};

// --- CLASS KLINIK ---
class Klinik {
private:
    vector<Pasien> daftarPasien;

    // Helper: Mendapatkan harga dasar poli
    double getHargaPoli(string namaPoli) {
        if (namaPoli == "Poli Umum") return 50000;
        if (namaPoli == "Poli Gigi") return 100000;
        if (namaPoli == "Poli Anak") return 75000;
        if (namaPoli == "Poli THT") return 120000;
        return 0;
    }

public:
    void tambahPasien() {
        printHeader();
        cout << YELLOW << "[ MENU: PENDAFTARAN PASIEN BARU ]" << RESET << "\n\n";
        
        string nik, nama, alamat;
        int umur;
        vector<string> listGejala;
        
        cout << "NIK (KTP)        : "; cin >> nik;
        cout << "Nama Lengkap     : "; cin.ignore(); getline(cin, nama);
        cout << "Alamat           : "; getline(cin, alamat);
        cout << "Umur             : "; cin >> umur;

        // Input Multi Gejala
        cout << "\n" << CYAN << "--- Input Gejala/Keluhan ---" << RESET << endl;
        cout << "(Ketik 'ok' jika selesai input gejala)\n";
        int count = 1;
        while (true) {
            string g;
            cout << "Gejala ke-" << count << " : ";
            if (count == 1) cin.ignore(); // handle buffer
            getline(cin, g);
            
            if (g == "ok" || g == "OK" || g == "Ok") break;
            if (!g.empty()) {
                listGejala.push_back(g);
                count++;
            }
        }

        // Pemilihan Poli
        cout << "\n" << CYAN << "--- Pilih Poli Tujuan ---" << RESET << endl;
        cout << "1. Poli Umum (Rp 50.000)\n";
        cout << "2. Poli Gigi (Rp 100.000)\n";
        cout << "3. Poli Anak (Rp 75.000)\n";
        cout << "4. Poli THT  (Rp 120.000)\n";
        cout << "Pilihan Poli [1-4]: ";
        int pilPoli; cin >> pilPoli;

        string namaPoli = "Poli Umum"; // Default
        if (pilPoli == 2) namaPoli = "Poli Gigi";
        else if (pilPoli == 3) namaPoli = "Poli Anak";
        else if (pilPoli == 4) namaPoli = "Poli THT";

        double biayaJasa = getHargaPoli(namaPoli);

        // Buat objek pasien (Obat & Biaya Obat masih kosong/0)
        vector<string> obatKosong;
        Pasien p(nik, nama, alamat, umur, listGejala, namaPoli, biayaJasa, obatKosong, 0, biayaJasa);
        
        loadingAnimation("Menyimpan data ke sistem");
        daftarPasien.push_back(p);
        cout << GREEN << ">> Sukses: Pasien terdaftar di " << namaPoli << "!" << RESET << endl;
        pauseScreen();
    }

    void lihatData(bool pause = true) {
        if (pause) printHeader();
        cout << YELLOW << "[ DATA PASIEN KLINIK ]" << RESET << "\n";
        
        // Header Tabel
        cout << BOLD << "--------------------------------------------------------------------------------------------------\n";
        cout << "| No | " << left << setw(16) << "NIK" 
             << "| " << setw(18) << "Nama" 
             << "| " << setw(10) << "Poli" 
             << "| " << setw(20) << "Gejala Utama" 
             << "| " << setw(12) << "Tagihan" << " |\n";
        cout << "--------------------------------------------------------------------------------------------------\n" << RESET;

        if (daftarPasien.empty()) {
            cout << RED << "                            Data masih kosong.\n" << RESET;
        } else {
            for (size_t i = 0; i < daftarPasien.size(); i++) {
                // Ambil gejala pertama saja untuk tampilan tabel agar rapi
                string gejalaUtama = daftarPasien[i].gejala.empty() ? "-" : daftarPasien[i].gejala[0];
                if (daftarPasien[i].gejala.size() > 1) gejalaUtama += " (+)";

                cout << "| " << left << setw(2) << (i + 1) << " | " 
                     << setw(16) << daftarPasien[i].nik 
                     << "| " << setw(18) << truncate(daftarPasien[i].nama, 18) 
                     << "| " << setw(10) << daftarPasien[i].poli 
                     << "| " << setw(20) << truncate(gejalaUtama, 20) 
                     << "| Rp " << setw(9) << fixed << setprecision(0) << daftarPasien[i].totalBiaya << " |\n";
            }
        }
        cout << "--------------------------------------------------------------------------------------------------\n";
        
        if (pause) pauseScreen();
    }

    void detailPasien(int idx) {
        Pasien p = daftarPasien[idx];
        cout << "\n" << CYAN << "--- Detail Pasien ---" << RESET << endl;
        cout << "NIK       : " << p.nik << endl;
        cout << "Nama      : " << p.nama << endl;
        cout << "Umur      : " << p.umur << " Tahun" << endl;
        cout << "Alamat    : " << p.alamat << endl;
        cout << "Poli      : " << p.poli << endl;
        cout << "Gejala    : " << joinVector(p.gejala, ", ") << endl;
        cout << "Obat      : " << (p.obat.empty() ? "-" : joinVector(p.obat, ", ")) << endl;
        cout << "Biaya     : Jasa (Rp " << (size_t)p.biayaJasa << ") + Obat (Rp " << (size_t)p.biayaObat << ")" << endl;
        cout << "Total     : Rp " << (size_t)p.totalBiaya << endl;
    }

    void editPasien() {
        printHeader();
        cout << YELLOW << "[ MENU: EDIT DATA SELEKTIF ]" << RESET << "\n\n";
        lihatData(false);

        if (daftarPasien.empty()) { pauseScreen(); return; }

        int nomor;
        cout << "\nNomor Pasien yang diedit: "; cin >> nomor;

        if (nomor > 0 && nomor <= daftarPasien.size()) {
            int idx = nomor - 1;
            int pilihanEdit;
            
            do {
                clearScreen();
                detailPasien(idx); // Tampilkan detail saat ini
                
                cout << "\n" << MAGENTA << "--- Pilih Bagian yang Ingin Diedit ---" << RESET << endl;
                cout << "1. Edit Nama\n";
                cout << "2. Edit Umur\n";
                cout << "3. Edit Alamat\n";
                cout << "4. Edit Keluhan/Gejala (Reset)\n";
                cout << "0. Selesai Edit / Kembali\n";
                cout << "Pilihan: "; cin >> pilihanEdit;

                switch(pilihanEdit) {
                    case 1: 
                        cout << "Nama Baru: "; cin.ignore(); getline(cin, daftarPasien[idx].nama); 
                        break;
                    case 2: 
                        cout << "Umur Baru: "; cin >> daftarPasien[idx].umur; 
                        break;
                    case 3: 
                        cout << "Alamat Baru: "; cin.ignore(); getline(cin, daftarPasien[idx].alamat); 
                        break;
                    case 4: {
                        cout << "Masukkan Gejala Baru (Ketik 'ok' untuk stop):\n";
                        daftarPasien[idx].gejala.clear();
                        int c = 1; cin.ignore();
                        while(true) {
                            string g; cout << "Gejala " << c++ << ": "; getline(cin, g);
                            if (g == "ok" || g == "OK") break;
                            daftarPasien[idx].gejala.push_back(g);
                        }
                        break;
                    }
                    case 0: break;
                    default: cout << RED << "Pilihan tidak valid!\n" << RESET;
                }
                if (pilihanEdit != 0) cout << GREEN << ">> Data tersimpan sementara.\n" << RESET;
                
            } while (pilihanEdit != 0);
            
            cout << GREEN << ">> Edit Selesai.\n" << RESET;

        } else {
            cout << RED << ">> Error: Nomor tidak valid!" << RESET << endl;
        }
        pauseScreen();
    }

    void hapusPasien() {
        printHeader();
        cout << YELLOW << "[ MENU: HAPUS DATA ]" << RESET << "\n\n";
        lihatData(false);

        if (daftarPasien.empty()) { pauseScreen(); return; }

        int nomor;
        cout << "\nNomor Pasien yang DIHAPUS: "; cin >> nomor;

        if (nomor > 0 && nomor <= daftarPasien.size()) {
            char yakin;
            cout << RED << "Yakin hapus data " << daftarPasien[nomor-1].nama << "? (y/n): " << RESET;
            cin >> yakin;

            if (yakin == 'y' || yakin == 'Y') {
                daftarPasien.erase(daftarPasien.begin() + (nomor - 1));
                cout << GREEN << ">> Sukses: Data terhapus!" << RESET << endl;
            }
        }
        pauseScreen();
    }

    void transaksi() {
        printHeader();
        cout << YELLOW << "[ MENU: KASIR & FARMASI ]" << RESET << "\n\n";
        lihatData(false);

        if (daftarPasien.empty()) { pauseScreen(); return; }

        int nomor;
        cout << "\nPilih Nomor Pasien: "; cin >> nomor;

        if (nomor > 0 && nomor <= daftarPasien.size()) {
            int idx = nomor - 1;
            Pasien &p = daftarPasien[idx];

            clearScreen();
            cout << BOLD << "--- PROSES PEMBAYARAN: " << p.nama << " (" << p.poli << ") ---" << RESET << endl;
            cout << "Biaya Jasa Dokter: Rp " << (size_t)p.biayaJasa << endl;

            cout << "\n" << CYAN << "--- Input Obat-obatan ---" << RESET << endl;
            cout << "(Masukkan nama obat dan harga. Ketik 'selesai' di nama obat untuk stop)\n";
            
            double totalHargaObat = 0;
            p.obat.clear(); // Reset list obat sebelumnya jika ada re-transaksi

            cin.ignore();
            while(true) {
                string namaObat;
                double hargaObat;

                cout << "Nama Obat : "; getline(cin, namaObat);
                if (namaObat == "selesai" || namaObat == "Selesai") break;

                cout << "Harga     : Rp "; cin >> hargaObat; cin.ignore();
                
                totalHargaObat += hargaObat;
                p.obat.push_back(namaObat);
                cout << GREEN << "  + Ditambahkan: " << namaObat << endl << RESET;
            }

            // Hitung Total Akhir
            p.biayaObat = totalHargaObat;
            p.totalBiaya = p.biayaJasa + p.biayaObat;

            loadingAnimation("Mencetak Struk");
            
            cout << "\n" << BOLD << GREEN << "========================================" << endl;
            cout << "            STRUK PEMBAYARAN            " << endl;
            cout << "========================================" << endl;
            cout << " Pasien      : " << p.nama << endl;
            cout << " Poli        : " << p.poli << endl;
            cout << " Jasa Dokter : Rp " << (size_t)p.biayaJasa << endl;
            cout << " Total Obat  : Rp " << (size_t)p.biayaObat << endl;
            cout << " List Obat   : " << joinVector(p.obat, ", ") << endl;
            cout << "----------------------------------------" << endl;
            cout << " TOTAL BAYAR : Rp " << (size_t)p.totalBiaya << endl;
            cout << "========================================" << RESET << endl;

        } else {
            cout << RED << ">> Error: Nomor tidak valid!" << RESET << endl;
        }
        pauseScreen();
    }

    void loadFromFile() {
        ifstream file("database_klinik.txt");
        string line;
        
        daftarPasien.clear(); // Bersihkan memori sebelum load
        
        if (file.is_open()) {
            while (getline(file, line)) {
                stringstream ss(line);
                string segment;
                vector<string> data;

                // Split by '|'
                while(getline(ss, segment, '|')) {
                    data.push_back(segment);
                }

                if (data.size() >= 10) { // Pastikan format valid
                    string nik = data[0];
                    string nama = data[1];
                    string alamat = data[2];
                    int umur = stoi(data[3]);
                    
                    // Parse Gejala (split by comma)
                    vector<string> gejala;
                    stringstream ssGejala(data[4]);
                    string g;
                    while(getline(ssGejala, g, ',')) gejala.push_back(g);

                    string poli = data[5];
                    double jasa = stod(data[6]);
                    
                    // Parse Obat
                    vector<string> obat;
                    stringstream ssObat(data[7]);
                    string o;
                    while(getline(ssObat, o, ',')) if(o != "-") obat.push_back(o);

                    double biobat = stod(data[8]);
                    double total = stod(data[9]);

                    daftarPasien.push_back(Pasien(nik, nama, alamat, umur, gejala, poli, jasa, obat, biobat, total));
                }
            }
            file.close();
        }
    }

    void saveToFile() {
        ofstream file("database_klinik.txt");
        if (file.is_open()) {
            for (auto& p : daftarPasien) file << p.toFileFormat() << endl;
            file.close();
            loadingAnimation("Menyimpan ke database_klinik.txt");
            cout << GREEN << ">> Data berhasil diamankan!" << RESET << endl;
        }
    }
};

// --- MAIN MENU ---
int main() {
    Klinik klinik;
    klinik.loadFromFile();
    int pilihan;

    do {
        printHeader();
        cout << " 1. " << BOLD << "Pendaftaran Pasien Baru" << RESET << endl;
        cout << " 2. " << BOLD << "Lihat Antrean & Data" << RESET << endl;
        cout << " 3. " << BOLD << "Edit Data (Selektif)" << RESET << endl;
        cout << " 4. " << BOLD << "Hapus Data Pasien" << RESET << endl;
        cout << " 5. " << BOLD << "Kasir & Farmasi" << RESET << endl;
        cout << " 6. " << BOLD << "Simpan & Keluar" << RESET << endl;
        cout << "==============================================================================\n";
        cout << " Pilihan [1-6]: "; cin >> pilihan;

        switch (pilihan) {
            case 1: klinik.tambahPasien(); break;
            case 2: klinik.lihatData(); break;
            case 3: klinik.editPasien(); break;
            case 4: klinik.hapusPasien(); break;
            case 5: klinik.transaksi(); break;
            case 6: klinik.saveToFile(); break;
            default: 
                cout << RED << ">> Pilihan salah!" << RESET; 
                this_thread::sleep_for(chrono::seconds(1));
        }
    } while (pilihan != 6);

    cout << "\n" << CYAN << "Terima kasih telah menggunakan Sistem Klinik." << RESET << endl;
    return 0;
}
