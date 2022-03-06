class Engine
{
private:
    int m_year;
    int m_month;
    int m_day;

public:
    Engine(int year, int month, int day);

    void SetEngine(int year, int month, int day);

    int getYear() { return m_year; }
    int getMonth() { return m_month; }
    int getDay()  { return m_day; }
};

// Engine constructor
Engine::Engine(int year, int month, int day)
{
    SetEngine(year, month, day);
}

// Engine member function
void Engine::SetEngine(int year, int month, int day)
{
    m_month = month;
    m_day = day;
    m_year = year;
}