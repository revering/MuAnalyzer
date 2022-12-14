// -*- C++ -*-
//
// Package:    DarkPhoton/MuAnalyzer
// Class:      MuAnalyzer
//
/**\class MuAnalyzer MuAnalyzer.cc DarkPhoton/MuAnalyzer/plugins/MuAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Michael Revering 
//         Created:  Friday, 18 Nov 2022 13:34:23 GMT
//
//
// system include files
#include <memory>
#include <iomanip>
#include <iostream>

#include <TLorentzVector.h>
#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Utilities/interface/transform.h"

#include "DataFormats/Math/interface/Vector3D.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "DataFormats/TrackReco/interface/Track.h"

//for Standalone Muon Tracking
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

// for vertexing
#include "FWCore/Framework/interface/ESHandle.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"

// for Global Coordinates
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "DataFormats/DetId/interface/DetId.h"

//Triggers
#include "FWCore/Common/interface/TriggerNames.h"

//Pileup
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

//Jets
#include "DataFormats/JetReco/interface/CaloJet.h"

//Event Weights
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "DarkPhoton/MuAnalyzer/interface/Histograms.h"
#include "DarkPhoton/MuAnalyzer/interface/EventInfo.h"
#include "DarkPhoton/MuAnalyzer/interface/Muons.h"
#include "DarkPhoton/MuAnalyzer/interface/Tracks.h"
#include "DarkPhoton/MuAnalyzer/interface/ECAL.h"

// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

class MuAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit MuAnalyzer(const edm::ParameterSet&);
  ~MuAnalyzer();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;

  // ----------member data ---------------------------

  double MatchTrackToGenMuon(const edm::Event& iEvent,
                             const reco::Track* selTrack,
                             edm::EDGetToken m_genParticleToken);
  bool MatchTrackToMuon(const edm::Event& iEvent,
                        math::XYZTLorentzVectorD mother,
                        Muons myMuons,
                        EventInfo info);
  int PairTagAndProbe(
      const edm::Event& iEvent,
      const edm::EventSetup& iSetup,
      Tracks myTracks,
      const reco::Muon* Tag,
      ECAL myECAL,
      edm::ESHandle<TransientTrackBuilder> transientTrackBuilder,
      edm::EDGetTokenT<std::vector<reco::Track>> trackCollection_label,
      edm::Handle<reco::VertexCollection> vtxHandle,
      edm::EDGetTokenT<EcalRecHitCollection> reducedEndcapRecHitCollection_Label,
      edm::EDGetTokenT<EcalRecHitCollection> reducedBarrelRecHitCollection_Label);
  std::vector<double> GetPuWeight(edm::Handle<std::vector<PileupSummaryInfo>> hPileupInfoProduct);

  edm::EDGetToken m_recoMuonToken;
  edm::EDGetTokenT<std::vector<reco::Track>> trackCollection_label;
  edm::EDGetTokenT<std::vector<reco::Vertex>> primaryVertices_Label;
  edm::EDGetToken m_genParticleToken;
  edm::EDGetToken m_trigResultsToken;
  edm::EDGetToken m_trigEventToken;
  edm::EDGetTokenT<GenEventInfoProduct> m_genInfoToken;
  std::vector<std::string> m_muonPathsToPass;
  const reco::Track* selectedTrack;
  const reco::Muon* selectedMuon;
  edm::EDGetTokenT<EcalRecHitCollection> reducedEndcapRecHitCollection_Label;
  edm::EDGetTokenT<EcalRecHitCollection> reducedBarrelRecHitCollection_Label;
  edm::EDGetToken m_caloJet_label;
  edm::EDGetToken m_PUInfoToken;
  edm::EDGetToken m_theSTAMuonLabel;
  bool m_isMC;
  bool pairedTagAndProbe_;
  double weight_;
  double nPUmean_;
  double standaloneE;
  double pairVtxChi;
  double diMuonMass;
  double probeTIP;
  double probeLIP;
  //pileup reweighting
  TH1F* fPUDataHist_;
  TH1F* fPUDataUpHist_;
  TH1F* fPUDataDownHist_;
  TH1F* fPUMCHist_;
  TH1F* puWeightHist_;
  TH1F* puWeightUpHist_;
  TH1F* puWeightDownHist_;

  edm::Handle<reco::CaloJetCollection> caloJets;
  Histograms pairedEvents;
  Histograms genMatched;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
MuAnalyzer::MuAnalyzer(const edm::ParameterSet& iConfig)
    : m_recoMuonToken(consumes<std::vector<reco::Muon>>(iConfig.getParameter<edm::InputTag>("recoMuons"))),
      trackCollection_label(consumes<std::vector<reco::Track>>(iConfig.getParameter<edm::InputTag>("tracks"))),
      primaryVertices_Label(
          consumes<std::vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("primaryVertices"))),
      reducedEndcapRecHitCollection_Label(
          consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("EERecHits"))),
      reducedBarrelRecHitCollection_Label(
          consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("EBRecHits"))),
      m_caloJet_label(consumes<reco::CaloJetCollection>(iConfig.getParameter<edm::InputTag>("CaloJetSource"))),
      m_theSTAMuonLabel(consumes<std::vector<reco::Track>>(iConfig.getParameter<edm::InputTag>("StandAloneTracks"))),
      m_isMC(iConfig.getUntrackedParameter<bool>("isMC", true))
  {
  usesResource("TFileService");
  edm::Service<TFileService> fs;
  //now do what ever initialization is needed
  if (m_isMC) {
    m_genParticleToken = consumes<std::vector<reco::GenParticle>>(iConfig.getParameter<edm::InputTag>("genParticles"));
    m_genInfoToken = consumes<GenEventInfoProduct>(edm::InputTag("generator"));
    m_PUInfoToken = consumes<std::vector<PileupSummaryInfo>>(iConfig.getParameter<edm::InputTag>("edmPileupInfo"));
    }
  m_trigResultsToken = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("trigResults"));
  m_trigEventToken = consumes<trigger::TriggerEvent>(edm::InputTag("hltTriggerSummaryAOD"));
  m_muonPathsToPass = iConfig.getParameter<std::vector<std::string>>("muonPathsToPass");
  if(m_isMC){
    TFile* puFile = TFile::Open(
        "/local/cms/user/revering/datafiles/PileupHists/PileupHistogram-goldenJSON-13tev-2018-69200ub-99bins.root");
    fPUDataHist_ = (TH1F*)puFile->Get("pileup");
    fPUDataHist_->SetDirectory(0);
    puFile->Close();
    TFile* puUpFile = TFile::Open(
        "/local/cms/user/revering/datafiles/PileupHists/PileupHistogram-goldenJSON-13tev-2018-72400ub-99bins.root");
    fPUDataUpHist_ = (TH1F*)puUpFile->Get("pileup");
    fPUDataUpHist_->SetDirectory(0);
    puUpFile->Close();
    TFile* puDownFile = TFile::Open(
        "/local/cms/user/revering/datafiles/PileupHists/PileupHistogram-goldenJSON-13tev-2018-66000ub-99bins.root");
    fPUDataDownHist_ = (TH1F*)puDownFile->Get("pileup");
    fPUDataDownHist_->SetDirectory(0);
    puDownFile->Close();

    TFile* MCPuFile = TFile::Open("/local/cms/user/revering/datafiles/PileupHists/DYJets_Private_MCWeights.root");
    fPUMCHist_ = (TH1F*)MCPuFile->Get("MCWeights/TrueNumInteractions");
    fPUDataHist_->Scale(1. / fPUDataHist_->Integral());
    fPUDataUpHist_->Scale(1. / fPUDataUpHist_->Integral());
    fPUDataDownHist_->Scale(1. / fPUDataDownHist_->Integral());
    fPUMCHist_->Scale(1. / fPUMCHist_->Integral());
    fPUDataHist_->Divide(fPUMCHist_);
    fPUDataUpHist_->Divide(fPUMCHist_);
    fPUDataDownHist_->Divide(fPUMCHist_);
    puWeightHist_ = (TH1F*)fPUDataHist_->Clone();
    puWeightUpHist_ = (TH1F*)fPUDataUpHist_->Clone();
    puWeightDownHist_ = (TH1F*)fPUDataDownHist_->Clone();
  }
  pairedEvents.book(fs->mkdir("pairedEvents"),m_isMC);
  genMatched.book(fs->mkdir("genMatched"),m_isMC);
}

MuAnalyzer::~MuAnalyzer() {
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//

// ------------ method called for each event  ------------
void MuAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  using namespace std;
  using namespace reco;
  using namespace pat;
  Muons myMuons;
  EventInfo myEventInfo;
  Tracks myTracks;
  ECAL myECAL;
  EventInfo info;
  if(!info.passTriggers(iEvent,m_trigResultsToken,m_trigEventToken,m_muonPathsToPass)) return;
  pairedEvents.ResetCutFlow();
  if(m_isMC)
  {
	edm::Handle<GenEventInfoProduct> eventInfo;
	iEvent.getByToken(m_genInfoToken, eventInfo);
	weight_ = eventInfo->weight();
	pairedEvents.IncCutFlow(weight_);
  }
  else{
    weight_ = 1.;
    pairedEvents.IncCutFlow(weight_);
  }
 
  //Do pileup reweighting if necessary 
  if (m_isMC) {
    edm::Handle<std::vector<PileupSummaryInfo>> hPileupInfoProduct;
    iEvent.getByToken(m_PUInfoToken, hPileupInfoProduct);
    assert(hPileupInfoProduct.isValid());
    std::vector<double> PUWeight = GetPuWeight(hPileupInfoProduct);
    weight_ = weight_ * PUWeight[0];
    info.PuUpDownWeights.push_back(PUWeight[1]);
    info.PuUpDownWeights.push_back(PUWeight[2]);
    info.nPUmean = nPUmean_;
    info.pileupWeight = PUWeight[0];
  }
  info.eventWeight = weight_;
  edm::Handle<reco::TrackCollection> staTracks;
  iEvent.getByToken(m_theSTAMuonLabel, staTracks);
  edm::Handle<reco::VertexCollection> vtxHandle;
  iEvent.getByToken(primaryVertices_Label, vtxHandle);
  edm::ESHandle<MagneticField> theMGField;
  iSetup.get<IdealMagneticFieldRecord>().get(theMGField);
  edm::ESHandle<GlobalTrackingGeometry> theTrackingGeometry;
  iSetup.get<GlobalTrackingGeometryRecord>().get(theTrackingGeometry);
 
  //Check reco muons in event, skip if no good tag muons are found.
  int muProgress = myMuons.SelectMuons(iEvent, m_recoMuonToken);
  for (int i = 0; i < muProgress; i++) {
    pairedEvents.IncCutFlow(info.eventWeight);
  }
  if (muProgress < 4) {
    return;
  }
  int maxTrackProgress = -1;
  pairedTagAndProbe_=false;
  iEvent.getByToken(m_caloJet_label, caloJets);
  edm::ESHandle<TransientTrackBuilder> transientTrackBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder", transientTrackBuilder);
  for (std::vector<const reco::Muon*>::const_iterator muon = myMuons.selectedMuons.begin();
       muon != myMuons.selectedMuons.end();
       muon++) {
    myTracks.selectedTracks.clear();
    myTracks.trackSelected = false;
    int trackProgress = myTracks.SelectTracks(iEvent, trackCollection_label, vtxHandle, *muon, transientTrackBuilder);
    if (myTracks.trackSelected) {
      trackProgress = trackProgress + PairTagAndProbe(iEvent,
                                                      iSetup,
                                                      myTracks,
                                                      *muon,
                                                      myECAL,
                                                      transientTrackBuilder,
                                                      trackCollection_label,
                                                      vtxHandle,
                                                      reducedEndcapRecHitCollection_Label,
                                                      reducedBarrelRecHitCollection_Label);
    }
    if (trackProgress > maxTrackProgress) {
      maxTrackProgress = trackProgress;
      if (m_isMC) {
        edm::Handle<reco::GenParticleCollection> genParticles;
        iEvent.getByToken(m_genParticleToken, genParticles);
        info.filledWeights = true;
        info.muRocWeight = myMuons.MuonRoccorWeight((*muon), genParticles);
        info.muIdWeights = myMuons.MuonTightIDWeight((*muon)->pt(), (*muon)->eta());
        info.muIsoWeights = myMuons.MuonTightIsoWeight((*muon)->pt(), (*muon)->eta());
        info.muTrigWeights = myMuons.MuonTrigWeight((*muon)->pt(), (*muon)->eta());
      }
    }
    if(pairedTagAndProbe_){break;}
  }

  if(m_isMC){
  info.eventWeight =
      info.eventWeight * info.muIdWeights[0] * info.muIsoWeights[0] * info.muTrigWeights[0] * info.muRocWeight;
  }
  if (pairedTagAndProbe_) {
    if (m_isMC) {
      std::vector<double> probeIsoWeights = myMuons.MuonTightIsoWeight(selectedTrack->pt(), selectedTrack->eta());
      info.eventWeight = info.eventWeight * probeIsoWeights[0];
      info.muIsoWeights[0] = info.muIsoWeights[0] * probeIsoWeights[0];
      info.muIsoWeights[1] = info.muIsoWeights[1] * probeIsoWeights[1];
      info.muIsoWeights[2] = info.muIsoWeights[2] * probeIsoWeights[2];
    }
  }
  for (int i = 0; i < maxTrackProgress; i++) {
    pairedEvents.IncCutFlow(info.eventWeight);
  }
  if (!pairedTagAndProbe_) {
    return;
  }
  //Make sure the tag muon would have fired the trigger
  if(!info.matchTagAndTrigger(selectedMuon)){return;}
  pairedEvents.IncCutFlow(info.eventWeight);
  //Reject events where another probe also could pair with the selected tag
  for (std::vector<const reco::Track*>::const_iterator iTrack = myTracks.selectedTracks.begin();
       iTrack != myTracks.selectedTracks.end();
       ++iTrack) {
    if (deltaR((*iTrack)->eta(), (*iTrack)->phi(), selectedTrack->eta(), selectedTrack->phi()) < 0.3)
      continue;
    if (selectedMuon->charge() == (*iTrack)->charge())
      continue;
    if (myTracks.PairTracks(iTrack, selectedMuon->globalTrack(), transientTrackBuilder)==3) {
      double probeTrackIso = myTracks.GetIsolation(iEvent, trackCollection_label, 0.3, vtxHandle, *iTrack);
      double probeEcalIso = myECAL.GetIsolation(iEvent,
                                                iSetup,
                                                reducedEndcapRecHitCollection_Label,
                                                reducedBarrelRecHitCollection_Label,
                                                transientTrackBuilder->build(*iTrack));
      if ((probeTrackIso / (*iTrack)->pt()) > 0.2 || probeEcalIso > 30.)
        continue;
      //Iso <0 means that the isolation calculation failed
      if (probeTrackIso < 0) {
        continue;
      }
      return;
    }
  }
  pairedEvents.IncCutFlow(info.eventWeight);
  info.nTagMuons = myMuons.selectedMuons.size();
  info.probeTrackIso =
      myTracks.GetIsolation(iEvent, trackCollection_label, 0.3, vtxHandle, selectedTrack) / selectedTrack->pt();
  info.probeReducedChi = selectedTrack->chi2() / selectedTrack->ndof();
  info.tagMuonEta = selectedMuon->eta();
  info.pairVtxChi = pairVtxChi;
  info.probeEcalIso = myECAL.GetIsolation(iEvent,
                                          iSetup,
                                          reducedEndcapRecHitCollection_Label,
                                          reducedBarrelRecHitCollection_Label,
                                          transientTrackBuilder->build(selectedTrack));
  info.tagMuonPt = selectedMuon->pt();
  info.probeTrackEta = selectedTrack->eta();
  info.probeCharge = selectedTrack->charge();
  info.probeTrackP = selectedTrack->p();
  info.probeTrackPt = selectedTrack->pt();
  info.probeTrackPhi = selectedTrack->phi();
  info.dxy = selectedTrack->dxy();
  info.dca = probeTIP;
  info.dcal = probeLIP;
  info.dsz = selectedTrack->dsz();
  info.diMuonMass = diMuonMass;
  GlobalPoint FillerPoint;
  double staMinDr = 7.;
  if(m_isMC){
     info.minGenMuDr = MatchTrackToGenMuon(iEvent,selectedTrack, m_genParticleToken);
  }
  double stadEoverE = 10.;
  double staE = -10;
  double closestapproach = 100.;
  if (staTracks->size() > 0) {
    for (reco::TrackCollection::const_iterator staTrack = staTracks->begin(); staTrack != staTracks->end();
         ++staTrack) {
      reco::TransientTrack track = transientTrackBuilder->build(*staTrack);
      double dR = deltaR(track.impactPointTSCP().momentum().eta(),
                         track.impactPointTSCP().momentum().phi(),
                         selectedTrack->eta(),
                         selectedTrack->phi());
      if (dR < staMinDr) {
        staMinDr = dR;
        closestapproach = track.stateAtBeamLine().transverseImpactParameter().value();
      }
      //Pick the largest energy standalone muon within dR of 1.0
      if (dR < 1.0 && std::sqrt(track.impactPointTSCP().momentum().mag2()) > staE) {
        if (track.ndof() < 1) {
          continue;
        }
        staE = std::sqrt(track.impactPointTSCP().momentum().mag2());
        info.stadEta = track.impactPointTSCP().momentum().eta() - selectedTrack->eta();
        info.stadPhi = track.impactPointTSCP().momentum().phi() - selectedTrack->phi();
        stadEoverE = (std::sqrt(track.impactPointTSCP().momentum().mag2()) - selectedTrack->p()) / selectedTrack->p();
        info.staChi2 = track.chi2() / track.ndof();
        int nstaHits = 0;
        trackingRecHit_iterator rhbegin = staTrack->recHitsBegin();
        trackingRecHit_iterator rhend = staTrack->recHitsEnd();
        for (trackingRecHit_iterator recHit = rhbegin; recHit != rhend; ++recHit) {
          nstaHits++;
        }
        info.staNHits = nstaHits;
        TrajectoryStateClosestToPoint traj = track.trajectoryStateClosestToPoint(myTracks.probeVertPoint);
        info.staTransDCA = traj.perigeeParameters().transverseImpactParameter();
        info.staLongDCA = traj.perigeeParameters().longitudinalImpactParameter();
      }
    }
  }
  info.closestApproach=closestapproach;
  if (staMinDr < 1.0) {
    info.standaloneE = staE;
    info.standaloneDEoverE = stadEoverE;
  }
  info.staMinDr = staMinDr;
  //Check fail HCAL location here (or probably a bit earlier actually)
  pairedEvents.IncCutFlow(info.eventWeight);
  //Calo Jet Stuff
  double nearProbeCaloJetHadE = -1.;
  double nearProbeCaloJetEmE = -1.;
  double minProbeDr = -1;
  for (auto CaloJet = caloJets->begin(); CaloJet != caloJets->end(); ++CaloJet) {
    double probeDr = deltaR(selectedTrack->eta(), selectedTrack->phi(), CaloJet->eta(), CaloJet->phi());
    double hadE = CaloJet->hadEnergyInHE() + CaloJet->hadEnergyInHB();
    double ecalE = CaloJet->emEnergyInEB() + CaloJet->emEnergyInEE();
    if (probeDr < minProbeDr || minProbeDr < 0) {
      minProbeDr = probeDr;
    }
    if (probeDr < 0.4) {
      if ((hadE + ecalE) > (nearProbeCaloJetHadE + nearProbeCaloJetEmE)) {
        nearProbeCaloJetHadE = hadE;
        nearProbeCaloJetEmE = ecalE;
      }
    }
  }
  info.minCaloJetDr = minProbeDr;
  info.caloJetHcalE = nearProbeCaloJetHadE;
  info.caloJetEcalE = nearProbeCaloJetEmE;
  info.caloJetTotalE = nearProbeCaloJetHadE + nearProbeCaloJetEmE;
  pairedEvents.FillHists(info);
  if(info.minGenMuDr<0.05){genMatched.FillHists(info);}

}

int MuAnalyzer::PairTagAndProbe(
    const edm::Event& iEvent,
    const edm::EventSetup& iSetup,
    Tracks myTracks,
    const reco::Muon* Tag,
    ECAL myECAL,
    edm::ESHandle<TransientTrackBuilder> transientTrackBuilder,
    edm::EDGetTokenT<std::vector<reco::Track>> trackCollection_label,
    edm::Handle<reco::VertexCollection> vtxHandle,
    edm::EDGetTokenT<EcalRecHitCollection> reducedEndcapRecHitCollection_Label,
    edm::EDGetTokenT<EcalRecHitCollection> reducedBarrelRecHitCollection_Label) {
  int cutpro = 0;
  for (std::vector<const reco::Track*>::const_iterator iTrack = myTracks.selectedTracks.begin();
       iTrack != myTracks.selectedTracks.end();
       ++iTrack) {
    
    if (Tag->charge() == (*iTrack)->charge()) continue;
    if (cutpro < 1) {
      cutpro = 1;
    }
    int pairingResult = myTracks.PairTracks(iTrack, Tag->globalTrack(), transientTrackBuilder);
    if (pairingResult > (cutpro - 1)) {
      cutpro = pairingResult + 1;
    }
    if (pairingResult == 3) {
      double probeTrackIso = myTracks.GetIsolation(iEvent, trackCollection_label, 0.3, vtxHandle, *iTrack);
      double probeEcalIso = myECAL.GetIsolation(iEvent,
                                                iSetup,
                                                reducedEndcapRecHitCollection_Label,
                                                reducedBarrelRecHitCollection_Label,
                                                transientTrackBuilder->build(*iTrack));
      double minProbeDr = 5;
      for (auto CaloJet = caloJets->begin(); CaloJet != caloJets->end(); ++CaloJet) {
        double probeDr = deltaR((*iTrack)->eta(), (*iTrack)->phi(), CaloJet->eta(), CaloJet->phi());
        double hadE = CaloJet->hadEnergyInHE() + CaloJet->hadEnergyInHB();
        double ecalE = CaloJet->emEnergyInEB() + CaloJet->emEnergyInEE();
        if (probeDr < minProbeDr) {
          if ((hadE + ecalE) > 15) {
            minProbeDr = probeDr;
          }
        }
      }

      if ((probeTrackIso / (*iTrack)->pt()) > 0.05)
        continue;
      if (cutpro < 5) {
        cutpro = 5;
      }
      if (probeEcalIso > 5. || probeEcalIso < 0.)
        continue;
      if (cutpro < 6) {
        cutpro = 6;
      }
      pairedTagAndProbe_=true;
      selectedTrack = (*iTrack);
      selectedMuon = Tag;
      diMuonMass = myTracks.MuonTrackMass;
      pairVtxChi = myTracks.pairvertexchi;
      reco::TransientTrack tk = transientTrackBuilder->build(*iTrack);
      TrajectoryStateClosestToPoint traj = tk.trajectoryStateClosestToPoint(myTracks.probeVertPoint);
      probeTIP = traj.perigeeParameters().transverseImpactParameter();
      probeLIP = traj.perigeeParameters().longitudinalImpactParameter();
    }
  }
  return cutpro;
}

double MuAnalyzer::MatchTrackToGenMuon(const edm::Event& iEvent,
                                      const reco::Track* selTrack,
                                      edm::EDGetToken m_genParticleToken)
{
   double minDR=-1;
   edm::Handle<reco::GenParticleCollection> genParticles;
   iEvent.getByToken(m_genParticleToken, genParticles);
   for (const auto& particle : *(genParticles.product())) {
      if (std::abs(particle.pdgId()) != 13) continue;
      double dR = deltaR(particle.eta(), particle.phi(), selTrack->eta(), selTrack->phi());
      if(dR<minDR||minDR<0){minDR=dR;}
   }
   return minDR;
}

bool MuAnalyzer::MatchTrackToMuon(const edm::Event& iEvent,
                                   math::XYZTLorentzVectorD mother,
                                   Muons myMuons,
                                   EventInfo info) {
  bool matched = false;
  double mindE = 1.;
  double globalmuonE;
  double mindR = 0.1;
  if (myMuons.selectedMuons.size() == 0) {
    return matched;
  }
  for (std::vector<const reco::Muon*>::const_iterator iMuon = myMuons.selectedMuons.begin();
       iMuon != myMuons.selectedMuons.end();
       ++iMuon) {
    double dR = deltaR((*iMuon)->eta(), (*iMuon)->phi(), mother.eta(), mother.phi());
    double dEOverE = std::abs(std::sqrt(pow((*iMuon)->p(), 2) + pow(0.1056, 2)) - mother.E()) / mother.E();
    if (dR > 0.2)
      continue;
    if (dEOverE < mindE) {
      mindR = dR;
      mindE = dEOverE;
      matched = true;
      if ((*iMuon)->isAValidMuonTrack(reco::Muon::OuterTrack)) {
        standaloneE = (*iMuon)->outerTrack()->p();
      } else {
        standaloneE = -1.;
      }
      globalmuonE = (*iMuon)->p();
    }
  }
  if (matched) {
    info.probeMuondE = mindE;
    info.probeMuondR = mindR;
    info.standaloneE = standaloneE;
    info.globalMuonE = globalmuonE;
  } else {
    standaloneE = 0;
  }
  return matched;
}

std::vector<double> MuAnalyzer::GetPuWeight(edm::Handle<std::vector<PileupSummaryInfo>> hPileupInfoProduct) {
  const std::vector<PileupSummaryInfo>* inPUInfos = hPileupInfoProduct.product();
  for (std::vector<PileupSummaryInfo>::const_iterator itPUInfo = inPUInfos->begin(); itPUInfo != inPUInfos->end();
       ++itPUInfo) {
    if (itPUInfo->getBunchCrossing() == 0) {
      nPUmean_ = itPUInfo->getTrueNumInteractions();
    }
  }
  float lNPVW = float(puWeightHist_->GetBinContent(puWeightHist_->FindBin(nPUmean_)));
  if (nPUmean_ > 70) {
    lNPVW = float(puWeightHist_->GetBinContent(puWeightHist_->FindBin(70)));
  }
  if (nPUmean_ < 1) {
    lNPVW = float(puWeightHist_->GetBinContent(puWeightHist_->FindBin(0)));
  }
  float puUpWeight = float(puWeightUpHist_->GetBinContent(puWeightUpHist_->FindBin(nPUmean_)));
  if (nPUmean_ > 70) {
    puUpWeight = float(puWeightUpHist_->GetBinContent(puWeightUpHist_->FindBin(70)));
  }
  if (nPUmean_ < 1) {
    puUpWeight = float(puWeightUpHist_->GetBinContent(puWeightUpHist_->FindBin(0)));
  }
  float puDownWeight = float(puWeightDownHist_->GetBinContent(puWeightDownHist_->FindBin(nPUmean_)));
  if (nPUmean_ > 70) {
    puDownWeight = float(puWeightDownHist_->GetBinContent(puWeightDownHist_->FindBin(70)));
  }
  if (nPUmean_ < 1) {
    puDownWeight = float(puWeightDownHist_->GetBinContent(puWeightDownHist_->FindBin(0)));
  }

  std::vector<double> puWeights;
  puWeights.push_back(lNPVW);
  puWeights.push_back(puUpWeight);
  puWeights.push_back(puDownWeight);
  return puWeights;
}

// ------------ method called once each job just before starting event loop  ------------
void MuAnalyzer::beginJob() {}

// ------------ method called once each job just after ending the event loop  ------------
void MuAnalyzer::endJob() {}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void MuAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MuAnalyzer);
