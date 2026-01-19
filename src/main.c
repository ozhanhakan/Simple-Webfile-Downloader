#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // isspace için
#include <curl/curl.h>

// --- Helper Functions ---

// Stringin başındaki ve sonundaki boşlukları (space, tab, newline) temizler
void trim_whitespace(char *str) {
    if (!str) return;

    char *end;

    // Baştaki boşlukları atla
    char *start = str;
    while(isspace((unsigned char)*start)) start++;

    // Tüm string boşluksa
    if(*start == 0) {
        *str = 0;
        return;
    }

    // Sondaki boşlukları bul
    end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;

    // Yeni bitiş noktası
    *(end+1) = 0;

    // Eğer baştan boşluk atıldıysa, stringi kaydır
    if (start > str) {
        memmove(str, start, (end - start + 2));
    }
}

// URL'den dosya adı çıkarma
void get_filename_from_url(const char *url, char *buffer) {
    const char *last_slash = strrchr(url, '/');
    if (last_slash) {
        const char *question_mark = strchr(last_slash, '?');
        if (question_mark) {
            size_t len = question_mark - (last_slash + 1);
            strncpy(buffer, last_slash + 1, len);
            buffer[len] = '\0';
        } else {
            strcpy(buffer, last_slash + 1);
        }
    } else {
        strcpy(buffer, "downloaded_file.dat");
    }
    // Dosya adında da boşluk kalmış olabilir, temizleyelim
    trim_whitespace(buffer);
}

// Base URL ve Relative Path birleştirme
void resolve_url(const char *base_url, const char *relative_path, char *full_url) {
    if (strncmp(relative_path, "http", 4) == 0) {
        strcpy(full_url, relative_path);
        return;
    }

    char domain[256];
    // Basit domain bulucu: https://site.com kısmını alır
    // Not: Daha karmaşık path yapıları için burası geliştirilebilir
    char *scheme_end = strstr(base_url, "://");
    if (scheme_end) {
        char *path_start = strchr(scheme_end + 3, '/');
        if (path_start) {
            size_t len = path_start - base_url;
            strncpy(domain, base_url, len);
            domain[len] = '\0';
        } else {
            strcpy(domain, base_url);
        }
    } else {
        strcpy(domain, base_url);
    }

    if (relative_path[0] == '/') {
        sprintf(full_url, "%s%s", domain, relative_path);
    } else {
        sprintf(full_url, "%s/%s", domain, relative_path);
    }
}

// --- Memory & Curl ---

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

void download_file(const char *url) {
    CURL *curl;
    FILE *fp;
    char filename[256];

    // URL'in temiz olduğundan emin olalım
    char clean_url[1024];
    strcpy(clean_url, url);
    trim_whitespace(clean_url);

    get_filename_from_url(clean_url, filename);
    
    if (strlen(filename) < 2) return;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(filename, "wb");
        if (!fp) {
            printf("  [HATA] Dosya acilamadi: %s\n", filename);
            return;
        }

        printf("  [INDIRILIYOR] %s\n", filename);
        curl_easy_setopt(curl, CURLOPT_URL, clean_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        curl_easy_perform(curl);
        
        fclose(fp);
        curl_easy_cleanup(curl);
    }
}

void process_html_content(char *html_content, const char *base_url, const char *filter_ext) {
    char *cursor = html_content;
    const char *search_str = "href=";
    char full_url[2048]; // Uzun URL'ler için buffer büyütüldü
    
    printf("\nSayfa taranıyor...\n");

    while ((cursor = strstr(cursor, search_str)) != NULL) {
        cursor += 5; 
        
        char quote_char = '"';
        if (*cursor == '\'') {
            quote_char = '\'';
            cursor++;
        } else if (*cursor == '"') {
            cursor++;
        } else {
            continue; 
        }

        char *end_quote = strchr(cursor, quote_char);
        
        if (end_quote) {
            long len = end_quote - cursor;
            if (len > 0 && len < 2000) {
                char *raw_link = malloc(len + 1);
                strncpy(raw_link, cursor, len);
                raw_link[len] = '\0';

                // KRİTİK NOKTA: Linkin başındaki/sonundaki boşlukları temizle
                trim_whitespace(raw_link);

                // Boş link değilse işle
                if (strlen(raw_link) > 0) {
                    resolve_url(base_url, raw_link, full_url);

                    char temp_filename[256];
                    get_filename_from_url(full_url, temp_filename);
                    
                    // Dosya uzantısını bul
                    char *ext = strrchr(temp_filename, '.');

                    int should_download = 0;
                    if (filter_ext && ext) {
                        // Case-insensitive karşılaştırma (PDF == pdf)
                        // +1 noktadan sonraki kısım
                        if (strcasecmp(ext + 1, filter_ext) == 0) {
                            should_download = 1;
                        }
                    } else if (ext) {
                        // Filtre yoksa genel dosyaları indir
                        if (strcasecmp(ext, ".html") != 0 && strcasecmp(ext, ".php") != 0 && strcasecmp(ext, ".com") != 0) {
                            should_download = 1;
                        }
                    }

                    if (should_download) {
                        // Aynı dosyayı tekrar tekrar indirmemek için basit bir kontrol eklenebilir
                        // Şimdilik direkt indiriyoruz.
                        download_file(full_url);
                    }
                }
                free(raw_link);
            }
            cursor = end_quote;
        }
    }
}

void fetch_and_parse_url(const char *url, const char *filter_ext) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if(curl) {
        printf("Siteye bağlanılıyor: %s\n", url);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "Curl error: %s\n", curl_easy_strerror(res));
        } else {
            process_html_content(chunk.memory, url, filter_ext);
        }

        curl_easy_cleanup(curl);
    }
    free(chunk.memory);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Kullanım: %s <url> [dosya_turu]\n", argv[0]);
        return 1;
    }

    char *url = argv[1];
    char *ext_filter = (argc == 3) ? argv[2] : NULL;
    
    // Girilen URL'de de yanlışlıkla boşluk bırakılmış olabilir, temizleyelim
    trim_whitespace(url);
    if(ext_filter) trim_whitespace(ext_filter);

    char filename[256];
    get_filename_from_url(url, filename);
    char *dot = strrchr(filename, '.');

    int is_direct_file = 0;
    if (dot) {
        if (strcasecmp(dot, ".html") != 0 && strcasecmp(dot, ".php") != 0 && strcasecmp(dot, ".tr") != 0) {
             is_direct_file = 1;
        }
    }

    if (is_direct_file) {
        download_file(url);
    } else {
        fetch_and_parse_url(url, ext_filter);
    }

    return 0;
}