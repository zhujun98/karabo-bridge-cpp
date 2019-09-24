/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#ifndef KARABO_BRIDGE_PIPE_BRIDGE_HPP
#define KARABO_BRIDGE_PIPE_BRIDGE_HPP

#include <memory>
#include <deque>

#include <tbb/concurrent_queue.h>

#include <QThread>
#include <QString>
#include <QPixmap>
#include <QSet>
#include <QMutex>

#include <karabo-bridge/kb_client.hpp>
#include "vf_sourceitem.hpp"
#include "vf_pipeline_data.hpp"
#include "vfai/vf_config.hpp"

namespace vf
{

class DataBroker : public QThread
{
  Q_OBJECT

  void run() override;

  static constexpr std::size_t QUEUE_CAPACITY = 5;

public:
  explicit DataBroker(QObject* parent = nullptr);

  std::shared_ptr<PipeLineQueue> outputChannel();

public slots:
  // set the TCP address of the endpoint
  void setEndpoint(std::string endpoint);

  void setSourceType(DataSourceType src_type);

  void stop();

  void updateSources(const SourceItem& item, bool checked);

  void dataProcessed();

signals:
  // emitted when a new 1D data is ready
  void newLine();
  // emitted when new data package received from the bridge
  void newSources(const QStringList& sources);

private:
  bool acquiring_;

  std::string endpoint_; // TCP address of the endpoint
  DataSourceType source_type_;
  QSet<SourceItem> source_items_; // store requested data sources

  QMutex mutex_;

  std::shared_ptr<PipeLineQueue> queue_;
  std::deque<std::map<std::string, karabo_bridge::kb_data>> kb_queue_;
};

} //vf


#endif //KARABO_BRIDGE_PIPE_BRIDGE_HPP
