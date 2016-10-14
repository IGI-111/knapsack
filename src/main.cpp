#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace {

struct Item {
    Item(int weight, int value) : weight(weight), value(value) {}
    int weight;
    int value;
    double rentability() { return static_cast<double>(value) / weight; }
};

template <size_t N>
using PossibleItems = std::array<Item, N>;

template <size_t N>
class Sack {
   public:
    Sack(const PossibleItems<N>& items, const int capacity)
        : decided(), value(0), weight(0), items(items), capacity(capacity) {}
    Sack(const Sack<N>& sack)
        : decided(sack.decided),
          value(sack.value),
          weight(sack.weight),
          items(sack.items),
          capacity(sack.capacity) {}
    Sack(const Sack<N>&& sack)
        : decided(sack.decided),
          value(sack.value),
          weight(sack.weight),
          items(sack.items),
          capacity(sack.capacity) {}
    void decide(bool take_item) {
        if (take_item) {
            size_t index = decided.size();
            Item item = items[index];
            weight += item.weight;
            value += item.value;
        }
        decided.push_back(take_item);
    }
    bool is_full(void) const {
        return weight >= capacity ||                              // we're exactly full
               decided.size() >= items.size() ||                  // all items are in
               weight + items[decided.size()].weight > capacity;  // the next item cannot be added
    }
    bool possibly_better(const int value) {
        int remaining_potential =
            std::accumulate(items.cbegin() + decided.size(), items.cend(), this->value,
                            [](int a, const auto& it) { return a + it.value; });
        return remaining_potential > value;
    }
    int get_value(void) const { return value; }
    int get_weight(void) const { return weight; }
    std::vector<Item> get_contents(void) const {
        std::vector<Item> result;
        for (size_t i = 0; i < decided.size(); ++i) {
            if (decided[i]) {
                result.push_back(items[i]);
            }
        }
        return result;
    }

   private:
    std::vector<bool> decided;
    int value;
    int weight;
    const PossibleItems<N>& items;
    const int capacity;
};

template <size_t N>
Sack<N> best_knapsack(PossibleItems<N>& items, const int capacity) {
    std::sort(items.begin(), items.end(),
              [](auto a, auto b) { return a.rentability() > b.rentability(); });
    auto best = std::make_unique<Sack<N>>(items, capacity);
    knapsack_rec(Sack<N>(items, capacity), best);
    return *best;
}

template <size_t N>
void knapsack_rec(Sack<N> existing, std::unique_ptr<Sack<N>>& best) {
    best = std::make_unique<Sack<N>>(std::max(existing, *best, [](const auto& a, const auto& b) {
        return a.get_value() < b.get_value();
    }));
    if (existing.is_full() || !existing.possibly_better(best->get_value())) {
        return;
    }
    Sack<N>& left = existing;
    Sack<N> right(existing);
    left.decide(true);
    right.decide(false);
    knapsack_rec(left, best);
    knapsack_rec(right, best);
}
}

int main(void) {
    PossibleItems<7> items = {{Item(41, 442), Item(50, 525), Item(49, 511), Item(59, 593),
                               Item(55, 546), Item(57, 564), Item(60, 617)}};
    constexpr int capacity = 170;
    const auto s = best_knapsack(items, capacity);
    for (const auto& val : s.get_contents()) {
        std::cout << '(' << val.weight << ", " << val.value << ") ";
    }
    std::cout << "weight: " << s.get_weight() << " value: " << s.get_value() << std::endl;
    return 0;
}
