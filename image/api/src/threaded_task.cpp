#include "../include/threaded_task.hpp"

namespace Image {

	ThreadedTask::ThreadedTask(std::size_t _max_steps) : m()  {
		this->maxSteps = _max_steps;
		this->currentStep = 0;
		this->timeInterval = std::chrono::milliseconds(10);
		this->msgs = {};
	}

	bool ThreadedTask::isComplete() {
		bool retval = false;
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			retval = (this->currentStep > this->maxSteps);
			m_lock.unlock();
		}
		return retval;
	}

	void ThreadedTask::end() {
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			this->maxSteps = 0;
			this->currentStep = 1;
			m_lock.unlock();
		}
		return;
	}

	bool ThreadedTask::hasSteps() {
		bool retval = false;
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			retval = this->maxSteps > std::size_t(0);
			m_lock.unlock();
		}
		return retval;
	}

	std::size_t ThreadedTask::getMaxSteps() {
		std::size_t retval = 0;
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			retval = this->maxSteps;
			m_lock.unlock();
		}
		return retval;
	}

	void ThreadedTask::setSteps(std::size_t _ms) {
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			this->maxSteps = _ms;
			m_lock.unlock();
		}
		return;
	}

	std::size_t ThreadedTask::getAdvancement() {
		std::size_t retval = 0;
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			retval = this->currentStep;
			m_lock.unlock();
		}
		return retval;
	}

	void ThreadedTask::setAdvancement(std::size_t newcurrentvalue) {
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			this->currentStep = newcurrentvalue;
			m_lock.unlock();
		}
		return;
	}

	void ThreadedTask::advance() {
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			this->currentStep++;
			m_lock.unlock();
		}
		return;
	}

	void ThreadedTask::pushMessage(std::string msg) {
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			this->msgs.push(msg);
			m_lock.unlock();
		}
	}

	bool ThreadedTask::popMessage(std::string &msg) {
		std::unique_lock m_lock(this->m, std::defer_lock);
		if (m_lock.try_lock_for(this->timeInterval)) {
			if (this->msgs.empty()) { return false; }
			// copy string, front returns ref and obj is destroyed when pop()-ed :
			msg = std::string(this->msgs.front());
			this->msgs.pop();
			m_lock.unlock();
			return true;
		}
		return false;
	}

} // Image namespace
