/* Copyright (C) 2018-present, Facebook, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once
#include <folly/MPMCQueue.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncPipe.h>
#include <memory>
#include <thread>
#include <vector>

#include "katran/lib/KatranLbStructs.h"
#include "katran/lib/MonitoringStructs.h"
#include "katran/lib/PcapMsgMeta.h"
#include "katran/lib/PcapWriter.h"

namespace folly {
class ScopedEventBaseThread;
}

namespace katran {

class KatranEventReader;
class PcapWriter;
/**
 * helper class which runs all introspection related routines
 */
class KatranMonitor {
 public:
  KatranMonitor() = delete;

  explicit KatranMonitor(const KatranMonitorConfig& config);

  ~KatranMonitor();

  void stopMonitor();

  void restartMonitor(uint32_t limit);

  PcapWriterStats getWriterStats();

  std::unique_ptr<folly::IOBuf> getEventBuffer(MonitoringEventId);

  /**
   * Enable event
   * Note: this does not start event loop nor does any internal synchronization.
   * It only marks the event as "enabled".
   */
  bool enableWriterEvent(MonitoringEventId event);

  /**
   * Disable event
   */
  bool disableWriterEvent(MonitoringEventId event);

  /**
   * Get enabled events
   */
  std::set<MonitoringEventId> getWriterEnabledEvents();

  /**
   * Get pacp storage format
   */
  PcapStorageFormat getStorageFormat() {
    return config_.storage;
  }

  /**
   * Tell the underlying pipe writer to use `writer`
   */
  void setAsyncPipeWriter(
      MonitoringEventId event,
      std::shared_ptr<folly::AsyncPipeWriter> writer);

  /**
   * Disable and destroy (if any) the pipe writer for the event
   */
  void unsetAsyncPipeWriter(MonitoringEventId event);

 private:
  /**
   * main config
   */
  KatranMonitorConfig config_;

  /**
   * event readers for introspection
   */
  std::vector<std::unique_ptr<KatranEventReader>> readers_;

  std::shared_ptr<PcapWriter> writer_;

  /**
   * queue toward writer
   */
  std::shared_ptr<folly::MPMCQueue<PcapMsgMeta>> queue_;
  /**
   * event base thread to run readers.
   */
  std::unique_ptr<folly::ScopedEventBaseThread> scopedEvb_;

  /**
   * thread which runs pcap writer
   */
  std::thread writerThread_;

  /**
   * map of iobufs where we store packets if IOBUF storage
   * is being used
   */
  std::unordered_map<MonitoringEventId, std::unique_ptr<folly::IOBuf>>
      buffers_;
};

} // namespace katran
