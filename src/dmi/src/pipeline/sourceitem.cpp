/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include "sourceitem.hpp"


dmi::SourceItem::SourceItem(const QString& category,
                            const QString& src,
                            const QString& ppt,
                            const QString& slicer,
                            const QString& vrange)
  : category_(category), source_(src), property_(ppt), slicer_(slicer), vrange_(vrange)
{
  int find = src.indexOf('*');
  if (find != -1)
  {
    for (int i = 0; i < 16; ++i)
    {
      auto n = QString::number(i);
      modules_.append(QString(src).replace(find, 1, n));
    }
  }
}

int dmi::SourceItem::nModules() const { return modules_.size(); }

QString dmi::SourceItem::getCategory() const { return category_; }

QString dmi::SourceItem::getSource() const { return source_; }

QStringList dmi::SourceItem::getModules() const { return modules_; };

QString dmi::SourceItem::getProperty() const { return property_; }

QString dmi::SourceItem::getSlicer() const { return slicer_; }

QString dmi::SourceItem::getVRange() const { return vrange_; }

dmi::SourceItem::iterator dmi::SourceItem::begin() { return modules_.begin(); }
dmi::SourceItem::iterator dmi::SourceItem::end() { return modules_.end(); }
dmi::SourceItem::const_iterator dmi::SourceItem::begin() const { return modules_.begin(); };
dmi::SourceItem::const_iterator dmi::SourceItem::end() const { return modules_.end(); };
dmi::SourceItem::const_iterator dmi::SourceItem::cbegin() const { return modules_.cbegin(); };
dmi::SourceItem::const_iterator dmi::SourceItem::cend() const { return modules_.cend(); };

const QSet<QString> dmi::SourceItem::exclusive_categories
{
  "DSSC",
  "LPD",
  "JungFrau"
};

const dmi::SourceItem::map_type dmi::SourceItem::categories
{
  {"DSSC",
    {
      "SCS_CDIDET_DSSC/CAL/APPEND_CORRECTED",
      "SCS_CDIDET_DSSC/CAL/APPEND_RAW",
      "SCS_DET_DSSC1M-1/DET/*CH0:xtdf"
    }
  },
  {"LPD",
    {
      "FXE_DET_LPD1M-1/CAL/APPEND_CORRECTED",
      "FXE_DET_LPD1M-1/DET/*CH0:xtdf",
    }
  },
  {"JungFrau",
    {
      "FXE_XAD_JF1M/DET/RECEIVER-1:display",
      "FXE_XAD_JF1M/DET/RECEIVER-2:display",
      "FXE_XAD_JF1M/CAL/APPEND",
      "FXE_XAD_JF1M/DET/RECEIVER-1:daqOutput",
      "FXE_XAD_JF1M/DET/RECEIVER-2:daqOutput",
      "FXE_XAD_JF500K/DET/RECEIVER:display",
      "FXE_XAD_JF500K/DET/RECEIVER:daqOutput",
    }
  },
  {"XGM",
    {
      "SCS_BLU_XGM/XGM/DOOCS",
      "SCS_BLU_XGM/XGM/DOOCS:output"
    }
  },
  {"MonoChromator",
    {
      "SA3_XTD10_MONO/MDL/PHOTON_ENERGY",
    }
  },
  {"Motor",
    {
      "SCS_ILH_LAS/DOOCS/PP800_PHASESHIFTER",
      "SCS_ILH_LAS/MOTOR/LT3",
    }
  },
  {"Magnet",
    {
      "SCS_CDIFFT_MAG/SUPPLY/CURRENT",
    }
  },
  {"Metadata",
    {
      "Metadata",
    }
  },
};

// Due to the chaotic naming convention, we provide property list for all
// the source name which ends with ":" + string.
const dmi::SourceItem::map_type dmi::SourceItem::properties
{
  {"DSSC",
    {
      "image.data",
    }
  },
  {"DSSC:xtdf",
    {
      "image.data",
    }
  },
  {"LPD",
    {
      "image.data",
    }
  },
  {"LPD:xtdf",
    {
      "image.data",
    }
  },
  {"JungFrau",
    {
      "data.adc",
    }
  },
  {"JungFrau:daqOutput",
    {
      "data.adc",
    }
  },
  {"JungFrau:display",
    {
      "data.adc",
    }
  },
  {"XGM:output",
    {
      "intensityTD",
      "sa1IntensityTD",
      "sa3IntensityTD",
    }
  },
  {"XGM",
    {
      "photonFlux",
    }
  },
  {"MonoChromator",
    {
      "actualEnergy",
    }
  },
  {"Motor",
    {
      "actualPosition",
    }
  },
  {"Magnet",
    {
      "actualCurrent",
    }
  },
  {"Metadata",
    {
      "timestamp.tid",
    }
  },
};
