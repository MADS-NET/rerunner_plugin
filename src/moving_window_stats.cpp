#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <complex>
#include <iostream>
#include <iomanip>
#include "moving_window_stats.hpp"

bool MovingWindowStats::add(std::string const key, double value) {
    auto &buffer = _signal_buffers[key];
    auto &sum = _sum[key];
    auto &sum_sq = _sum_sq[key];

    // push new value
    buffer.push_back(value);
    sum += value;
    sum_sq += value * value;

    if (buffer.size() > _size) {
        double old = buffer.front();
        buffer.pop_front();
        sum -= old;
        sum_sq -= old * old;
    }

    size_t n = buffer.size();

    // recurrent mean & stdev
    double mean = sum / n;
    _mean[key] = mean;

    double variance = (sum_sq / n) - mean * mean;
    if (variance < 0) variance = 0; // numerical guard
    _stdev[key] = std::sqrt(variance);

    // compute acf and fft only if buffer is full
    if (buffer.size() == _size) {
        calculate_acf(key);
        calculate_fft(key);
        return true;
    } else {
        return false;
    }
}

void MovingWindowStats::calculate_acf(std::string const &key) {
    const auto &buffer = _signal_buffers[key];
    size_t N = buffer.size();
    std::vector<double> acf(N, 0.0);

    double mean = _mean[key];
    double denom = 0.0;
    for (size_t i = 0; i < N; i++) {
        denom += (buffer[i] - mean) * (buffer[i] - mean);
    }

    for (size_t lag = 0; lag < N; lag++) {
        double num = 0.0;
        for (size_t i = 0; i < N - lag; i++) {
            num += (buffer[i] - mean) * (buffer[i + lag] - mean);
        }
        acf[lag] = denom > 0 ? num / denom : 0.0;
    }

    _signal_acf[key] = std::move(acf);
}

void MovingWindowStats::calculate_fft(std::string const &key) {
    const auto &buffer = _signal_buffers[key];
    size_t N = buffer.size();
    std::vector<double> fft_magnitude(N, 0.0);

    // naive DFT implementation
    for (size_t k = 0; k < N; k++) {
        std::complex<double> sum(0.0, 0.0);
        for (size_t n = 0; n < N; n++) {
            double angle = -2.0 * M_PI * k * n / N;
            sum += buffer[n] * std::exp(std::complex<double>(0.0, angle));
        }
        fft_magnitude[k] = std::abs(sum) / N;
    }

    // Store only the first N/2 magnitudes (for real signals, FFT is symmetric)
    std::vector<double> fft_half(fft_magnitude.begin(), fft_magnitude.begin() + N / 2);
    _signal_fft[key] = std::move(fft_half);
}

#ifdef MOVING_WINDOW_STATS_TEST
int main() {
    MovingWindowStats stats;
    stats.reset(1000);

    std::string key = "signal";
    for (int i = 0; i < 1000; i++) {
        double val = std::sin(2 * M_PI * 3 * i / 1000.0) + 0.1 * ((rand() % 100) / 100.0 - 0.5);
        stats.add(key, val);
    }

    std::cout << "Mean: " << stats.mean(key) << "\n";
    std::cout << "StdDev: " << stats.stdev(key) << "\n";

    std::cout << "ACF: ";
    for (auto v : stats.acf(key)) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "FFT Magnitudes (first 20 bins): ";
    const auto& fft_vals = stats.fft(key);
    std::cout << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < fft_vals.size() && i < 20; ++i)
        std::cout << fft_vals[i] << " ";
    std::cout << "\n";

    return 0;
}
#endif