#ifndef _UTIL_COMPONENT_HPP
#define _UTIL_COMPONENT_HPP

#include <coroutine>


namespace toygb {
	/** Here is some fine C++ coroutine boilerplate code
	 * This handles the coroutine handle and promise for each GB component */
	class GBComponent {
		public:
			struct promise_type {
				GBComponent get_return_object() noexcept;
				std::suspend_never initial_suspend() noexcept;
				std::suspend_always final_suspend() noexcept;
				void unhandled_exception() noexcept;
			};

			GBComponent(promise_type* p);
			~GBComponent();

			bool done();
			void onCycle();

		protected:
			std::coroutine_handle<promise_type> m_handle;
	};
}

#endif
