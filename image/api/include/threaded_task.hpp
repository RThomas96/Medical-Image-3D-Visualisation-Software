#ifndef VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_

#include <atomic>
#include <memory>
#include <mutex>

namespace Image {

	/**
	 * @brief The ThreadedTask class provides a simple way to interact with a task done in a separate thread.
	 * @note Shamelessly stolen from `image/include/reader.hpp`.
	 */
	class ThreadedTask {
		public:
			using Ptr = std::shared_ptr<ThreadedTask>;
		public:
			/// @b Ctor for a threaded task.
			ThreadedTask(std::size_t _maxSteps = 0) : m_lock() {
				this->maxSteps = _maxSteps;
				this->currentStep = 0;
				this->timeInterval = std::chrono::milliseconds(10);
			}
			/// @b Default dtor for the class.
			~ThreadedTask(void) = default;
			/// @b Checks if the task is complete.
			bool isComplete(void) {
				bool retval = false;
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					retval = (this->maxSteps > std::size_t(0)) && (this->currentStep >= this->maxSteps-1);
					this->m_lock.unlock();
				}
				return retval;
			}
			/// @b Allows to immediately end a task.
			void end(void) {
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					if (this->maxSteps == 0) {
						this->maxSteps = 1;
						this->currentStep = 2;
					} else {
						this->currentStep = this->maxSteps+1;
					}
					this->m_lock.unlock();
				}
				return;
			}
			/// @b Check if the task has steps.
			bool hasSteps(void) {
				bool retval = false;
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					retval = this->maxSteps > std::size_t(0);
					this->m_lock.unlock();
				}
				return retval;
			}
			/// @b Get the maximum number of steps possible
			std::size_t getMaxSteps(void) {
				std::size_t retval = 0;
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					retval = this->maxSteps;
					this->m_lock.unlock();
				}
				return retval;
			}
			/// @b Set the max number of steps for the task
			void setSteps(std::size_t _ms) {
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					this->maxSteps = _ms;
					this->m_lock.unlock();
				}
				return;
			}
			/// @b Get current advancement of the task
			std::size_t getAdvancement(void) {
				std::size_t retval = 0;
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					retval = this->currentStep;
					this->m_lock.unlock();
				}
				return retval;
			}
			void setAdvancement(std::size_t newcurrentvalue) {
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					this->currentStep = newcurrentvalue;
					this->m_lock.unlock();
				}
				return;
			}
			/// @b Advances a step (thread-safe)
			void advance(void) {
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					this->currentStep++;
					this->m_lock.unlock();
				}
				return;
			}
		protected:
			std::timed_mutex m_lock;				/// @b The mutex resposible for thread-safety.
			std::atomic<std::size_t> currentStep;	/// @b The current number of steps achieved
			std::size_t maxSteps;					/// @b The maximum number of steps. If 0, task has not been initialized.
			std::chrono::milliseconds timeInterval;	/// @b The time interval to use for try_lock() on the mutex
	};
}

#endif // VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
