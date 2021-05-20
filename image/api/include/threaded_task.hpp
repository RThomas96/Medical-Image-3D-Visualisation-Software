#ifndef VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <iostream>

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
			ThreadedTask(std::size_t _maxSteps = 0);
			/// @b Default dtor for the class.
			~ThreadedTask(void) = default;

			/// @b Checks if the task is complete.
			bool isComplete(void);

			/// @b Allows to immediately end a task.
			void end(void);

			/// @b Check if the task has steps.
			bool hasSteps(void);

			/// @b Get the maximum number of steps possible
			std::size_t getMaxSteps(void);

			/// @b Set the max number of steps for the task
			void setSteps(std::size_t _ms);

			/// @b Get current advancement of the task
			std::size_t getAdvancement(void);

			void setAdvancement(std::size_t newcurrentvalue);

			/// @b Advances a step (thread-safe)
			void advance(void);

			/// @b Pushes a new message into the FIFO.
			void pushMessage(std::string msg);

			/// @b Pops the top message of the FIFO. Returns true if there still was
			bool popMessage(std::string& msg);

		protected:
			std::timed_mutex m;						///< The lock resposible for thread-safety.
			std::atomic<std::size_t> currentStep;	///< The current number of steps achieved
			std::size_t maxSteps;					///< The maximum number of steps. If 0, task not initialized.
			std::chrono::milliseconds timeInterval;	///< The time interval to use for try_lock() on the mutex
			std::queue<std::string> msgs;			///< A queue holding all error/warning messages from the thread(s)
	};
}

#endif // VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
