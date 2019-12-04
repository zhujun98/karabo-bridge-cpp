/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#ifndef KBCPP_DMI_SOURCEITEM_HPP
#define KBCPP_DMI_SOURCEITEM_HPP

#include <QDebug>
#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>


namespace dmi
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
  SourceItem(const QString& category,
             const QString& src,
             const QString& ppt,
             const QString& slicer,
             const QString& vrange);

  int nModules() const;

  friend bool operator==(const SourceItem& lhs, const SourceItem& rhs);

  QString getCategory() const;
  QStringList getModules() const;
  QString getSource() const;
  QString getProperty() const;
  QString getSlicer() const;
  QString getVRange() const;

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
  QString slicer_;
  QString vrange_;
};

inline bool operator==(const SourceItem& lhs, const SourceItem& rhs)
{
  return lhs.category_ == rhs.category_
         && lhs.source_ == rhs.source_
         && lhs.modules_ == rhs.modules_
         && lhs.property_ == rhs.property_
         && lhs.slicer_ == rhs.slicer_
         && lhs.vrange_ == rhs.vrange_;
}

inline QDebug operator<<(QDebug debug, const SourceItem &item)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "SourceItem(" << item.getCategory() << ", "
                  << item.getSource() << " {" << item.getModules() << "}, "
                  << item.getProperty() << ", " << item.getSlicer() << ", "
                  << item.getVRange() << ")";
  return debug;
}

inline uint qHash(const SourceItem& item)
{
  return qHash(item.getCategory())
    ^ qHash(item.getSource())
    ^ qHash(item.getModules())
    ^ qHash(item.getProperty())
    ^ qHash(item.getSlicer())
    ^ qHash(item.getVRange());
}

} // namespace dmi

#endif //KBCPP_DMI_SOURCEITEM_HPP
