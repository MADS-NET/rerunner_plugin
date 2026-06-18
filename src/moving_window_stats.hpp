#include <cmath>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <string>

class MovingWindowStats {
public:
  MovingWindowStats(size_t size = 0) : _size(size){};

  void reset(size_t size) {
    std::lock_guard<std::mutex> lock(_mtx);
    _size = size;
    _signals.clear();
  }

  // Pre-create the per-signal state for a key. Call this single-threaded (e.g.
  // from set_params) for every key that will later be passed to add(), so that
  // concurrent add() calls never need to mutate the map structure.
  void register_key(std::string const &key) {
    std::lock_guard<std::mutex> lock(_mtx);
    ensure(key);
  }

  // append a new value to a given signal according to key; calculate acf and
  // fft only if the buffer is full, calculate mean and stdev regardless of the
  // number of elements in the buffer.
  //
  // Thread-safety: safe to call concurrently as long as each concurrent call
  // uses a *distinct* key. Only the brief map lookup is locked; the per-signal
  // statistics (including the O(N^2) DFT) run lock-free on an object that no
  // other thread touches.
  bool add(std::string const &key, double value);

  bool is_full(std::string const &key) {
    Signal *s = find(key);
    return s && s->buffer.size() >= _size;
  }

  std::vector<double> &acf(std::string const &key) { return get(key).acf; };
  std::vector<double> &fft(std::string const &key) { return get(key).fft; };
  double mean(std::string const &key) { return get(key).mean; };
  double stdev(std::string const &key) { return get(key).stdev; };
  double st_uncertainty(std::string const &key) {
    return get(key).stdev / std::sqrt(_size);
  };

private:
  // All mutable state for a single signal lives here. Each key owns one of
  // these, kept behind a unique_ptr so its address is stable for the lifetime
  // of the key regardless of how the owning map rehashes.
  struct Signal {
    std::deque<double> buffer;
    double sum = 0.0;
    double sum_sq = 0.0;
    double mean = 0.0;
    double stdev = 0.0;
    std::vector<double> acf;
    std::vector<double> fft;
  };

  // Guards only the structure of _signals (insertion/lookup of keys). It does
  // NOT guard the contents of an individual Signal: distinct keys map to
  // distinct Signals, so concurrent add() calls on different keys never alias.
  std::mutex _mtx;
  size_t _size;
  std::unordered_map<std::string, std::unique_ptr<Signal>> _signals;

  // Caller must hold _mtx.
  Signal &ensure(std::string const &key) {
    auto &slot = _signals[key];
    if (!slot)
      slot = std::make_unique<Signal>();
    return *slot;
  }

  // Lock-free lookup; returns nullptr if the key is absent.
  Signal *find(std::string const &key) {
    std::lock_guard<std::mutex> lock(_mtx);
    auto it = _signals.find(key);
    return it == _signals.end() ? nullptr : it->second.get();
  }

  Signal &get(std::string const &key) {
    std::lock_guard<std::mutex> lock(_mtx);
    return ensure(key);
  }

  void calculate_acf(Signal &s);
  void calculate_fft(Signal &s);
};
