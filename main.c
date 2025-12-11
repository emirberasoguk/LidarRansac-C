#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef M_PI // pi sayısını değiştirilemez tek bir değişkene atıyoruz
#define M_PI 3.14159265358979323846
#endif 

// ----- veri yapıları

typedef struct { // noktanın kartezyen koordinat değerlerini tututuyor
    double x;
    double y;
} Point;

// RANSAC'ın bulduğu doğruları ax + by + c = 0 şeklinde saklıyor
typedef struct {
    double a;
    double b;
    double c;
    int inlier_sayaci; // doğruyu oluşturan nokta sayısı
} dogru_denklemi;

typedef struct { // toml dosya içeriğinin hem ham hemde filtrelenmiş verilerini içerir
    double angle_min;
    double angle_max;
    double angle_increment;
    double range_min;
    double range_max;
    double* ranges;
    int num_ranges;
    Point* points;
    int points_sayisi;
} LidarData;

typedef struct { // doğruyu oluşturan noktaların doğrudaki konumu ve sırasını tutuyor
    Point p; // konumu
    double t; // sırası
} PointWithParam;

// ----- fonksiyonlar 

char* bosluk_temizleyici(char* metin) { // okunan verinin başındaki ve sonundaki boşlukları temizleer böylece strtok strcmp gibi karşılaştırma fonksiyonlarında hata almayız
    char* son;
    while (isspace((unsigned char)*metin)) metin++;
    if (*metin == 0) return metin;
    son = metin + strlen(metin) - 1;
    while (son > metin && isspace((unsigned char)*son)) son--;
    *(son + 1) = '\0';
    return metin;
}

int dosya_indir(const char* url, const char* hedef_dosya) {
    char komut[2048];
    // -s: silent, -L: follow redirects, -o: output file
    // Güvenlik: URL ve dosya adı tırnak içine alındı
    sprintf(komut, "curl -s -L \"%s\" -o \"%s\"", url, hedef_dosya);
    printf("Veri indiriliyor: %s -> %s\n", url, hedef_dosya);
    
    int sonuc = system(komut);
    if (sonuc != 0) {
        printf("HATA: İndirme başarısız! (curl hatası veya internet yok)\n");
        return 0;
    }
    return 1;
}

int toml_okuma_yazma(const char* dosya_adi, LidarData* data) {
    FILE* okunan_dosya = fopen(dosya_adi, "r");
    if (!okunan_dosya) {
        printf("HATA: '%s' dosyası açılamadı!\n", dosya_adi);
        return 0;
    }

    char okunan_satir[2048];
    int ranges_mi = 0;
    double* ranges_tutucu = NULL;
    int ranges_kapasite = 0;
    data->num_ranges = 0;

    while (fgets(okunan_satir, sizeof(okunan_satir), okunan_dosya)) { // satır okuyabildiği sürece okuyor
        char* satir = bosluk_temizleyici(okunan_satir);
        if (satir[0] == '#' || satir[0] == '\0' || satir[0] == '[') {  // gereksiz satırları atlıyoruz ; [header] / [scan], boş satırlar(\0), # yorum satırları
            continue;
        }

        if (ranges_mi) {
            if (strchr(satir, ']')) { 
                ranges_mi = 0; 
            }
            char* token = strtok(satir, ", \t\n[]");
            while (token) { // token olduğu sürece devam et
                if (data->num_ranges >= ranges_kapasite) { 
                    ranges_kapasite = (ranges_kapasite == 0) ? 100 : ranges_kapasite * 2;
                    double* kontrol = realloc(ranges_tutucu, ranges_kapasite * sizeof(double));
                    if(!kontrol)
                    {
                        printf("HATA: 'ranges' için bellek ayrılmadı!\n");
                        free (ranges_tutucu);
                        fclose(okunan_dosya);
                        return 0;
                    }
                    ranges_tutucu= kontrol;
                }
                ranges_tutucu[data->num_ranges++] = atof(token);
                token = strtok(NULL, ", \t\n[]");
            }
        } else {
            char* anahtar = strtok(satir, "="); // = kısmına kadar olan kısmı anahtar yapar
            char* deger = strtok(NULL, "="); // = den sonrasını değer yapar
            if (anahtar && deger) { // anahtar ve değer var mı  
                anahtar = bosluk_temizleyici(anahtar);
                deger = bosluk_temizleyici(deger);
                if (strcmp(anahtar, "angle_min") == 0) data->angle_min = atof(deger);
                else if (strcmp(anahtar, "angle_max") == 0) data->angle_max = atof(deger);
                else if (strcmp(anahtar, "angle_increment") == 0) data->angle_increment = atof(deger);
                else if (strcmp(anahtar, "range_min") == 0) data->range_min = atof(deger);
                else if (strcmp(anahtar, "range_max") == 0) data->range_max = atof(deger);
                else if (strcmp(anahtar, "ranges") == 0) {
                    ranges_mi = 1;
                    // bu kısım ranges = [ ile aynı satırda değerler varsa onları alır eğer yoksa ana ranges değerlerinin işlendiği yerde devam eder
                    // böyle yapmamızın sebebi bir kere ranges = [ olan satırı işledikten sonra döngününn ilerlemesi ve eğer aynı satırda değer varsa onları da işlemesi
                    char* token = strtok(deger, ", \t\n[]"); //ranges değerleri olan sayıları ayıran her türlü şeyi belirliyor ve sayıları birbirinden ayırıyor(parçalıyor) 
                    while (token) {
                        if (data->num_ranges >= ranges_kapasite) {
                            ranges_kapasite = (ranges_kapasite == 0) ? 100 : ranges_kapasite * 2;
                                double* kontrol = realloc(ranges_tutucu, ranges_kapasite * sizeof(double));
                        if (!kontrol) {
                                printf("HATA: 'ranges' için bellek ayrılmadı!\n");
                                free(ranges_tutucu);
                                fclose(okunan_dosya);
                                return 0;
                                }
                        ranges_tutucu = kontrol;
                        }
                        ranges_tutucu[data->num_ranges++] = atof(token);
                        token = strtok(NULL, ", \t\n[]");
                    }
                }
            }
        }
    }
    fclose(okunan_dosya);
    data->ranges = ranges_tutucu;
    printf("Dosya manuel olarak okundu. %d adet 'range' degeri bulundu.\n", data->num_ranges);
    return 1;
}

void veri_filtreleme_ve_donusturme(LidarData* data) { // toml dosyasından elde edilen range verilerini hatalı değerlerden arındırır ve kartezyen kordinat sistemine çevirir
    Point* gecici_noktalar = malloc(data->num_ranges * sizeof(Point));
    if (gecici_noktalar == NULL) {
        printf("Hata: Bellek ayrılamadı!\n");
        return;
    }
    int gecerli_nokta_sayisi = 0;
    for (int i = 0; i < data->num_ranges; i++) {
        double range_degeri = data->ranges[i];
        if (range_degeri > data->range_min && range_degeri < data->range_max) { // burada filtreliyor işte
            double tarama_acisi = data->angle_min + i * data->angle_increment;
            gecici_noktalar[gecerli_nokta_sayisi].x = range_degeri * cos(tarama_acisi);
            gecici_noktalar[gecerli_nokta_sayisi].y = range_degeri * sin(tarama_acisi);
            gecerli_nokta_sayisi++;
        }
    }
    data->points_sayisi = gecerli_nokta_sayisi;
    if (gecerli_nokta_sayisi > 0) {
        data->points = malloc(data->points_sayisi * sizeof(Point)); // tam ihtiyacımız kadar alan kullanacakk olan yeni nokta tutucumuz
        if (data->points == NULL) {
            printf("Hata: Sonuçlar için bellek ayrılamadı!\n");
            free(gecici_noktalar);
            return;
        }
        for (int i = 0; i < data->points_sayisi; i++) {
            data->points[i] = gecici_noktalar[i];
        }
    } else {
        data->points = NULL;
    }
    free(gecici_noktalar);
    printf("Filtreleme ve donusumm tamamlandi. %d adet gecerli nokta bulundu.\n", data->points_sayisi);
}

static dogru_denklemi iki_noktadan_dogru_bul(Point p1, Point p2) { // iki noka alır ve bunların üzerinden geçeb doğrunun denklemini bulur
    dogru_denklemi line;
    line.a = p2.y - p1.y;
    line.b = p1.x - p2.x;
    line.c = -line.a * p1.x - line.b * p1.y;
    line.inlier_sayaci = 0;

    double norm = sqrt(line.a * line.a + line.b * line.b); // denklemi birim haline çevirmek için bunu kullandık süre optimizasyonu için
    if (norm > 1e-9) {
        line.a /= norm;
        line.b /= norm;
        line.c /= norm;
    }
    return line;
}

static double noktanin_dogruya_uzakligi(dogru_denklemi line, Point p) { // doğru denklemini bulurken denklemin normalini aldığımız için paydadaki sqrt işlemine gerek kalmadı
    return fabs(line.a * p.x + line.b * p.y + line.c); // noktanin doğruya uzaklığını buluyor 
}

int xe_gore_karsilastir(const void* a, const void* b) { // doğrudan seçilmiş iki noktanın x değerlerini kıyaslar
    Point* p1 = (Point*)a;
    Point* p2 = (Point*)b;
    if (p1->x < p2->x) return -1;
    if (p1->x > p2->x) return 1;
    return 0;
}

int parametreye_gore_karsilastir(const void* a, const void* b) { // sadece x ile sıralasaydık doğru dike yakın olduğunda hatalı çıktı alacaktık bunun önüne geçmek için bu fonksiyonu yazdık
    PointWithParam* p1 = (PointWithParam*)a;
    PointWithParam* p2 = (PointWithParam*)b;
    if (p1->t < p2->t) return -1;
    if (p1->t > p2->t) return 1;
    return 0;
}

dogru_denklemi* ransac_ile_dogru_bulma(Point* tum_noktalar, int nokta_sayisi, double mesafe_esigi, int min_nokta_sayisi, int tekrar_sayisi, int* bulunan_dogru_sayisi_adresi) { // ransac algoritması
    printf("RANSAC: %d nokta uzerinde %d iterasyon ile calisiliyor...\n", nokta_sayisi, tekrar_sayisi);
    *bulunan_dogru_sayisi_adresi = 0;
    if (nokta_sayisi < min_nokta_sayisi) {
        return NULL;
    }

    int* nokta_kullanildi_mi = (int*)calloc(nokta_sayisi, sizeof(int));
    dogru_denklemi* gecici_dogru = NULL;
    int dogru_listesi_kapasitesi = 0;
    int kalan_nokta_sayisi = nokta_sayisi;

    while (kalan_nokta_sayisi >= min_nokta_sayisi) 
    {
        dogru_denklemi en_iyi_dogru_modeli;
        int en_fazla_destekci_sayisi = 0;
        int* en_iyi_destekci_indeksleri = (int*)malloc(nokta_sayisi * sizeof(int));

        // RANSAC: En iyi doğruyu bul
        for (int i = 0; i < tekrar_sayisi; i++) 
        {
            int index1, index2;
            int deneme_sayaci = 0;
            
            // İki rastgele nokta seç (kullanılmamış olanlar)
            do { 
                index1 = rand() % nokta_sayisi; 
                deneme_sayaci++; 
            } while (nokta_kullanildi_mi[index1] == 1 && deneme_sayaci < nokta_sayisi * 5);
            
            if(deneme_sayaci >= nokta_sayisi * 5) break;
            
            deneme_sayaci = 0;
            do { 
                index2 = rand() % nokta_sayisi; 
                deneme_sayaci++; 
            } while ((index1 == index2 || nokta_kullanildi_mi[index2] == 1) && deneme_sayaci < nokta_sayisi * 5);
            
            if(deneme_sayaci >= nokta_sayisi * 5) break;

            dogru_denklemi mevcut_aday_dogru = iki_noktadan_dogru_bul( tum_noktalar[index1], tum_noktalar[index2]);
            
            int mevcut_destekci_sayisi = 0;
            int* mevcut_destekci_indeksleri = (int*)malloc(nokta_sayisi * sizeof(int));
            
            for (int j = 0; j < nokta_sayisi; j++) 
            {
                if (nokta_kullanildi_mi[j] == 0 && 
                    noktanin_dogruya_uzakligi(mevcut_aday_dogru, tum_noktalar[j]) < mesafe_esigi) {
                    mevcut_destekci_indeksleri[mevcut_destekci_sayisi++] = j;
                }
            }

            if (mevcut_destekci_sayisi > en_fazla_destekci_sayisi) 
            {
                en_fazla_destekci_sayisi = mevcut_destekci_sayisi;
                
                en_iyi_dogru_modeli = mevcut_aday_dogru;
            

                memcpy(en_iyi_destekci_indeksleri, mevcut_destekci_indeksleri, mevcut_destekci_sayisi * sizeof(int));
            

            }


            free(mevcut_destekci_indeksleri);
        }

        if (en_fazla_destekci_sayisi < min_nokta_sayisi) {
            printf("  RANSAC: Yeterli inlier bulunamadi (%d < %d). Durduruluyor.\n", 
                   en_fazla_destekci_sayisi, min_nokta_sayisi);
            free(en_iyi_destekci_indeksleri);
            break;
        }

        Point* siralanacak_noktalar = (Point*)malloc(en_fazla_destekci_sayisi * sizeof(Point));
        for (int i = 0; i < en_fazla_destekci_sayisi; i++) {
            siralanacak_noktalar[i] = tum_noktalar[en_iyi_destekci_indeksleri[i]];
        }
        
        PointWithParam* siralanacak_parametre = (PointWithParam*)malloc(en_fazla_destekci_sayisi * sizeof(PointWithParam));
        for (int i = 0; i < en_fazla_destekci_sayisi; i++) {
            siralanacak_parametre[i].p = siralanacak_noktalar[i];
            siralanacak_parametre[i].t = (-en_iyi_dogru_modeli.b * siralanacak_noktalar[i].x + 
                          en_iyi_dogru_modeli.a * siralanacak_noktalar[i].y);
        }
        
        qsort(siralanacak_parametre, en_fazla_destekci_sayisi, sizeof(PointWithParam), parametreye_gore_karsilastir);
        
        for (int i = 0; i < en_fazla_destekci_sayisi; i++) 
        {
            siralanacak_noktalar[i] = siralanacak_parametre[i].p;
        }
        free(siralanacak_parametre);
        double maksimum_bosluk_karesi = 0.15 * 0.15;
        int alt_gurub_baslangici = 0;

        for (int i = 0; i < en_fazla_destekci_sayisi - 1; i++) 
        {
            double dx = siralanacak_noktalar[i].x - siralanacak_noktalar[i+1].x;
            double dy = siralanacak_noktalar[i].y - siralanacak_noktalar[i+1].y;
            double mesafenin_karesi = dx*dx + dy*dy;

            if (mesafenin_karesi > maksimum_bosluk_karesi)
            {
                int alt_gurub_boyutu = (i + 1) - alt_gurub_baslangici;
                
                if (alt_gurub_boyutu >= min_nokta_sayisi) {
                    printf("  Alt grup: %d nokta.\n", alt_gurub_boyutu);
                    
                    dogru_denklemi alt_gurub_dogrusu = iki_noktadan_dogru_bul(siralanacak_noktalar[alt_gurub_baslangici],siralanacak_noktalar[i]);
                    
                    int alt_gurub_nokta_sayisi_sonucu = 0;
                    for (int k = alt_gurub_baslangici; k <= i; k++) {
                        if (noktanin_dogruya_uzakligi(alt_gurub_dogrusu, siralanacak_noktalar[k]) < mesafe_esigi) {
                            alt_gurub_nokta_sayisi_sonucu++;
                        }
                    }
                    
                    if (alt_gurub_nokta_sayisi_sonucu >= min_nokta_sayisi) {
                        if (*bulunan_dogru_sayisi_adresi >= dogru_listesi_kapasitesi) 
                        {
                            dogru_listesi_kapasitesi = (dogru_listesi_kapasitesi == 0) ? 10 : dogru_listesi_kapasitesi * 2;
                            dogru_denklemi* kontrol=(dogru_denklemi*)realloc(gecici_dogru, dogru_listesi_kapasitesi * sizeof(dogru_denklemi));
                        if(kontrol == NULL){
                            printf("HATA: RANSAC sonuc dizisi için bellek yeniden ayrılamadı!\n ");
                            free(gecici_dogru);
                            return NULL;
                        }    
                        gecici_dogru = kontrol;
                        }
                        alt_gurub_dogrusu.inlier_sayaci = alt_gurub_nokta_sayisi_sonucu;
                        gecici_dogru[*bulunan_dogru_sayisi_adresi] = alt_gurub_dogrusu;
                        (*bulunan_dogru_sayisi_adresi)++;
                    }
                }
                alt_gurub_baslangici = i + 1;
            }
        }
        
        int son_alt_gurub_boyutu = en_fazla_destekci_sayisi - alt_gurub_baslangici;
        if (son_alt_gurub_boyutu >= min_nokta_sayisi) {
            dogru_denklemi alt_gurub_dogrusu = iki_noktadan_dogru_bul(
                siralanacak_noktalar[alt_gurub_baslangici],
                siralanacak_noktalar[en_fazla_destekci_sayisi - 1]
            );
            
            int final_count = 0;
            for (int k = alt_gurub_baslangici; k < en_fazla_destekci_sayisi; k++) {
                if (noktanin_dogruya_uzakligi(alt_gurub_dogrusu, siralanacak_noktalar[k]) < mesafe_esigi) {
                    final_count++;
                }
            }
            
            if (final_count >= min_nokta_sayisi) {
                if (*bulunan_dogru_sayisi_adresi >= dogru_listesi_kapasitesi) {
                    dogru_listesi_kapasitesi = (dogru_listesi_kapasitesi == 0) ? 10 : dogru_listesi_kapasitesi * 2;
                    dogru_denklemi* kontrol = (dogru_denklemi*)realloc(gecici_dogru, dogru_listesi_kapasitesi * sizeof(dogru_denklemi));
                if(kontrol == NULL){
                    printf("HATA: RANSAC sonuç dizisi bellek yeniden ayrılamadı!\n");
                    free(gecici_dogru);
                    return NULL;
                }
                gecici_dogru = kontrol;
                }
                alt_gurub_dogrusu.inlier_sayaci = final_count;
                gecici_dogru[*bulunan_dogru_sayisi_adresi] = alt_gurub_dogrusu;
                (*bulunan_dogru_sayisi_adresi)++;
            }
        }
        
        free(siralanacak_noktalar);
        
        for (int i = 0; i < en_fazla_destekci_sayisi; i++) {
            nokta_kullanildi_mi[en_iyi_destekci_indeksleri[i]] = 1;
        }
        kalan_nokta_sayisi -= en_fazla_destekci_sayisi;
        printf("  Kalan noktalar: %d\n", kalan_nokta_sayisi);
        free(en_iyi_destekci_indeksleri);
    }

    free(nokta_kullanildi_mi);
    return gecici_dogru;
}

Point kesisim_noktasi_bul(dogru_denklemi l1, dogru_denklemi l2, int* basarili_mi) { // verilen iki doğrunun kesişim noktasını bulur
    Point p = {0, 0};
    // l1: a1x + b1y = -c1
    // l2: a2x + b2y = -c2
    double det = l1.a * l2.b - l2.a * l1.b; // Determinant (a1*b2 - a2*b1)

    if (fabs(det) < 1e-9) { // determinant 0'a çok yakınsa doğrular paralel olur
        *basarili_mi = 0;
        return p;
    }

    // cramer Kuralı ile x ve y'yi bulur
    p.x = ((-l1.c) * l2.b - (-l2.c) * l1.b) / det;
    p.y = (l1.a * (-l2.c) - l2.a * (-l1.c)) / det;
    *basarili_mi = 1;
    return p;
}

double dogrular_arasi_aci_bul(dogru_denklemi l1, dogru_denklemi l2) { // iki doğru arasındaki açıyı hesaplar
    double skaler_carpim = l1.a * l2.a + l1.b * l2.b;
    double uzunluk1 = sqrt(l1.a * l1.a + l1.b * l1.b);
    double uzunluk2 = sqrt(l2.a * l2.a + l2.b * l2.b);

    if (uzunluk1 == 0 || uzunluk2 == 0) return 0; // Geçersiz doğru (sıfır vektörü)

    double cos_theta = skaler_carpim / (uzunluk1 * uzunluk2);
    
    // tanım aralığı dışına taşmaması için
    if (cos_theta > 1.0) cos_theta = 1.0;
    if (cos_theta < -1.0) cos_theta = -1.0;
    
    double radyan_aci = acos(cos_theta);
    double derece_aci = radyan_aci * 180.0 / M_PI;

    if (derece_aci > 90.0) {
        return 180.0 - derece_aci;
    }
    return derece_aci;
}

void create_gnuplot_script(LidarData* data, dogru_denklemi* lines, int num_lines, Point* intersections, int num_intersections, Point closest_p, double closest_dist, double closest_ang, char* closest_lbl) {
    FILE* dosya_yazici, *fp_segment;
    double inlier_esigi = 0.15; // RANSAC'taki inlier eşik değeri

    // tüm geçerli noktaları noktalar.dat a yaz
    dosya_yazici = fopen("noktalar.dat", "w");
    if (dosya_yazici == NULL) {
        printf("HATA: noktalar.dat olusturulamadi.\n");
        return;
    }
    for (int i = 0; i < data->points_sayisi; i++) {
        fprintf(dosya_yazici, "%f %f\n", data->points[i].x, data->points[i].y);
    }
    fclose(dosya_yazici);

    // 2. sadece en yakın kesişimi dosyaya yaz ('hedef.dat')
    dosya_yazici = fopen("hedef.dat", "w");
    if (dosya_yazici == NULL) {
        printf("HATA: hedef.dat olusturulamadi.\n");
        return;
    }
    if (closest_dist > 0) { // Sadece geçerli bir hedef varsa
        fprintf(dosya_yazici, "%f %f\n", closest_p.x, closest_p.y);
    }
    fclose(dosya_yazici);
    
    // Gnuplot komut dosyasını oluştur ('cizim.gp')
    dosya_yazici = fopen("cizim.gp", "w");
    if (dosya_yazici == NULL) {
        printf("HATA: cizim.gp olusturulamadi.\n");
        return;
    }
    
    fprintf(dosya_yazici, "set terminal pngcairo size 1024,768 enhanced font 'Verdana,10'\n");
    fprintf(dosya_yazici, "set output 'proje_ciktisi.png'\n");
    fprintf(dosya_yazici, "set title 'LIDAR Veri Analizi - RANSAC ile Dogru Tespiti'\n");
    fprintf(dosya_yazici, "set xlabel 'X (metre)'\n");
    fprintf(dosya_yazici, "set ylabel 'Y (metre)'\n");
    fprintf(dosya_yazici, "set grid\n");
    fprintf(dosya_yazici, "set key outside right top\n"); 
    fprintf(dosya_yazici, "set object 1 circle at 0,0 size 0.05 fillcolor rgb 'red' fs empty border rgb 'black' lw 2 front\n");    

    if (closest_dist > 0) {
        fprintf(dosya_yazici, "set arrow 1 from 0,0 to %f,%f dashtype 2 linecolor rgb 'red' lw 3 head filled size 0.1,15,45 front\n", closest_p.x, closest_p.y);    
        fprintf(dosya_yazici, "set object 2 rectangle at %f,%f size 0.4,0.2 fillcolor rgb 'yellow' fs solid 1.0 border rgb 'red' front\n", closest_p.x / 4.0, closest_p.y / 4.0);
        fprintf(dosya_yazici, "set label 1 '%.2f m' at %f,%f textcolor rgb 'red' font ',12,bold' center front\n", closest_dist, closest_p.x / 4.0, closest_p.y / 4.0);

        double label_x_pos = closest_p.x;
        double label_y_pos = closest_p.y + 0.3; 
        
        fprintf(dosya_yazici, "set object 3 rectangle at %f,%f size 0.7,0.3 fillcolor rgb 'yellow' fs solid 1.0 border rgb 'black' front\n",
                label_x_pos, label_y_pos); 
        fprintf(dosya_yazici, "set label 2 '%.1f derece %s' at %f,%f textcolor rgb 'black' font ',12,bold' center front\n", closest_ang, closest_lbl, label_x_pos, label_y_pos);
    }
// bütün doğrular için inlier, segment dosyaları ve 'd1, d2' etiketlerini oluşturuyor
    for (int i = 0; i < num_lines; i++) {
        if (lines[i].inlier_sayaci <= 0) continue;

        Point* destekci_noktalar = (Point*)malloc(lines[i].inlier_sayaci * sizeof(Point));
        if (destekci_noktalar == NULL) continue;
        int current_inlier_idx = 0;
    
        for (int j = 0; j < data->points_sayisi; j++) {
            if (noktanin_dogruya_uzakligi(lines[i], data->points[j]) < inlier_esigi) {
                if (current_inlier_idx < lines[i].inlier_sayaci) {
                    destekci_noktalar[current_inlier_idx++] = data->points[j];
                }
            }
        }

        Point son1 = destekci_noktalar[0];
        Point son2 = destekci_noktalar[0];
        double max_dist_sq = 0;
        for (int k = 0; k < current_inlier_idx; k++) {
            for (int m = k + 1; m < current_inlier_idx; m++) {
                double dx = destekci_noktalar[k].x - destekci_noktalar[m].x;
                double dy = destekci_noktalar[k].y - destekci_noktalar[m].y;
                double dist_sq = dx*dx + dy*dy;
                if (dist_sq > max_dist_sq) {
                    max_dist_sq = dist_sq;
                    son1 = destekci_noktalar[k];
                    son2 = destekci_noktalar[m];
                }
            }
        }
        
        double segment_length = sqrt(max_dist_sq);
        double mid_x = (son1.x + son2.x) / 2.0;
        double mid_y = (son1.y + son2.y) / 2.0;
        char label_text[10];
        sprintf(label_text, "d%d", i+1);
    
        fprintf(dosya_yazici, "set label ' %s ' at %f,%f textcolor rgb 'black' font ',9,bold' center front boxed\n", 
        label_text, mid_x, mid_y);
 
        char filename_inliers[40];
        sprintf(filename_inliers, "dogru_inliers_%d.dat", i);
        fp_segment = fopen(filename_inliers, "w");
        if (fp_segment == NULL) {
            free(destekci_noktalar);
            continue;
        }
        for (int k = 0; k < current_inlier_idx; k++) {
            fprintf(fp_segment, "%f %f\n", destekci_noktalar[k].x, destekci_noktalar[k].y);
        }
        fclose(fp_segment);
        char filename_segment[40];
        sprintf(filename_segment, "dogru_segment_%d.dat", i);
        fp_segment = fopen(filename_segment, "w");
        if (fp_segment == NULL) {
            free(destekci_noktalar);
            continue;
        }
        fprintf(fp_segment, "%f %f\n", son1.x, son1.y);
        fprintf(fp_segment, "%f %f\n", son2.x, son2.y);
        fclose(fp_segment);

        free(destekci_noktalar);
    }
// çizim kısmı
    const char* line_color = "#006400";
    fprintf(dosya_yazici, "plot 'noktalar.dat' with points pointtype 7 pointsize 0.5 linecolor rgb '#c0c0c0' title 'Tum Noktalar', \\\n");
    fprintf(dosya_yazici, "     NaN with points pointtype 7 pointsize 1.5 linecolor rgb 'red' title 'Robot', \\\n");

    for (int i = 0; i < num_lines; i++) 
    {
        if (lines[i].inlier_sayaci > 0) { 
            char filename_inliers[40];
            char filename_segment[40];
            sprintf(filename_inliers, "dogru_inliers_%d.dat", i);
            sprintf(filename_segment, "dogru_segment_%d.dat", i);
            
            fprintf(dosya_yazici, "     '%s' with points pointtype 7 pointsize 0.7 linecolor rgb '%s' title 'd%d noktaları (%d nokta)', \\\n", filename_inliers, line_color, i+1, lines[i].inlier_sayaci);  
            fprintf(dosya_yazici, "     '%s' with lines linewidth 5 linecolor rgb '%s' title 'd%d', \\\n", filename_segment, line_color, i+1);
    
        }
    

    }


    
    fprintf(dosya_yazici, "     'hedef.dat' with points pointtype 2 pointsize 3 linewidth 3 linecolor rgb 'yellow' title '60+ Kesisim'\n");
    fclose(dosya_yazici);
    printf("Gnuplot: Veri dosyalari (noktalar.dat, dogru_X_..., hedef.dat) ve komut dosyasi (cizim.gp) olusturuldu.\n");
    printf("Otomatik calistiriliyor...\n");
    system("gnuplot cizim.gp");
    printf("Gorsel 'proje_ciktisi.png' olarak kaydedildi.\n");
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    const char* islenecek_dosya = "scan.toml"; // Varsayılan dosya
    char indirilen_dosya_adi[] = "indirilen_veri.toml"; // URL indirmesi için geçici ad

    // Argüman kontrolü
    if (argc > 1) {
        char* arg = argv[1];
        if (strncmp(arg, "http://", 7) == 0 || strncmp(arg, "https://", 8) == 0) {
            // URL verildi, indir
            if (dosya_indir(arg, indirilen_dosya_adi)) {
                islenecek_dosya = indirilen_dosya_adi;
            } else {
                return 1; // İndirme hatası
            }
        } else {
            // Yerel dosya yolu verildi
            islenecek_dosya = arg;
        }
    } else {
        printf("Bilgi: Veri dosyası belirtilmedi, varsayılan '%s' aranıyor.\n", islenecek_dosya);
        printf("Kullanım: %s <dosya_yolu> veya %s <url>\n\n", argv[0], argv[0]);
    }
  
    printf("--- LIDAR Veri Analiz Programi Başlatiliyor ---\n");
    printf("İşlenen dosya: %s\n", islenecek_dosya);

    LidarData tarama_verisi = {0}; 
    
    if (!toml_okuma_yazma(islenecek_dosya, &tarama_verisi)) {
        printf("Kritik hata: '%s' dosyası okunamadı veya bulunamadı.\n", islenecek_dosya);
        return 1;
    }

 /*TEST KODU----------------------------------------------
    if(!toml_okuma_yazma("scan.toml",&tarama_verisi)){
        printf("Kritik hata: dosya okunamadı. Program sonlandırılıyor.\n");
        return 1;
    }
//----------------------------------------------------------*/
    veri_filtreleme_ve_donusturme(&tarama_verisi);
    
    if (tarama_verisi.points_sayisi > 0) {
        printf("\nFiltreleme sonrasi ilk 5 noktanin koordinatlari:\n");
        for (int i = 0; i < 5 && i < tarama_verisi.points_sayisi; i++) {
            printf("Nokta %d: (x: %.3f, y: %.3f)\n", i, tarama_verisi.points[i].x, tarama_verisi.points[i].y);
        }
    } 
    else 
    {
        printf("Filtreleme sonrasi gecerli nokta bulunamadi. Program sonlandiriliyor.\n");
        free(tarama_verisi.ranges);
        return 1;
    }
printf("\n--- BOLUM 2: RANSAC Dogru Tespiti (Sırala ve Böl ile) ---\n");

// ransac parametreleri
double mesafe_esigi = 0.15; // Bir noktanın bir doğruya 'inlier' sayılması için maksimum uzaklık (5cm)
int min_nokta_sayisi = 8;      // Bir doğrunun en az kaç noktadan oluşması gerektiği
int tekrar_sayisi = 500;   // RANSAC'ın en iyi modeli bulmak için yapacağı deneme sayısı

int bulunan_toplam_dogru_sayisi = 0;
dogru_denklemi* tum_bulunan_dogrular = ransac_ile_dogru_bulma(tarama_verisi.points,  tarama_verisi.points_sayisi, mesafe_esigi, min_nokta_sayisi, tekrar_sayisi, &bulunan_toplam_dogru_sayisi);
// ransac sonrası bulunan doğruları listeleme
if (bulunan_toplam_dogru_sayisi == 0) {
    printf("RANSAC sonucu: Analiz edilecek anlamlı bir doğru bulunamadı.\n");} 
else {
    printf("\nToplam RANSAC sonucu: %d adet anlamlı doğru bulundu.\n", bulunan_toplam_dogru_sayisi);
    for(int i = 0; i < bulunan_toplam_dogru_sayisi; i++) 
    {
        printf("  Dogru %d: a=%.3f, b=%.3f, c=%.3f (%d nokta)\n", i+1, tum_bulunan_dogrular[i].a, tum_bulunan_dogrular[i].b, tum_bulunan_dogrular[i].c, tum_bulunan_dogrular[i].inlier_sayaci);
    }
}

printf("\n--- BOLUM 3/4/5: Kesisim, Aci ve Mesafe Analizi ---\n");    
    int maks_kesisim_sayisi = bulunan_toplam_dogru_sayisi * (bulunan_toplam_dogru_sayisi - 1) / 2;
    if (maks_kesisim_sayisi <= 0){
        maks_kesisim_sayisi = 1;
    }
    
    Point* gecerli_kesisimler = (Point*)malloc(maks_kesisim_sayisi * sizeof(Point));
    int gecerli_kesisim_sayisi = 0;
    double gerekli_minimum_aci = 60.0;

    Point en_yakin_kesisim_noktasi = {0, 0};
    double en_kucuk_mesafe = -1.0;
    char en_yakin_hedef_etiketi[32] = "";
    double en_yakin_hedef_acisi = 0.0;

    for (int i = 0; i < bulunan_toplam_dogru_sayisi; i++) {
        for (int j = i + 1; j < bulunan_toplam_dogru_sayisi; j++) {
            double aci = dogrular_arasi_aci_bul(tum_bulunan_dogrular[i], tum_bulunan_dogrular[j]);
            
            if (aci >= gerekli_minimum_aci) {
                int basarili_mi = 0;
                Point p = kesisim_noktasi_bul(tum_bulunan_dogrular[i], tum_bulunan_dogrular[j], &basarili_mi);
                
                if (basarili_mi) {
                    printf("  GECERLI CIF_T (Dogru %d & %d):\n", i+1, j+1);
                    double robota_uzaklik = sqrt(p.x * p.x + p.y * p.y);
                    if(gecerli_kesisim_sayisi < maks_kesisim_sayisi) {
                        gecerli_kesisimler[gecerli_kesisim_sayisi++] = p;
                    }
                    if (en_kucuk_mesafe == -1.0 || robota_uzaklik < en_kucuk_mesafe) {
                        en_kucuk_mesafe = robota_uzaklik;
                        en_yakin_kesisim_noktasi = p;
                        en_yakin_hedef_acisi = aci;
                        sprintf(en_yakin_hedef_etiketi, "(d%d & d%d)", i+1, j+1);
                    }
                }
            }
        }
    }

    printf("\n--- BOLUM 6: Grafiksel Gosterim (Ister 6) ---\n");
    create_gnuplot_script(&tarama_verisi, tum_bulunan_dogrular, bulunan_toplam_dogru_sayisi, gecerli_kesisimler, gecerli_kesisim_sayisi, en_yakin_kesisim_noktasi, en_kucuk_mesafe, en_yakin_hedef_acisi, en_yakin_hedef_etiketi);
    printf("\nBellek temizleniyor...\n");
    free(tarama_verisi.ranges);
    free(tarama_verisi.points);
    if (tum_bulunan_dogrular != NULL) {
    free(tum_bulunan_dogrular);
    }
    free(gecerli_kesisimler);
    printf("\nProgram basariyla tamamlandi.\n");
    return 0;
}
