#ifndef VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>

namespace Image {

	/// @brief Simple enum to track the stack's state. Can be conveted to integer for easier comparison.
	/// @note Also contains two exit states : End_Success and End_Failure.
	enum TaskState {
		Created		= 0,
		Ready		= 1,
		Launched	= 2,
		Running		= 3,
		Finishing	= 4,
		Finished	= 5,
		End_Success = 6,
		End_Failure = 7
	};

	/**
	 * @brief The ThreadedTask class provides a simple way to interact with a task done in a separate thread.
	 * @note Shamelessly stolen from `image/include/reader.hpp`.
	 */
	class ThreadedTask : public std::enable_shared_from_this<ThreadedTask> {
	public:
		/// @brief A simple typedef enclosing a pointer to a threaded task
		using Ptr = std::shared_ptr<ThreadedTask>;

	public:
		/// @brief Ctor for a threaded task.
		ThreadedTask(std::size_t _maxSteps = 0);
		/// @brief Default dtor for the class.
		~ThreadedTask(void) = default;

		/// @brief Returns the result of std::enable_shared_from_this<>::shared_from_this()
		ThreadedTask::Ptr getPtr();

		/// @brief Checks if the task is complete.
		bool isComplete(void);

		/// @brief Allows to immediately end a task.
		void end(bool success = true);

		/// @brief Check if the task has steps.
		bool hasSteps(void);

		/// @brief Get the maximum number of steps possible
		std::size_t getMaxSteps(void);

		/// @brief Set the max number of steps for the task
		void setSteps(std::size_t _ms);

		/// @brief Get current advancement of the task
		std::size_t getAdvancement(void);

		/// @brief Set the current progress of the task
		void setAdvancement(std::size_t newcurrentvalue);

		/// @brief Set the task's state
		void setState(TaskState _new_state);

		/// @brief Get the current task's state.
		TaskState getState(void);

		/// @brief Advances a step (thread-safe)
		void advance(void);

		/// @brief Pushes a new message into the FIFO.
		void pushMessage(std::string msg);

		/// @brief Pops the top message of the FIFO. Returns true if there still was
		bool popMessage(std::string& msg);

	protected:
		std::timed_mutex m;	   ///< The lock resposible for thread-safety.
		std::atomic<std::size_t> currentStep;	 ///< The current number of steps achieved
		std::size_t maxSteps;	 ///< The maximum number of steps. If 0, task not initialized.
		std::chrono::milliseconds timeInterval;	   ///< The time interval to use for try_lock() on the mutex
		std::queue<std::string> msgs;	 ///< A queue holding all error/warning messages from the thread(s)
		TaskState task_state;	 ///< The state of the current task
	};
}	 // namespace Image

#endif	  // VISUALIZATION_IMAGE_API_INCLUDE_THREADED_TASK_HPP_
