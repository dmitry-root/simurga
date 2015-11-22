#include "jobqueue.hpp"

JobQueue::JobQueue()
	: stop_(false)
{
}

JobQueue::~JobQueue()
{
}

JobQueue& JobQueue::instance()
{
	static JobQueue the_instance;
	return the_instance;
}

// dummy function for empty completion handler
static void doNothing() {}

void JobQueue::add(const Job& job)
{
	add(job, doNothing);
}

void JobQueue::add(const Job& job, const Job& on_complete)
{
	std::unique_lock<std::mutex> lock(guard_);
	queue_.push_back( std::make_pair(job, on_complete) );
	signal_.notify_all();
}

void JobQueue::stop()
{
	std::unique_lock<std::mutex> lock(guard_);
	stop_ = true;
	signal_.notify_all();
}

void JobQueue::run()
{
	while (true)
	{
		JobList jobs;
		{
			std::unique_lock<std::mutex> lock(guard_);
			while (!stop_ && queue_.empty())
				signal_.wait(lock);
			if (stop_)
				break;
			jobs.swap(queue_);
		}
		for (auto item : jobs)
		{
			try
			{
				item.first();
			}
			catch (const std::exception& e)
			{
				// TODO log error
				// NOTE currently all job implementations properly handle all exceptions
			}
			try
			{
				item.second();
			}
			catch (const std::exception& e)
			{
				// TODO log error
				// NOTE currently all job implementations properly handle all exceptions
			}
		}
		jobs.clear();
	}
}

