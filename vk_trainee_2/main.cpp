#include <set>
#include <random>
#include <cassert>
#include <unordered_set>
#include <iostream>
#include <numeric>
#include <cmath>

using namespace std;

class UniqCounter {
private:	
	const int section_count_ = 32768 / sizeof(int);
	int max_section_size_ = 1;

	// no more than 32kb of memory should be used here + sizeof(std::vector<int>)
	// we will store only approximate sum of different numbers on section
	vector<int> sections_ = vector<int>(section_count_, 0);
	random_device rd_;
	mt19937 gen_ = mt19937(rd_());
	uniform_int_distribution<> dis_ = uniform_int_distribution<int>(1, max_section_size_);

	// from start each section contains only one number, but than we have to reduce accuracy
	void change_section_size_(int max_x)
	{
		const int old_section_size = max_section_size_;
		max_section_size_ = static_cast<int>(pow(2, log2(max_x) - log2(section_count_) + 1));
		vector<int> sections_old = sections_;
		sections_ = vector<int>(section_count_, 0);

		const size_t section_grow = max_section_size_ / old_section_size;
		for (size_t i = 0; i < section_count_ / section_grow; i++)
		{
			int sum = 0;
			for (size_t j = 0; j < section_grow; j++)
			{
				sum += sections_old[i*section_grow + j];
			}
			sections_[i] = sum;
		}
	
		dis_ = uniform_int_distribution<int>(1, max_section_size_);
	}

public:
	// it can be assumed x >= 0
	void add(int x) {
		if (x > max_section_size_ * section_count_)
		{
			change_section_size_(x);
		}

		const int current_section_size = sections_[x / max_section_size_];
		const int rand = dis_(gen_);
		if (rand > current_section_size)
		{
			sections_[x / max_section_size_]++;
		}
	}

	int get_uniq_num() const {
		const int count = accumulate(sections_.begin(), sections_.end(), 0);
		return count;
	}
};

double relative_error(int expected, int got) {
	return abs(got - expected) / (double)expected;
}

int main() {
	std::random_device rd;
	std::mt19937 gen(rd());

	const int N = (int) 1e6;
	for (int k : {1, 10, 1000, 10000, N / 10, N, N * 10}) {
		std::uniform_int_distribution<> dis(1, k);
		std::set<int> all;
		UniqCounter counter;
		for (int i = 0; i < N; i++) {
			int value = dis(gen);
			all.insert(value);
			counter.add(value);
		}
		int expected = (int)all.size();
		int counter_result = counter.get_uniq_num();
		double error = relative_error(expected, counter_result);
		printf("%d numbers in range [1 .. %d], %d uniq, %d result, %.5f relative error\n", N, k, expected, counter_result, error);
		assert(error <= 0.1);
	}

	return 0;
}