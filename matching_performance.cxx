#include <podio/ROOTReader.h>
#include <podio/ROOTReader.h>
#include <podio/ROOTFrameData.h>
#include <podio/Frame.h>
#include <podio/CollectionBase.h>

#include <edm4eic/Track.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegment.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackClusterMatch.h>
#include <edm4eic/TrackClusterMatchCollection.h>

#include <edm4hep/utils/vector_utils.h>

#include <TROOT.h>
#include <TH1.h>
#include <TH1D.h>
#include <TFile.h>
#include <TH2.h>
#include <TH2D.h>
#include <TH3.h>
#include <TH3D.h>
#include <TMath.h>

podio::ROOTReader *setup_reader(std::string filename) {
    podio::ROOTReader *reader = new podio::ROOTReader();
    reader->openFile(filename);
    std::cout << "Opened file with " << reader->getEntries(podio::Category::Event) << " events" << std::endl;
    auto categories = reader->getAvailableCategories();
    for (auto category : categories) {
        std::cout << "Found category " << category << std::endl;
    }
    auto frame = podio::Frame(reader->readEntry(podio::Category::Event, 0));
    auto collections = frame.getAvailableCollections();
    for (auto collection : collections) {
        std::cout << "Found collection " << collection << std::endl;
    }
    // Reset the counter...
    // reader->
    return reader;
    
}

const float MAX_DR = 1;
std::vector<std::string> cluster_collections = {
    "EcalEndcapPTrackClusterMatches",
    "LFHCALTrackClusterMatches",
    "HcalEndcapPInsertClusterMatches",
    "EcalBarrelTrackClusterMatches",
    "HcalBarrelTrackClusterMatches",
    "EcalEndcapNTrackClusterMatches",
    "HcalEndcapNTrackClusterMatches",
};

void matching_performance(std::string particle) {
    podio::ROOTReader *reader = setup_reader(Form("output/%s.root", particle.c_str()));

    TFile *output_file = new TFile(Form("output/matching_performance_%s.root", particle.c_str()), "RECREATE");

    TH1 *track_pt_distribution = new TH1D("track_pt_distribution", "Track pT distribution;p_{T};Counts", 100, 0, 10);
    TH2 *track_eta_phi_distribution = new TH2D("track_eta_phi_distribution", "Track eta-phi distribution;#eta;#phi;Counts", 50, -4, 4, 50, -1 * TMath::Pi(), TMath::Pi());
    TH1 *cluster_E_distribution = new TH1D("cluster_E_distribution", "Cluster E distribution;E;Counts", 100, 0, 10);
    TH2 *cluster_eta_phi_distribution = new TH2D("cluster_eta_phi_distribution", "Cluster eta-phi distribution;#eta;#phi;Counts", 50, -4, 4, 50, -1 * TMath::Pi(), TMath::Pi());

    TH2 *track_cluster_dR = new TH2D("track_cluster_dR", "Track-cluster dR distribution;p_{T};#Delta R;Counts", 100, 0, 10, 100, 0, MAX_DR);
    TH2 *track_cluster_dR_eta = new TH2D("track_cluster_dR_vs_eta", "Track-cluster dR distribution;#eta;#Delta R;Counts", 100, -3, 3, 100, 0, MAX_DR/2);
    TH2 *track_cluster_dR_phi = new TH2D("track_cluster_dR_vs_phi", "Track-cluster dR distribution;#phi;#Delta R;Counts", 100, -TMath::Pi(), TMath::Pi(), 100, 0, MAX_DR/2);
    TH2 *track_cluster_dEta = new TH2D("track_cluster_dEta", "Track-cluster dEta distribution;p_{T};#Delta #eta;Counts", 100, 0, 10, 100, 0, MAX_DR);
    TH2 *track_cluster_dEta_phi = new TH2D("track_cluster_dEta_phi", "Track-cluster dEta distribution;#phi;#Delta #eta;Counts", 100, -TMath::Pi(), TMath::Pi(), 100, 0, MAX_DR/2);
    TH2 *track_cluster_dPhi = new TH2D("track_cluster_dPhi", "Track-cluster dPhi distribution;#phi;#Delta #phi;Counts", 100, 0, 10, 100, 0, MAX_DR);
    TH2 *track_cluster_dPhi_eta = new TH2D("track_cluster_dPhi_eta", "Track-cluster dPhi distribution;#eta;#Delta #phi;Counts", 100, -3, 3, 100, 0, MAX_DR/2);
    TH2 *track_cluster_eta = new TH2D("track_cluster_eta", "Track-cluster eta comparison;Track #eta;Cluster #eta;Counts", 50, -4, 4, 50, -4, 4);
    TH2 *track_cluster_phi = new TH2D("track_cluster_phi", "Track-cluster phi comparison;Track #phi;Cluster #phi;Counts", 50, -1 * TMath::Pi(), TMath::Pi(), 50, -1 * TMath::Pi(), TMath::Pi());
    TH2 *track_cluster_E = new TH2D("track_cluster_E", "Track-cluster E comparison;Track p_{T};Cluster E;Counts", 100, 0, 10, 100, 0, 10);

    TH1 *num_matches = new TH1D("num_matches", "Number of matches;Number of matches;Counts", 10, 0, 10);
    TH2 *num_matches_trk_pt = new TH2D("num_matches_trk_pt", "Number of matches;Track pT;Number of matches;Counts", 100, 0, 10, 10, 0, 10);
    TH2 *num_matches_clstr_E = new TH2D("num_matches_clstr_E", "Number of matches;Cluster E;Number of matches;Counts", 100, 0, 10, 10, 0, 10);

    // std::unique_ptr<podio::ROOTFrameData> data = nullptr;
    for (int i = 0; i < reader->getEntries(podio::Category::Event); i++) {
        auto frame = podio::Frame(reader->readEntry(podio::Category::Event, i));  // I really need to understand unique and shared pointers one of these days...
        // std::cout << "Processing event" << std::endl;

        // Check individual collections
        auto &tracks = frame.get<edm4eic::TrackSegmentCollection>("CalorimeterTrackProjections");
        for (auto track : tracks) {
            if (track.points_size() == 0) {
                std::cout << "Track has no points" << std::endl;
                continue;
            }
            track_pt_distribution->Fill(edm4hep::utils::magnitudeTransverse(track.getPoints()[0].momentum));   // sigh
            auto track_eta = edm4hep::utils::eta(track.getPoints()[0].position);
            auto track_phi = edm4hep::utils::angleAzimuthal(track.getPoints()[0].position);
            track_eta_phi_distribution->Fill(track_eta, track_phi);
        }

        for (auto collection : cluster_collections) {
            auto &clusters = frame.get<edm4eic::ClusterCollection>(collection);
            for (auto cluster : clusters) {
                cluster_E_distribution->Fill(cluster.getEnergy());
                auto cluster_eta = edm4hep::utils::eta(cluster.getPosition());
                auto cluster_phi = edm4hep::utils::angleAzimuthal(cluster.getPosition());
                cluster_eta_phi_distribution->Fill(cluster_eta, cluster_phi);
            }
        }

        // Check the matching
        for (auto collection : cluster_collections) {
            auto &matches = frame.get<edm4eic::TrackClusterMatchCollection>(collection);
            num_matches->Fill(matches.size());
            // std::cout << "Found " << matches.size() << " matches" << std::endl;
            bool first = true;
            for (auto match : matches) {
                // std::cout << "Processing match" << std::endl;
                auto matched_track = match.getTrack();
                auto mached_track_id = matched_track.getObjectID();
                // Now we need to loop and find the TrackSegment with the same object ID
                std::optional<edm4eic::TrackSegment> matched_track_segment = std::nullopt;
                for (auto track_segment : tracks) {
                    if (track_segment.getTrack().getObjectID() == mached_track_id) {
                        matched_track_segment = track_segment;
                        break;
                    }
                }
                if (!matched_track_segment.has_value()) {
                    std::cout << "Could not find a matching track segment for the track" << std::endl;
                    continue;
                }
                if (matched_track_segment->points_size() == 0) {
                    std::cout << "Matched track segment has no points" << std::endl;
                    continue;
                }
                auto matched_cluster = match.getCluster();

                auto track_eta = edm4hep::utils::eta(matched_track_segment->getPoints()[0].position);
                auto track_phi = edm4hep::utils::angleAzimuthal(matched_track_segment->getPoints()[0].position);
                auto cluster_eta = edm4hep::utils::eta(matched_cluster.getPosition());
                auto cluster_phi = edm4hep::utils::angleAzimuthal(matched_cluster.getPosition());

                auto dEta = std::abs(track_eta - cluster_eta);
                auto dPhi = std::abs(track_phi - cluster_phi);
                auto dR = std::hypot(dEta, dPhi);
                // std::cout << "t_eta = " << track_eta << ", c_eta = " << cluster_eta << ", dEta = " << dEta << std::endl;
                // std::cout << "t_phi = " << track_phi << ", c_phi = " << cluster_phi << ", dPhi = " << dPhi << std::endl;
                // std::cout << "dR = " << dR << std::endl;
                track_cluster_dR->Fill(edm4hep::utils::magnitudeTransverse(matched_track_segment->getPoints()[0].momentum), dR);
                track_cluster_dR_eta->Fill(track_eta, dR);
                track_cluster_dR_phi->Fill(track_phi, dR);
                track_cluster_dEta->Fill(edm4hep::utils::magnitudeTransverse(matched_track_segment->getPoints()[0].momentum), dEta);
                track_cluster_dEta_phi->Fill(track_phi, dEta);
                track_cluster_dPhi->Fill(edm4hep::utils::magnitudeTransverse(matched_track_segment->getPoints()[0].momentum), dPhi);
                track_cluster_dPhi_eta->Fill(track_eta, dPhi);
                track_cluster_eta->Fill(track_eta, cluster_eta);
                track_cluster_phi->Fill(track_phi, cluster_phi);
                track_cluster_E->Fill(edm4hep::utils::magnitudeTransverse(matched_track_segment->getPoints()[0].momentum), matched_cluster.getEnergy());

                if (first) {
                    num_matches_trk_pt->Fill(edm4hep::utils::magnitudeTransverse(matched_track_segment->getPoints()[0].momentum), matches.size());
                    num_matches_clstr_E->Fill(matched_cluster.getEnergy(), matches.size());
                    first = false;
                }
            }
        }
    }

    // if (project.getTrack() == association.getTrack()

    // Tracks are base, track segments are only used for projections

    // Write to a file

    // track_pt_distribution->Write();
    // cluster_E_distribution->Write();
    // track_eta_phi_distribution->Write();
    // cluster_eta_phi_distribution->Write();
    // track_cluster_dR->Write();
    // track_cluster_dEta->Write();
    // track_cluster_dPhi->Write();
    // track_cluster_eta->Write();
    // track_cluster_phi->Write();
    // num_matches_trk_pt->Write();
    // num_matches_clstr_E->Write();

    output_file->Write();
    output_file->Close();
    delete output_file;
    delete reader;
}
