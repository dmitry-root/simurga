/**
 * \brief Queue of jobs to run consequensly
 */

#include "common.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <list>

typedef std::function< void() > Job;

class JobQueue : public NonCopyable
{
public:
	static JobQueue& instance();
	~JobQueue();

	//! Add a job to the queue; it will run as soon as possible
	void add(const Job& job);
	
	//! Add a job and a completion handler to the queue; the completion handler will run straight after the job
	void add(const Job& job, const Job& on_complete);

	//! Main function to run from the main program; it ends only after a call to stop
	void run();
	
	//! Terminate the run() call
	void stop();

private:
	JobQueue();

	typedef std::pair< Job, Job > JobPair;
	typedef std::list<JobPair> JobList;

	JobList queue_;
	volatile bool stop_;
	std::mutex guard_;
	std::condition_variable signal_;
};

