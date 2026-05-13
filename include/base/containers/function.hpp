#pragma once

#include "base/containers/stack_array.hpp"
#include "base/memory/arena.hpp" // NOLINT(misc-include-cleaner)
#include "base/memory/fwds.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/placement_new.hpp" // NOLINT(misc-include-cleaner)
#include "base/utilities/type_aliases.hpp"

namespace base {

namespace internal {

template <typename Allocator, usize BufSz, typename Ret, typename... Args> class FunctionMembers {
protected:
	StackArray<uchar, BufSz> small_buffer_																	= {};
	void*										 data_																					= small_buffer_.data();
	void (*cleanup_)(void* func, UnderlyingAllocator<Allocator>& allocator) = nullptr;
	Ret (*call_)(void* func, Args... args)																	= nullptr;
	Allocator allocator_																										= Allocator();
};

template <typename Allocator, typename Ret, typename... Args>
class FunctionMembers<Allocator, 0, Ret, Args...> {
protected:
	void* data_																															= nullptr;
	void (*cleanup_)(void* func, UnderlyingAllocator<Allocator>& allocator) = nullptr;
	Ret (*call_)(void* func, Args... args)																	= nullptr;
	Allocator allocator_																										= Allocator();
};

template <SingletonAllocatorLike Allocator, usize BufSz, typename Ret, typename... Args>
class FunctionMembers<Allocator, BufSz, Ret, Args...> {
protected:
	StackArray<uchar, BufSz> small_buffer_																	= {};
	void*										 data_																					= small_buffer_.data();
	void (*cleanup_)(void* func, UnderlyingAllocator<Allocator>& allocator) = nullptr;
	Ret (*call_)(void* func, Args... args)																	= nullptr;
};

template <SingletonAllocatorLike Allocator, typename Ret, typename... Args>
class FunctionMembers<Allocator, 0, Ret, Args...> {
protected:
	void* data_																															= nullptr;
	void (*cleanup_)(void* func, UnderlyingAllocator<Allocator>& allocator) = nullptr;
	Ret (*call_)(void* func, Args... args)																	= nullptr;
};

} // namespace internal

template <typename F, usize BufSz = 8, typename Allocator = MainArenaAllocator> class Function;

template <typename Ret, typename... Args, usize BufSz, typename Allocator>
class Function<Ret(Args...), BufSz, Allocator>
		: internal::FunctionMembers<Allocator, BufSz, Ret, Args...> {
public:
	// NOLINTNEXTLINE(google-explicit-constructor)
	Function(auto&& func);

	Function(const Function&)			= delete;
	Function(Function&&) noexcept = delete;

	~Function() noexcept { cleanup_(data_, allocator()); }

	Function& operator=(const Function&)		 = delete;
	Function& operator=(Function&&) noexcept = delete;

	Ret operator()(Args... args) { return call_(data_, base::forward<Args>(args)...); }

	[[nodiscard]] const auto& get_allocator() const noexcept { return allocator(); }

private:
	[[nodiscard]] auto& allocator() BASE_LIFETIMEBOUND;

private:
	using Members = internal::FunctionMembers<Allocator, BufSz, Ret, Args...>;
	using Members::call_;
	using Members::cleanup_;
	using Members::data_;
};

template <typename Ret, typename... Args, usize BufSz, typename Allocator>
Function<Ret(Args...), BufSz, Allocator>::Function(auto&& func) {
	using Func = RemoveReference<decltype(func)>;

	if constexpr (sizeof(Func) > sizeof(uchar) * BufSz || alignof(Func) > 8) {
		data_ = allocator().template allocate<Func>();

		cleanup_ = [](void* data, UnderlyingAllocator<Allocator>& allocator) {
			static_cast<Func*>(data)->~Func();
			allocator.template free<Func>(static_cast<Func*>(data));
		};
	} else {
		cleanup_ = [](void* data, UnderlyingAllocator<Allocator>& /*allocator*/) {
			static_cast<Func*>(data)->~Func();
		};
	}

	new (data_) Func(base::forward<decltype(func)>(func));

	call_ = [](void* data, Args... args) {
		return (*static_cast<Func*>(data))(base::forward<Args>(args)...);
	};
}

template <typename Ret, typename... Args, usize BufSz, typename Allocator>
auto& Function<Ret(Args...), BufSz, Allocator>::allocator() {
	if constexpr (SingletonAllocatorLike<Allocator>) {
		return Allocator::get();
	} else {
		return Members::allocator_;
	}
}

#ifdef BASE_LSP_ONLY
template Function<void(), 0>::Function(decltype([]() {})& func);
template Function<void()>::Function(decltype([]() {})&& func);
#endif

} // namespace base
