#pragma once
#include "windows.h"
#include <ctime>

class CTime {
    time_t m_time;
public:
    CTime() : m_time(0) {}
    explicit CTime(time_t t) : m_time(t) {}
    static CTime GetCurrentTime() { return CTime(std::time(nullptr)); }
    time_t GetTime() const { return m_time; }
    int GetYear() const   { struct tm t{}; localtime_r(&m_time, &t); return t.tm_year + 1900; }
    int GetMonth() const  { struct tm t{}; localtime_r(&m_time, &t); return t.tm_mon + 1; }
    int GetDay() const    { struct tm t{}; localtime_r(&m_time, &t); return t.tm_mday; }
    int GetHour() const   { struct tm t{}; localtime_r(&m_time, &t); return t.tm_hour; }
    int GetMinute() const { struct tm t{}; localtime_r(&m_time, &t); return t.tm_min; }
    int GetSecond() const { struct tm t{}; localtime_r(&m_time, &t); return t.tm_sec; }
    int GetDayOfWeek() const { struct tm t{}; localtime_r(&m_time, &t); return t.tm_wday + 1; }
    bool operator==(const CTime& o) const { return m_time == o.m_time; }
    bool operator!=(const CTime& o) const { return m_time != o.m_time; }
    bool operator<(const CTime& o) const  { return m_time < o.m_time; }
    bool operator>(const CTime& o) const  { return m_time > o.m_time; }
    bool operator<=(const CTime& o) const { return m_time <= o.m_time; }
    bool operator>=(const CTime& o) const { return m_time >= o.m_time; }
};
