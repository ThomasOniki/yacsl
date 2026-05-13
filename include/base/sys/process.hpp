#pragma once

#include "base/containers/cstring.hpp"
#include "base/containers/fwds.hpp"
#include "base/containers/initializer_list.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/vector.hpp"
#include "base/sys/common.hpp"
#include "base/sys/file.hpp"
#include "base/type_traits/range_like.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"
#include <assert.h>

namespace base {

class Process {
public:
	class Stdio {
	public:
		enum class Type : u8 {
			Buffer			 = 0,
			ParentStdin	 = 1,
			ParentStdout = 2,
			ParentStderr = 3,

			File = 254,
			Pipe = 255,
		};
		using enum Type;

		Stdio(const char* path, FileOpenFlags flags) noexcept
				: file_info_{.path = path, .flags = flags}, type_(File) {}

		Stdio(const StringView& path, FileOpenFlags flags) noexcept
				: file_info_{.path = nullptr, .flags = flags}, type_(File) {
			CString cstr(path);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			file_info_.path = cstr.data();
		}

		// NOLINTNEXTLINE(google-explicit-constructor)
		Stdio(Type type) noexcept : type_(type) { assert(type != Pipe && type != File); }

		Stdio(BASE_LIFETIMEBOUND Stdio& pipe) noexcept : pipe_(&pipe), type_(Pipe) {
			assert(pipe.type_ == Buffer);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
			assert(pipe.process_ != nullptr);
		}

		~Stdio() noexcept = default;

		Stdio& operator=(const Stdio&) = delete;

	private:
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
		Stdio& operator=(Stdio&&) = default;
		Stdio(Stdio&&) noexcept		= default;
		Stdio() noexcept					= default;

	private:
		struct FileInfo {
			alignas(16) const char* path;
			FileOpenFlags flags;
		};

		union {
			FileInfo file_info_;
			Process* process_ = nullptr;
			Stdio*	 pipe_;
		};

		Type type_{0};

		friend class Process;
	};

	Process() noexcept = default;
	~Process() noexcept;

	Process(const Process&)						 = delete;
	Process(Process&&)								 = delete;
	Process& operator=(const Process&) = delete;
	Process& operator=(Process&&)			 = delete;

	void start(const char* const* argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
						 Stdio&& stderr_opts) noexcept;

	void start(InitializerList<const char*> argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
						 Stdio&& stderr_opts) noexcept {
		start<InitializerList<const char*>>(argv, base::move(stdin_opts), base::move(stdout_opts),
																				base::move(stderr_opts));
	}

	template <RangeOf<const char*> Range>
	void start(const Range& argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
						 Stdio&& stderr_opts) noexcept {
		Vector<const char*, 6> cstrs;
		for (auto arg : argv) {
			cstrs.push_back(arg);
		}
		cstrs.push_back(nullptr);
		start(cstrs.data(), base::move(stdin_opts), base::move(stdout_opts), base::move(stderr_opts));
	}

	void start(InitializerList<StringView> argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
						 Stdio&& stderr_opts) noexcept {
		start<InitializerList<StringView>>(argv, base::move(stdin_opts), base::move(stdout_opts),
																			 base::move(stderr_opts));
	}

	template <RangeOf<StringView> Range>
	void start(const Range& argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
						 Stdio&& stderr_opts) noexcept {
		const Vector<CString, 6> cstrs(argv);
		Vector<const char*, 6>	 cstr_ptrs;
		cstr_ptrs.reserve(cstrs.size() + 1);
		for (const auto& cstr : cstrs) {
			cstr_ptrs.push_back(cstr.data());
		}
		cstr_ptrs.push_back(nullptr);
		start(cstr_ptrs.data(), base::move(stdin_opts), base::move(stdout_opts),
					base::move(stderr_opts));
	}

	struct ExitStatus {
		alignas(4) u8 exit_code;
		Signal signal;
		Signal stopped;
	};

	ExitStatus wait() noexcept;

	void send_signal(Signal signal) noexcept;

	Stdio& get_stdout() noexcept BASE_LIFETIMEBOUND { return stdout_; }
	Stdio& get_stderr() noexcept BASE_LIFETIMEBOUND { return stderr_; }

private:
	void handle_piped_stdio(void* voidp_file_actions) noexcept;
	void handle_file_stdio(void* voidp_file_actions) noexcept;

private:
	ProcessHandle			 handle_			= NIL_PROCESS_HANDLE;
	StackArray<int, 2> stdin_pipe_	= {-1, -1};
	StackArray<int, 2> stdout_pipe_ = {-1, -1};
	StackArray<int, 2> stderr_pipe_ = {-1, -1};
	Stdio							 stdin_;
	Stdio							 stdout_;
	Stdio							 stderr_;
};

} // namespace base
