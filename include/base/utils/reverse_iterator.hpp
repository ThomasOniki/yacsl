#pragma once

namespace base {

template <typename It> class ReverseIterator {
public:
	ReverseIterator(const It& it) : it_(it) { --it_; }
	void						operator*() { return *it_; }
	ReverseIterator operator++() { return --it_; }
	ReverseIterator operator--() { return ++it_; }
	ReverseIterator operator++(int) { return it_--; }
	ReverseIterator operator--(int) { return it_++; }
	bool						operator==(const ReverseIterator& other) { return it_ == other.it_; }
	bool						operator!=(const ReverseIterator& other) { return it_ != other.it_; }

private:
	It it_;
};

} // namespace base
