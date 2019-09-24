/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_SOURCEITEM_HPP
#define KARABO_BRIDGE_VF_SOURCEITEM_HPP

#include <QDebug>
#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>


namespace vf
{

class SourceItem
{
public:

  using map_type = QMap<QString, QStringList>;

  static const map_type categories;
  static const map_type properties;
  static const QSet<QString> exclusive_categories;
  static const map_type modules;

public:
  SourceItem(const QString& category, const QString& src, const QString& ppt);

  int nModules() const;

  friend bool operator==(const SourceItem& lhs, const SourceItem& rhs);

  QString getCategory() const;
  QString getSource() const;
  QStringList getModules() const;
  QString getProperty() const;

  using iterator = QStringList::iterator;
  using const_iterator = QStringList::const_iterator;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;

private:
  QString category_;
  QString source_;
  QStringList modules_;
  QString property_;
};

inline bool operator==(const SourceItem& lhs, const SourceItem& rhs)
{
  return lhs.category_ == rhs.category_
         && lhs.source_ == rhs.source_
         && lhs.modules_ == rhs.modules_
         && lhs.property_ == rhs.property_;
}

inline QDebug operator<<(QDebug debug, const SourceItem &item)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "SourceItem(" << item.getCategory() << ", "
                  << item.getSource() << " {" << item.getModules() << "}, "
                  << item.getProperty() << ")";
  return debug;
}

inline uint qHash(const SourceItem& item)
{
  return qHash(item.getCategory())
    ^ qHash(item.getSource())
    ^ qHash(item.getModules())
    ^ qHash(item.getProperty());
}

} // namespace vf

#endif //KARABO_BRIDGE_VF_SOURCEITEM_HPP
