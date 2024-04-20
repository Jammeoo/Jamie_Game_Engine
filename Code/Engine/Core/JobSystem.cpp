#include "Engine/Core/JobSystem.hpp"

JobSystem* g_theJobSystem = nullptr;

JobSystem::JobSystem(JobSystemConfig const& config) : m_config(config)
{

}

JobSystem::~JobSystem()
{

}

void JobSystem::Startup()
{
	for (int threadIndex = 0; threadIndex < m_config.m_workerThreadsNum; threadIndex++)
	{
		JobWorker* worker = new JobWorker(this, threadIndex);
		m_workersList.push_back(worker);
	}
}

void JobSystem::BeginFrame()
{

}

void JobSystem::ShutDown()
{
	m_isQuiting = true;
	DeleteAllWorkers();
}

bool JobSystem::IsQuitting() const
{
	return m_isQuiting;
}

Job* JobSystem::ClaimJob()
{
	Job* job = nullptr;
	m_unclaimedJobsMutex.lock();
	if (!m_unclaimedJobList.empty()) 
	{
		job = m_unclaimedJobList.front();
		m_unclaimedJobList.pop();
		job->m_status = JOB_STATUS_WORKING;
	}
	m_unclaimedJobsMutex.unlock();
	return job;
}


Job* JobSystem::RetrieveCompletedJobs()
{
	Job* completedJob = nullptr;
	m_completedJobsMutex.lock();
	if (!m_completedJobList.empty()) 
	{
		completedJob = m_completedJobList.front();
		m_completedJobList.pop();
		completedJob->m_status = JOB_STATUS_RETRIEVED;
	}
	m_completedJobsMutex.unlock();
	return completedJob;
}



void JobSystem::ReportCompletedJobs(Job* job)
{
	m_completedJobsMutex.lock();
	m_completedJobList.push(job);
	job->m_status = JOB_STATUS_CONPLETED;
	m_completedJobsMutex.unlock();
}

void JobSystem::PostNewJob(Job* job)
{
	m_unclaimedJobsMutex.lock();
	m_unclaimedJobList.push(job);
	job->m_status = JOB_STATUS_QUEUED;
	m_unclaimedJobsMutex.unlock();
}

void JobSystem::CreateNewWorkers(int num)
{
	for (int threadIndex = 0; threadIndex < num; threadIndex++)
	{
		JobWorker* worker = new JobWorker(this, threadIndex);
		m_workersList.push_back(worker);
	}
}

void JobSystem::DeleteAllWorkers()
{
	for (int threadIndex = 0; threadIndex < m_config.m_workerThreadsNum; threadIndex++)
	{
		delete m_workersList[threadIndex];
	}
	m_workersList.clear();
}

JobWorker::JobWorker(JobSystem* jobSystem, int workerID) :m_jobSystem(jobSystem)
{
	m_threadID = workerID;
	m_thread = new std::thread(&JobWorker::ThreadMain, this);
}

JobWorker::~JobWorker()
{
	m_thread->join();
	delete m_thread;
	m_thread = nullptr;
}

void JobWorker::ThreadMain()
{
	while (!m_jobSystem->IsQuitting())
	{
		Job* claimedJob = m_jobSystem->ClaimJob();
		if (claimedJob) 
		{
			claimedJob->Execute();//actually 
			m_jobSystem->ReportCompletedJobs(claimedJob);
		}
		else 
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
}

