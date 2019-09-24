/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <chrono>

#include <QDebug>
#include <QMutexLocker>

#include "vf_databroker.hpp"
#include <vf_mainwindow.hpp>


vf::DataBroker::DataBroker(QObject *parent)
  : QThread(parent),
    acquiring_(false),
    source_type_(DataSourceType::file),
    mutex_(),
    queue_(std::make_shared<PipeLineQueue>())
{
  queue_->set_capacity(QUEUE_CAPACITY);
}

void vf::DataBroker::run()
{
  acquiring_ = true;

  int timeout = 100; // 100 ms timeout

  karabo_bridge::Client client(timeout);
  try
  {
    client.connect(endpoint_);
    qDebug() << "Connected to server: " << endpoint_.c_str();
  } catch(const std::exception& e)
  {
    qDebug() << "Failed to connect to server: " << e.what();
    return;
  }

  int count = 0;
  double total_daq_time = 0;
  const int interval = 20;
  while (acquiring_)
  {
    if (isInterruptionRequested()) return;

    auto start = std::chrono::high_resolution_clock::now();

    std::map<std::string, karabo_bridge::kb_data> data_pkg = client.next();
    if (data_pkg.empty()) continue;

    total_daq_time += std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now() - start).count();
    ++count;
    if (count >= interval)
    {
      qDebug() << "\nAverage data acquisition time: " <<  total_daq_time / (1000. * count) << " ms";
      count = 0;
      total_daq_time = 0;
    }

    // update available sources
    QStringList available_srcs;
    for (auto& src : data_pkg)
    {
      available_srcs.append(QString::fromStdString(src.first));
    }
    emit newSources(available_srcs);

    // extract requested data
    QMutexLocker locker(&mutex_);
    for (auto& item : source_items_)
    {
      std::string ctg = item.getCategory().toStdString();
      std::vector<void*> item_data;
      MetaData meta;
      meta.source_category = ctg;

      if (item.nModules() > 0)
      {
        for (auto it = item.cbegin(); it != item.cend(); ++it)
        {
          std::string src = it->toStdString();
          auto m_it = data_pkg.find(src);
          if (m_it != data_pkg.end())
          {
            item_data.push_back(m_it->second.array[item.getProperty().toStdString()].data());
            meta.tid = m_it->second.metadata["timestamp.tid"].as<uint64_t>();
            meta.source_name = src;
          } else
          {
            item_data.push_back(nullptr);
          }
        }
      } else
      {
        std::string src = item.getSource().toStdString();
        auto it = data_pkg.find(src);
        if (it != data_pkg.end())
        {
          item_data.push_back(it->second.array.at(item.getProperty().toStdString()).data());
          meta.tid = it->second.metadata["timestamp.tid"].as<uint64_t>();
          meta.source_name = src;
        }
      }

      if (! item_data.empty())
      {
        queue_->push(std::make_pair(std::move(meta), std::move(item_data)));
      }
    }

    // maintain the data lifetime
    kb_queue_.emplace_back(std::move(data_pkg));
  }
}

void vf::DataBroker::setEndpoint(std::string endpoint) { endpoint_ = std::move(endpoint); };

void vf::DataBroker::setSourceType(DataSourceType src_type) { source_type_ = src_type; }

void vf::DataBroker::stop() { acquiring_ = false; }

void vf::DataBroker::updateSources(const SourceItem& item, bool checked)
{
  QMutexLocker locker(&mutex_);
  if (checked) source_items_.insert(item);
  else
    source_items_.remove(item);

  qDebug() << source_items_;
}

std::shared_ptr<vf::PipeLineQueue> vf::DataBroker::outputChannel()
{
  return queue_;
}

void vf::DataBroker::dataProcessed()
{
  kb_queue_.pop_front();
}