#include "util/component.hpp"


namespace toygb {
	GBComponent::GBComponent(promise_type* p) : m_handle(std::coroutine_handle<promise_type>::from_promise(*p)) {

	}

	GBComponent::~GBComponent() {
		m_handle.destroy();
	}

	bool GBComponent::done() {
		return m_handle.done();
	}

	void GBComponent::onCycle() {
		m_handle.resume();
	}


	GBComponent GBComponent::promise_type::get_return_object() noexcept {
		return this;
	}

	std::suspend_never GBComponent::promise_type::initial_suspend() noexcept {
		return {};
	}

	std::suspend_always GBComponent::promise_type::final_suspend() noexcept {
		return {};
	}

	void GBComponent::promise_type::unhandled_exception() noexcept {

	}
}
