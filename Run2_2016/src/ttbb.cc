/*! Common part of stat models for the hh->bbtautau analyses.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <iostream>
#include <tuple>
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/CardWriter.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "HHStatAnalysis/Run2_2016/interface/ttbb.h"
#include "HHStatAnalysis/Core/interface/exception.h"
#include "HHStatAnalysis/Core/interface/Tools.h"
#include "HHStatAnalysis/Core/interface/TextIO.h"
#include "HHStatAnalysis/Core/interface/RootExt.h"
#include "HHStatAnalysis/Run2_2016/interface/CommonUncertainties.h"
#include "HHStatAnalysis/StatModels/interface/StatTools.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2_2016 {

const StatModel::v_str ttbb_base::ana_name = { "hh_ttbb" };
const StatModel::v_str ttbb_base::eras = { "13TeV" };
const std::string ttbb_base::bkg_TT = "TT";
const std::string ttbb_base::bkg_tW = "tW";
const std::string ttbb_base::bkg_W = "W";
const std::string ttbb_base::bkg_EWK = "EWK";
const std::string ttbb_base::bkg_ZH = "ZH";
const std::string ttbb_base::bkg_QCD = "QCD";
const std::string ttbb_base::bkg_DY_0b = "DY_0b";
const std::string ttbb_base::bkg_DY_1b = "DY_1b";
const std::string ttbb_base::bkg_DY_2b = "DY_2b";
const std::string ttbb_base::bkg_WW = "WW";
const std::string ttbb_base::bkg_WZ = "WZ";
const std::string ttbb_base::bkg_ZZ = "ZZ";
const StatModel::v_str ttbb_base::bkg_DY = { bkg_DY_0b, bkg_DY_1b, bkg_DY_2b };
const StatModel::v_str ttbb_base::bkg_VV = { bkg_WW, bkg_WZ, bkg_ZZ };
const StatModel::v_str ttbb_base::bkg_pure_MC = analysis::tools::join(bkg_tW, bkg_VV, bkg_W, bkg_EWK, bkg_ZH);
const StatModel::v_str ttbb_base::bkg_MC = analysis::tools::join(bkg_pure_MC, bkg_DY, bkg_TT);
const StatModel::v_str ttbb_base::bkg_all = analysis::tools::join(bkg_MC, bkg_QCD);

ttbb_base::ttbb_base(const StatModelDescriptor& _desc, const std::string& input_file_name) :
    StatModel(_desc, input_file_name), signal_processes({ desc.signal_process }),
    all_mc_processes(ch::JoinStr({ signal_processes, bkg_MC })),
    all_processes(ch::JoinStr({ signal_processes, bkg_all }))
{
}

void ttbb_base::AddSystematics(ch::CombineHarvester& cb)
{
    using ch::syst::SystMap;
    using CU = CommonUncertainties;
    static constexpr double unc_thr = 0.005;

    CU::lumi().ApplyGlobal(cb, signal_processes, bkg_pure_MC, bkg_TT);
    CU::QCDscale_W().ApplyGlobal(cb, bkg_W);
    CU::QCDscale_WW().ApplyGlobal(cb, bkg_WW);
    CU::QCDscale_WZ().ApplyGlobal(cb, bkg_WZ);
    CU::QCDscale_ZZ().ApplyGlobal(cb, bkg_ZZ);
    CU::QCDscale_EWK().ApplyGlobal(cb, bkg_EWK);
    CU::QCDscale_ttbar().ApplyGlobal(cb, bkg_TT);
    CU::QCDscale_tW().ApplyGlobal(cb, bkg_tW);
    CU::QCDscale_ZH().ApplyGlobal(cb, bkg_ZH);

    CU::scale_j().Apply(cb, all_mc_processes);
//    CU::scale_b().ApplyGlobal(cb, signal_processes, bkg_pure_MC, bkg_TT);

    static constexpr double eff_b_unc = 0.02;
    CU::eff_b().Apply(cb, eff_b_unc, bkg_DY_1b, bkg_VV, bkg_EWK, bkg_ZH, bkg_tW);
    CU::eff_b().Apply(cb, eff_b_unc * std::sqrt(2.), signal_processes, bkg_DY_2b, bkg_TT);

    CU::eff_e().Channel("eTau").Apply(cb, CU::eff_e().up_value, all_mc_processes);
    CU::eff_m().Channel("muTau").Apply(cb, CU::eff_m().up_value, all_mc_processes);
    CU::eff_t().Channels({"eTau", "muTau"}).Apply(cb, CU::eff_t().up_value, all_mc_processes);
    CU::eff_t().Channel("tauTau").Apply(cb, CU::eff_t().up_value * std::sqrt(2.), all_mc_processes);
    CU::scale_t().Apply(cb, all_mc_processes);

    CU::topPt().Apply(cb, bkg_TT);

    static const size_t DYUncDim = 4;
    TMatrixD dy_unc_cov(DYUncDim, DYUncDim);
    dy_unc_cov[0][0] = 2.727e-06;
    dy_unc_cov[1][1] = 0.0002202;
    dy_unc_cov[2][2] = 0.0007727;
    dy_unc_cov[3][3] = 0.0004435;
    dy_unc_cov[0][1] = dy_unc_cov[1][0] = -6.962e-06;
    dy_unc_cov[0][2] = dy_unc_cov[2][0] = 6.771e-06;
    dy_unc_cov[0][3] = dy_unc_cov[3][0] = -8.506e-06;
    dy_unc_cov[1][2] = dy_unc_cov[2][1] = -0.0001962;
    dy_unc_cov[1][3] = dy_unc_cov[3][1] = -3.962e-05;
    dy_unc_cov[2][3] = dy_unc_cov[3][2] = -0.0001762;

    TVectorD dy_sf(DYUncDim);
    dy_sf[0] = 1.05357;
    dy_sf[1] = 1.09229;
    dy_sf[2] = 1.06439;
    dy_sf[3] = 0.933618;

    auto dy_w_inv = stat_tools::ComputeWhiteningMatrix(dy_unc_cov).Invert();
    std::cout << "ttbb: inverse whitening matrix for DY sf covariance matrix" << std::endl;
    dy_w_inv.Print();

    for(size_t n = 0; n < DYUncDim; ++n) {
        std::ostringstream ss_unc_name;
        ss_unc_name << "DY_norm_unc_" << n;
        const Uncertainty DY_norm_unc(ss_unc_name.str(), CorrelationRange::Analysis, UncDistributionType::lnN);
        for(size_t k = 0; k < bkg_DY.size(); ++k) {
            const double unc_value = dy_w_inv[k][n] / dy_sf[k];
//            if(std::abs(unc_value) >= unc_thr)
            DY_norm_unc.Apply(cb, unc_value, bkg_DY.at(k));
        }
    }

    static const std::map<std::string, std::tuple<double, double, double, double>> qcd_os_ss_sf = {
        { "eTau", std::make_tuple(1.24, 0.05, 1.87, 0.13 /*2.663, 0.167*/) },
        { "muTau", std::make_tuple(1.363, 0.055, 2.108, 0.149 /*4.252, 0.403*/) },
        { "tauTau", std::make_tuple(1.6, 0.1, 1.521, 0.172 /*2.729, 0.260*/) }
    };
    const Uncertainty qcd_norm("qcd_norm", CorrelationRange::Category, UncDistributionType::lnN);
    for(const auto& channel : desc.channels) {
        for(const auto& category : desc.categories) {
            const Yield qcd_yield = GetBackgroundYield(bkg_QCD, channel, category);
            const double ss_qcd_yield = qcd_yield.value / std::get<0>(qcd_os_ss_sf.at(channel));
            const double rel_error = 1 / std::sqrt(ss_qcd_yield);
            if(rel_error >= unc_thr)
                qcd_norm.Channel(channel).Category(category).Apply(cb, rel_error, bkg_QCD);
        }
    }

    const Uncertainty qcd_sf_unc("qcd_sf_unc", CorrelationRange::Channel, UncDistributionType::lnN);
    for(const auto& sf_entry : qcd_os_ss_sf) {
        const double rel_stat_unc = std::get<1>(sf_entry.second) / std::get<0>(sf_entry.second);
        double rel_ext_unc = 0;
//        if(std::abs(std::get<2>(sf_entry.second) - std::get<0>(sf_entry.second)) >
//                std::get<1>(sf_entry.second) + std::get<3>(sf_entry.second))
//            rel_ext_unc = std::get<2>(sf_entry.second) / std::get<0>(sf_entry.second) - 1;
        const double cmb_unc = std::sqrt(std::pow(rel_stat_unc, 2) + std::pow(rel_ext_unc, 2));
        const double cmb_unc_up = rel_ext_unc > 0 ? cmb_unc : rel_stat_unc;
        const double cmb_unc_down = rel_ext_unc < 0 ? -cmb_unc : -rel_stat_unc;
        qcd_sf_unc.Channel(sf_entry.first).Apply(cb, std::make_pair(cmb_unc_up, cmb_unc_down), bkg_QCD);
        const auto prev_precision = std::cout.precision();
        std::cout << std::setprecision(4) << "ttbb/" << sf_entry.first << ": QCD OS/SS scale factor uncertainties:\n"
                  << "\tstat unc: +/- " << rel_stat_unc * 100 << "%\n"
                  << "\textrapolation unc: " << rel_ext_unc * 100 << "%\n"
                  << "\ttotal unc: +" << cmb_unc_up * 100 << "% / " << cmb_unc_down * 100 << "%."
                  << std::setprecision(prev_precision) << std::endl;
    }
}

} // namespace Run2_2016
} // namespace stat_models
} // namespace hh_analysis
