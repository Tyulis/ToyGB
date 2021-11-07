#include <coroutine>


namespace toygb {
	struct GBComponent {
		struct promise_type {
			GBComponent get_return_object() noexcept;
			std::suspend_never initial_suspend() noexcept;
			std::suspend_always final_suspend() noexcept;
			void unhandled_exception() noexcept;
		};

		GBComponent(promise_type* p);
		~GBComponent();
		std::coroutine_handle<promise_type> m_handle;

		bool hasNext();
		void next();
	};
}
