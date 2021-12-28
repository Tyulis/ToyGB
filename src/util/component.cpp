#include "util/component.hpp"


namespace toygb {
	// Initialize the coroutine handle
	GBComponent::GBComponent(promise_type* p) : m_handle(std::coroutine_handle<promise_type>::from_promise(*p)) {

	}

	// Destroy the coroutine handle
	GBComponent::~GBComponent() {
		m_handle.destroy();
	}

	// Tell whether the coroutine terminated
	bool GBComponent::done() {
		return m_handle.done();
	}

	// Called at every clock tick, advances the coroutine by one step
	void GBComponent::onCycle() {
		m_handle.resume();
	}


	////////// GBComponent::promise_type

	// Get the object to return
	GBComponent GBComponent::promise_type::get_return_object() noexcept {
		return this;
	}

	// Return the action at first call time
	std::suspend_never GBComponent::promise_type::initial_suspend() noexcept {
		return {};
	}

	// Return the action at coroutine end
	std::suspend_always GBComponent::promise_type::final_suspend() noexcept {
		return {};
	}

	// Actions to do when an unhandled exception occurs in the coroutine
	void GBComponent::promise_type::unhandled_exception() noexcept {

	}
}
