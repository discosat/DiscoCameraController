#include "message_queue.hpp"
#include "camera_controller.hpp"
#include "common.hpp"
#include <functional>
#include <string>

#ifndef CSPSERVER_H
#define CSPSERVER_H

void server(void);

void server_init(std::function<void(CaptureMessage params, CameraController*, MessageQueue*, std::vector<VmbCPP::CameraPtr>)> callback,
				CameraController* vmbProvider, MessageQueue* mq, std::vector<VmbCPP::CameraPtr> cameras);

#endif