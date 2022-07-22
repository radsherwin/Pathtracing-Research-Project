#include <chrono>
#include <iostream>

class timer
{
public:
    void tic()
    {
        m_start_time = std::chrono::steady_clock::now();
        m_is_running = true;
    }

    void toc()
    {
        m_end_time = std::chrono::steady_clock::now();
        m_is_running = false;
    }

    double elapsed_time()
    {
        return elapsed_time_ms() / 1000.0;
    }

    double elapsed_time_ms()
    {
        std::chrono::time_point<std::chrono::steady_clock> end_time;

        if (m_is_running)
        {
            end_time = std::chrono::steady_clock::now();
        }
        else
        {
            end_time = m_end_time;
        }

        return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_start_time).count());
    }

    void print_ms()
    {
        printf("Total time: %.2fms\n", elapsed_time_ms());
    }

    void print_s()
    {
        printf("Total time: %.2fs\n", elapsed_time());
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> m_start_time;
    std::chrono::time_point<std::chrono::steady_clock> m_end_time;
    bool m_is_running = false;
};