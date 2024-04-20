#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>


enum JobStatus
{
	JOB_STATUS_NEW,
	JOB_STATUS_QUEUED, // Queued, waiting to be claimed by a worker thread
	JOB_STATUS_WORKING, // Claimed by a worker who is currently executing it
	JOB_STATUS_CONPLETED,//Completed by worker,placed into the completed list for the main thread to retrieve
	JOB_STATUS_RETRIEVED//Retrieved by the main thread, retired from Job system
};


class Job
{
public:
	Job() {}
	virtual ~Job() = default;
	virtual void Execute() = 0;
public:
	std::atomic<JobStatus> m_status = JOB_STATUS_NEW;
};

class JobSystem;
class JobWorker
{
public:
	JobWorker(JobSystem* jobSystem, int workerID);
	~JobWorker();
	void ThreadMain();
public:
	JobSystem* m_jobSystem = nullptr;
	int m_threadID = -1;
	std::thread* m_thread = nullptr;

};


struct JobSystemConfig  
{
	int m_workerThreadsNum = 0;
};

class JobSystem
{
	friend class JobWorker;
public:
	JobSystem(JobSystemConfig const& config);
	~JobSystem();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();


	//Main thread
	void PostNewJob(Job* job);		//Called by main thread to add job to ToDo List(give up ownership)
	Job* RetrieveCompletedJobs();   //Called by main thread to get a job out of system(take back ownership)

protected:
	bool IsQuitting()const;
	Job* ClaimJob();
	void ReportCompletedJobs(Job* job);

	void CreateNewWorkers(int num);
	void DeleteAllWorkers();

protected:

private:
	JobSystemConfig m_config;
	std::vector<JobWorker*> m_workersList;

	//Only Main thread can post a job
	std::queue<Job*> m_unclaimedJobList;			//List of jobs posted but not yet claimed by anyone
	std::mutex	     m_unclaimedJobsMutex;			

	std::queue <Job*> m_claimedJobList;				//List of jobs currently claimed by workers
	std::mutex	     m_claimedJobsMutex;

	std::queue<Job*> m_completedJobList;			//List of jobs finished, ready to be retrieved
	std::mutex	     m_completedJobsMutex;


	std::atomic<bool> m_isQuiting = false;
};