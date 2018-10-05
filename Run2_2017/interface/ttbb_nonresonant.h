/*! Stat model for h->hh->bbtautau.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#pragma once

#include "HHStatAnalysis/Run2_2017/interface/ttbb.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2_2017 {

class ttbb_nonresonant : public ttbb_base {
public:
    using ttbb_base::ttbb_base;
    virtual void CreateDatacards(const std::string& output_path) override;

protected:
    virtual void AddSystematics(ch::CombineHarvester& combine_harvester) override;
};

} // namespace Run2_2017
} // namespace stat_models
} // namespace hh_analysis
