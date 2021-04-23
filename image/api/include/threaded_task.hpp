#ifndef VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>

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

			/// @b Pushes a new message into the FIFO.
			void pushMessage(std::string msg) {
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					this->msgs.push(msg);
					this->m_lock.unlock();
				}
			}

			/// @b Pops the top message of the FIFO. Returns true if there still was
			bool popMessage(std::string& msg) {
				if (this->m_lock.try_lock_for(this->timeInterval)) {
					if (this->msgs.empty()) { return false; }
					// copy string, front returns ref and obj is destroyed when pop()-ed :
					msg = std::string(this->msgs.front());
					this->msgs.pop();
					this->m_lock.unlock();
					return true;
				}
				return false;
			}
		protected:
			std::timed_mutex m_lock;				///< The mutex resposible for thread-safety.
			std::atomic<std::size_t> currentStep;	///< The current number of steps achieved
			std::size_t maxSteps;					///< The maximum number of steps. If 0, task not initialized.
			std::chrono::milliseconds timeInterval;	///< The time interval to use for try_lock() on the mutex
			std::queue<std::string> msgs;			///< A queue holding all error/warning messages from the thread(s)
	};
}

#endif // VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
