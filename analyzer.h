#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;
struct ZoneCount {
    string zone;
    long long count;
};

struct SlotCount {
    string zone;
    int hour;
    long long count;
};

class TripAnalyzer {
private:
    unordered_map<std::string, long long> zoneMap;
    unordered_map<std::string, long long> slotMap;
    string intToString(int number) const;
    
    int parseHour(const std::string& dateTime); //saati ayıkla tarihten
    int safeStringToInt(const std::string& str); //metindeki sayıyyı int hale getirme
    string makeSlotKey(const std::string& zone, int hour) const; //bölge ve saati birleştirsin
    void merge(std::vector<SlotCount>& array, int left, int mid, int right) const;
    void mergeSortSiralama(std::vector<SlotCount>& array, int left, int right) const;
public:
    TripAnalyzer();
    ~TripAnalyzer();

    void ingestFile(const std::string& csvPath);
    vector<ZoneCount> topZones(int k = 10) const;
    vector<SlotCount> topBusySlots(int k = 10) const;
};

#endif






