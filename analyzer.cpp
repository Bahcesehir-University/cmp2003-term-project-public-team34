#include "analyzer.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

TripAnalyzer::TripAnalyzer() {
}

TripAnalyzer::~TripAnalyzer() {
}


int TripAnalyzer::safeStringToInt(const string& str) {
    if (str.empty()) return -1;

    int result = 0;
    //string üzerinde gezinme
    for (size_t i = 0; i < str.length(); i++) 
    {
        char c = str[i];
        //karakter rakam mı ?
         if (c < '0' || c > '9') 
         {
            return -1; 
        }
        //matematiksel basamak hesabı dönüşüm
        result = result * 10 + (c - '0');
    }

    return result;
}

string TripAnalyzer::intToString(int number) const {
    // sayı 0 ise direkt sıfra dön.
    if (number == 0) return "0";

    string result = "";
    
    while (number > 0) {
        // % ile son basamağı al
        int digit = number % 10; 
        
        
        //rakamları asciiye cevirr
        char c = digit + '0'; 
        
        
        result = c + result; 
        
      
        number = number / 10; 
    }
    
    return result;
}

int TripAnalyzer::parseHour(const string& dateTime) {
    int length = dateTime.length(); 
    int spaceIndex = -1;
    int colonIndex = -1;

    for (int i = 0; i < length; i++) {
        if (dateTime[i] == ' ') {
            spaceIndex = i;
            break;
        }
    }

    if (spaceIndex == -1) return -1;

    for (int i = spaceIndex + 1; i < length; i++) {
        if (dateTime[i] == ':') {
            colonIndex = i;
            break;
        }
    }

    if (colonIndex == -1) return -1;

    string hourStr = "";
    for (int i = spaceIndex + 1; i < colonIndex; i++) {
        hourStr += dateTime[i];
    }

    int hour = safeStringToInt(hourStr);
    
    if (hour < 0 || hour > 23) {
        return -1;
    }
    
    return hour;
}

string TripAnalyzer::makeSlotKey(const string& zone, int hour) const {
    string hourStr = intToString(hour);
    // KADIKOY | 14 gibi göstersin diye
    return zone + "|" + hourStr;
}



void TripAnalyzer::ingestFile(const string& csvPath) {
    ifstream file(csvPath);
    if (!file.is_open()) return;

    string line;
    
    if (!getline(file, line)) { 
        file.close(); 
        return; 
    }

    //Complexity Analysis- her satıra sıra sıra girer  - O(n)
    while (getline(file, line)) {
        if (line.empty()) continue;

        string zone = "";
        string dateTime = "";
        
        // Week 3'teki Sequential Searc konusu
        //satırı harf harf gezeriz virgüle gelince ayırmak için
        
        int commaCount = 0; // virgül sayısı(sonra artacak işte)
        int t=line.length();
        for (int i = 0; i < t; i++) {
            char c = line[i];
            
            if (c == ',') {
                commaCount++; 
                continue;     //virgülü kaydetmeyiz o sadce ayıraç
            }
            
            // Hocanın data şeyi ID, Zone, DateTime (Varsayım)
            // 1. virgülden sonra Zone geliyo, 2. virgülden sonra DateTime geliyo. onun ayrımın da aşağıdaki kodlar
            
            if (commaCount == 1) {
                // 1. virgülden sonraki her karakteri zone'a ekle
                // Week 3: String concatenation logic
                zone += c; 
            }
            else if (commaCount == 2) {
                // 2. virgülden sonraki her karakteri dateTime'a ekle
                dateTime += c;
            }
        }

        // yine hata olayı için
        if (zone.empty() || dateTime.empty()) {
            continue;
        }

        //önce yazılan saat ayırması(parsesi)
        int hour = parseHour(dateTime); 
        if (hour < 0 || hour > 23) {
            continue;
        }

        //Week 9 - Hash konusuu
        zoneMap[zone]++;
        
        // hash'in key'i (zone ve hour)
        string slotKey = makeSlotKey(zone, hour);
        slotMap[slotKey]++;
    }

    file.close();
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> results;
    // kod her seferinde sormasın diye rezerve mantığı yapıyoruz.Önden sayı belirleme işi
    results.reserve(zoneMap.size());
    
    // 1. Map'i ayrı bi vektöre atarız
    for (const auto& pair : zoneMap) {
        results.push_back({pair.first, pair.second});
    }
    
    // Kontrol
    if (k > (int)results.size()) {
        k = results.size();
    }

    // Partial Sort - normal sorta gerek yok sadece 10 tane sıralama lazım bize 
    //[] bu kısım araya sadece burada kullanmalık fonskiyon ekliyo
    auto comp = [](const ZoneCount& a, const ZoneCount& b) {
        // Önce sayıya bak (Büyük olan öne)
        if (a.count != b.count) {
            return a.count > b.count;
        }
        // Sayılar eşitse isme göre 
        return a.zone < b.zone;
    };

    // Tüm liste yerine k'ya kadar sırala ologn'den ologk'ya küçüldü
    partial_sort(results.begin(), results.begin() + k, results.end(), comp);

    // k ya kadar olanı aldım gerisini atarız fazlalığı atarım
    results.resize(k);
    
    return results;
}



void TripAnalyzer::merge(vector<SlotCount>& array, int left, int mid, int right) const {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<SlotCount> L(n1), R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = array[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = array[mid + 1 + j];

    int i = 0; 
    int j = 0; 
    int k = left;

    while (i < n1 && j < n2) {
        
        bool pickLeft = false;

        if (L[i].count > R[j].count) {
            pickLeft = true;
        }
        else if (L[i].count == R[j].count) {
            if (L[i].zone < R[j].zone) {
                pickLeft = true;
            }
            else if (L[i].zone == R[j].zone) {
                if (L[i].hour < R[j].hour) {
                    pickLeft = true;
                }
            }
        }




        if (pickLeft) {
            array[k] = L[i];
            i++;
        } else {
            array[k] = R[j];
            j++;
        }
        k++;
    }

    // Geriye kalan elemanları kopyala
    while (i < n1) {
        array[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        array[k] = R[j];
        j++;
        k++;
    }
}

//topbusyslotu sıralama için merge sort ayarlamdı
void TripAnalyzer::mergeSortSiralama(vector<SlotCount>& array, int left, int right) const {
    if (left >= right) {
        return; 
    }
    
    int mid = left + (right - left) / 2;
    
    // Solu sırala
    mergeSortSiralama(array, left, mid);
    // Sağı sırala
    mergeSortSiralama(array, mid + 1, right);
    // Birleştir
    merge(array, left, mid, right);
}


vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> results;
    results.reserve(slotMap.size()); 
    
    for (const auto& pair : slotMap) {
        string key = pair.first;
        int count = pair.second;
        
        string zone = "";
        string hourStr = "";
        bool pipeFound = false;
        int s=key.length();
        for (size_t i = 0; i < s; i++) {
            if (key[i] == '|') {
                pipeFound = true;
                continue; 
            }
            if (!pipeFound) {
                zone += key[i]; 
            } else {
                hourStr += key[i]; 
            }
        }
        
        if (!pipeFound || hourStr.empty()) continue;
        
        int hour = 0;
        for(char c : hourStr) {
            hour = hour * 10 + (c - '0');
        }
        
        results.push_back({zone, hour, count});
    }
    
    if (!results.empty()) {
        mergeSortSiralama(results, 0, results.size() - 1);
    }
    
    if (k < (int)results.size()) {
        results.resize(k);
    }
    
    return results;
}
