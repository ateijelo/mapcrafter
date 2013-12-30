/*
 * Copyright 2012-2016 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MULTITHREADING_H_
#define MULTITHREADING_H_

#include "../../compat/thread.h"
#include "../../renderer/tilerenderworker.h"
#include "../dispatcher.h"
#include "../workermanager.h"
#include "concurrentqueue.h"

#include <set>
#include <thread>
#include <vector>

namespace mapcrafter {
namespace thread {

class ThreadManager : public WorkerManager<renderer::RenderWork, renderer::RenderWorkResult> {
  public:
	ThreadManager();
	virtual ~ThreadManager();

    virtual bool getWork(renderer::RenderWork &work);
    virtual void workFinished(const renderer::RenderWork &work,
	void setFinished();

    bool getResult(renderer::RenderWorkResult &result);

  private:
    ConcurrentQueue<renderer::RenderWork> work_queue, work_extra_queue;
    ConcurrentQueue<renderer::RenderWorkResult> result_queue;

	bool finished;
    thread_ns::mutex mutex;
    thread_ns::condition_variable condition_wait_jobs, condition_wait_results;
};

class ThreadWorker {
  public:
    ThreadWorker(WorkerManager<renderer::RenderWork, renderer::RenderWorkResult> &manager,
                 const renderer::RenderContext &context);
    ~ThreadWorker();

	void operator()();
private:
	WorkerManager<renderer::RenderWork, renderer::RenderWorkResult>& manager;

  private:
	renderer::TileRenderWorker render_worker;
};

class MultiThreadingDispatcher : public Dispatcher {
public:
    MultiThreadingDispatcher(int threads);
	virtual ~MultiThreadingDispatcher();

  private:
			util::IProgressHandler* progress);
private:
	int thread_count;

    ThreadManager manager;
    std::vector<thread_ns::thread> threads;

	std::set<renderer::TilePath> rendered_tiles;
};

} /* namespace thread */
} /* namespace mapcrafter */

#endif /* MULTITHREADING_H_ */
