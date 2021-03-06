/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APU_AUDIO_SYSTEM_H_
#define XENIA_APU_AUDIO_SYSTEM_H_

#include <atomic>
#include <mutex>
#include <queue>

#include "xenia/emulator.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
class XHostThread;
}  // namespace kernel
}  // namespace xe

namespace xe {
namespace apu {

class AudioDriver;

class AudioSystem {
 public:
  virtual ~AudioSystem();

  Emulator* emulator() const { return emulator_; }
  Memory* memory() const { return memory_; }
  cpu::Processor* processor() const { return processor_; }

  virtual X_STATUS Setup();
  virtual void Shutdown();

  X_STATUS RegisterClient(uint32_t callback, uint32_t callback_arg,
                          size_t* out_index);
  void UnregisterClient(size_t index);
  void SubmitFrame(size_t index, uint32_t samples_ptr);

  virtual X_STATUS CreateDriver(size_t index, HANDLE semaphore,
                                AudioDriver** out_driver) = 0;
  virtual void DestroyDriver(AudioDriver* driver) = 0;

  // TODO(gibbed): respect XAUDIO2_MAX_QUEUED_BUFFERS somehow (ie min(64,
  // XAUDIO2_MAX_QUEUED_BUFFERS))
  static const size_t kMaximumQueuedFrames = 64;

 protected:
  virtual void Initialize();

 private:
  void WorkerThreadMain();

 protected:
  AudioSystem(Emulator* emulator);

  Emulator* emulator_;
  Memory* memory_;
  cpu::Processor* processor_;

  std::atomic<bool> worker_running_;
  kernel::object_ref<kernel::XHostThread> worker_thread_;

  xe::mutex lock_;

  static const size_t kMaximumClientCount = 8;

  struct {
    AudioDriver* driver;
    uint32_t callback;
    uint32_t callback_arg;
    uint32_t wrapped_callback_arg;
  } clients_[kMaximumClientCount];

  HANDLE client_semaphores_[kMaximumClientCount];
  HANDLE shutdown_event_;  // Event is always there in case we have no clients.
  HANDLE wait_handles_[kMaximumClientCount + 1];
  std::queue<size_t> unused_clients_;
};

}  // namespace apu
}  // namespace xe

#endif  // XENIA_APU_AUDIO_SYSTEM_H_
