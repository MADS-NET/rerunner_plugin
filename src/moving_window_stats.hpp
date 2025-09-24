#include <deque>
#include <map>
#include <vector>
#include <unordered_map>
#include <string>

class MovingWindowStats {
public:
  MovingWindowStats(size_t size = 0) : _size(size){};

  void reset(size_t size) {
    _size = size;
    _mean.clear();
    _stdev.clear();
    _signal_buffers.clear();
    _signal_acf.clear();
    _signal_fft.clear();
    _sum.clear();
    _sum_sq.clear();
  }

  // append a new value to a given signal according to key; calculate acf and
  // fft in _signal_stats only if the fuffer is full, calculate _mean and _stdev
  // regardless the number of elements in the buffer
  bool add(std::string const key, double value);

  bool is_full(std::string const &key) { return _signal_buffers[key].size() >= _size; }

  std::vector<double> &acf(std::string const &key) { return _signal_acf[key]; };
  std::vector<double> &fft(std::string const &key) { return _signal_fft[key]; };
  double mean(std::string const &key) { return _mean[key]; };
  double stdev(std::string const &key) { return _stdev[key]; };
  double st_uncertainty(std::string const &key) {
    return _stdev[key] / std::sqrt(_size);
  };

private:
  size_t _size;
  std::unordered_map<std::string, double> _mean;
  std::unordered_map<std::string, double> _stdev;

  std::unordered_map<std::string, std::deque<double>> _signal_buffers;
  std::unordered_map<std::string, std::vector<double>> _signal_acf;
  std::unordered_map<std::string, std::vector<double>> _signal_fft;
  std::unordered_map<std::string, double> _sum;
  std::unordered_map<std::string, double> _sum_sq;

  void calculate_acf(std::string const &key);
  void calculate_fft(std::string const &key);
};