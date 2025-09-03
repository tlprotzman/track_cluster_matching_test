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
std::vector<std::string> match_collections = {
    "EcalBarrelTrackClusterMatches",
    "EcalEndcapNTrackClusterMatches",
    "EcalEndcapPTrackClusterMatches",
    "HcalBarrelTrackClusterMatches",
    "HcalEndcapNTrackClusterMatches",
    "LFHCALTrackClusterMatches",
};

std::vector<std::string> cluster_collections  = {
    "EcalBarrelClusters",
    "EcalEndcapNClusters",
    "EcalEndcapPClusters",
    "HcalBarrelClusters",
    "HcalEndcapNClusters",
    "LFHCALClusters",
};


void matching_performance(std::string particle) {
    podio::ROOTReader *reader = setup_reader(Form("root://dtn-eic.jlab.org//volatile/eic/EPIC%s", particle.c_str()));
    auto filename = std::filesystem::path(particle).filename();

    TFile *output_file = new TFile(Form("matching_performance.%s", filename.c_str()), "RECREATE");


    TH1* track_pt_distributions;
    TH2* track_eta_phi_distributions;
    std::map<std::string, TH1*> cluster_E_distributions;
    std::map<std::string, TH2*> cluster_eta_phi_distributions;

    std::map<std::string, TH2*> track_cluster_dRs;
    std::map<std::string, TH2*> track_cluster_dR_etas;
    std::map<std::string, TH2*> track_cluster_dR_phis;
    std::map<std::string, TH2*> track_cluster_dEtas;
    std::map<std::string, TH2*> track_cluster_dEta_phis;
    std::map<std::string, TH2*> track_cluster_dPhis;
    std::map<std::string, TH2*> track_cluster_dPhi_etas;
    std::map<std::string, TH2*> track_cluster_etas;
    std::map<std::string, TH2*> track_cluster_phis;
    std::map<std::string, TH2*> track_cluster_Es;
    std::map<std::string, TH2*> track_cluster_dxdy;
    std::map<std::string, TH1*> track_cluster_dr;
    std::map<std::string, TH1*> track_cluster_dz;


    std::map<std::string, TH1*> num_matches;
    std::map<std::string, TH2*> num_matches_trk_pts;
    std::map<std::string, TH2*> num_matches_clstr_Es;

    track_pt_distributions = new TH1D("track_pt_distribution", "Track p_{T} Distribution;p_{T};Counts", 100, 0, 10);
    track_eta_phi_distributions = new TH2D("track_eta_phi_distribution", "Track #eta-#phi distributions;#eta;#phi;Counts", 100, -5, 5, 100, -TMath::Pi(), TMath::Pi());

    for (const auto& collection : cluster_collections) {
        cluster_E_distributions[collection] = new TH1D(Form("cluster_E_distribution_%s", collection.c_str()), "cluster E Distribution;E;Counts", 100, 0, 10);
        cluster_eta_phi_distributions[collection] = new TH2D(Form("cluster_eta_phi_distribution_%s", collection.c_str()), "cluster #eta-#phi distributions;#eta;#phi;Counts", 100, -5, 5, 100, -TMath::Pi(), TMath::Pi());
    }

    for (const auto& collection : match_collections) {
        track_cluster_dRs[collection] = new TH2D(Form("track_cluster_dR_%s", collection.c_str()), 
                Form("Track-cluster dR distribution for %s;p_{T};#Delta R;Counts", collection.c_str()), 100, 0, 10, 100, 0, MAX_DR);
        track_cluster_dR_etas[collection] = new TH2D(Form("track_cluster_dR_vs_eta_%s", collection.c_str()), 
                Form("Track-cluster dR distribution for %s;#eta;#Delta R;Counts", collection.c_str()), 100, -3, 3, 100, 0, MAX_DR/2);
        track_cluster_dR_phis[collection] = new TH2D(Form("track_cluster_dR_vs_phi_%s", collection.c_str()), 
                Form("Track-cluster dR distribution for %s;#phi;#Delta R;Counts", collection.c_str()), 100, -TMath::Pi(), TMath::Pi(), 100, 0, MAX_DR/2);
        track_cluster_dEtas[collection] = new TH2D(Form("track_cluster_dEta_%s", collection.c_str()), 
                Form("Track-cluster dEta distribution for %s;p_{T};#Delta #eta;Counts", collection.c_str()), 100, 0, 10, 100, 0, MAX_DR);
        track_cluster_dEta_phis[collection] = new TH2D(Form("track_cluster_dEta_phi_%s", collection.c_str()), 
                Form("Track-cluster dEta distribution for %s;#phi;#Delta #eta;Counts", collection.c_str()), 100, -TMath::Pi(), TMath::Pi(), 100, 0, MAX_DR/2);
        track_cluster_dPhis[collection] = new TH2D(Form("track_cluster_dPhi_%s", collection.c_str()), 
                Form("Track-cluster dPhi distribution for %s;#phi;#Delta #phi;Counts", collection.c_str()), 100, 0, 10, 100, 0, MAX_DR);
        track_cluster_dPhi_etas[collection] = new TH2D(Form("track_cluster_dPhi_eta_%s", collection.c_str()), 
                Form("Track-cluster dPhi distribution for %s;#eta;#Delta #phi;Counts", collection.c_str()), 100, -3, 3, 100, 0, MAX_DR/2);
        track_cluster_etas[collection] = new TH2D(Form("track_cluster_eta_%s", collection.c_str()), 
                Form("Track-cluster eta comparison for %s;Track #eta;Cluster #eta;Counts", collection.c_str()), 50, -4, 4, 50, -4, 4);
        track_cluster_phis[collection] = new TH2D(Form("track_cluster_phi_%s", collection.c_str()), 
                Form("Track-cluster phi comparison for %s;Track #phi;Cluster #phi;Counts", collection.c_str()), 50, -1 * TMath::Pi(), TMath::Pi(), 50, -1 * TMath::Pi(), TMath::Pi());
        track_cluster_Es[collection] = new TH2D(Form("track_cluster_E_%s", collection.c_str()), 
                Form("Track-cluster E comparison for %s;Track p_{T};Cluster E;Counts", collection.c_str()), 100, 0, 10, 100, 0, 10);
        track_cluster_dxdy[collection] = new TH2D(Form("track_cluster_dxdy_%s", collection.c_str()), "",  1000, -1000, 1000, 1000, -1000, 1000);
        track_cluster_dr[collection] = new TH1D(Form("track_cluster_dr_%s", collection.c_str()), "", 100, -100, 100);
        track_cluster_dz[collection] = new TH1D(Form("track_cluster_dz_%s", collection.c_str()), "", 100, -100, 100);

        num_matches[collection] = new TH1D(Form("num_matches_%s", collection.c_str()), Form("Number of matches for %s;Number of matches;Counts", collection.c_str()), 10, 0, 10);
        num_matches_trk_pts[collection] = new TH2D(Form("num_matches_trk_pt_%s", collection.c_str()), 
                Form("Number of matches for %s;Track pT;Number of matches;Counts", collection.c_str()), 100, 0, 10, 10, 0, 10);
        num_matches_clstr_Es[collection] = new TH2D(Form("num_matches_clstr_E_%s", collection.c_str()), 
                Form("Number of matches for %s;Cluster E;Number of matches;Counts", collection.c_str()), 100, 0, 10, 10, 0, 10);
    }
    // std::unique_ptr<podio::ROOTFrameData> data = nullptr;
    auto n_events = reader->getEntries(podio::Category::Event);
    for (int i = 0; i < n_events; i++) {
        auto frame = podio::Frame(reader->readEntry(podio::Category::Event, i));  // I really need to understand unique and shared pointers one of these days...
        if (i % 1000 == 0) {
            std::cout << "Processing event " << i << ".  " << 100 * ((float)i / n_events) << " done" << std::endl;
        }

        // Check individual collections
        auto &tracks = frame.get<edm4eic::TrackSegmentCollection>("CalorimeterTrackProjections");
        for (auto track : tracks) {
            if (track.points_size() == 0) { std::cout << "Track has no points" << std::endl;
                continue;
            }
            auto track_pt = edm4hep::utils::magnitudeTransverse(track.getPoints()[0].momentum);
            auto track_eta = edm4hep::utils::eta(track.getPoints()[0].position);
            auto track_phi = edm4hep::utils::angleAzimuthal(track.getPoints()[0].position);
            track_pt_distributions->Fill(track_pt);
            track_eta_phi_distributions->Fill(track_eta, track_phi);
        }

        for (const auto& collection : cluster_collections) {
            // std::cout << "found " << frame.get<edm4eic::ClusterCollection>(collection).size() << " clusters in collection " << collection << std::endl;
            auto &clusters = frame.get<edm4eic::ClusterCollection>(collection);
            for (auto cluster : clusters) {
                auto cluster_E = cluster.getEnergy(); auto cluster_eta = edm4hep::utils::eta(cluster.getPosition());
                auto cluster_phi = edm4hep::utils::angleAzimuthal(cluster.getPosition());
                cluster_E_distributions[collection]->Fill(cluster_E);
                cluster_eta_phi_distributions[collection]->Fill(cluster_eta, cluster_phi);
            }
        }

        // Check the matching
        for (const auto& collection : match_collections) {
            // std::cout << "looking for collection " << collection << std::endl;
            auto &matches = frame.get<edm4eic::TrackClusterMatchCollection>(collection);
            num_matches[collection]->Fill(matches.size());
            // std::cout << "found " << matches.size() << " matches" << std::endl;

            for (auto match : matches) {
                auto matched_track = match.getTrack();
                auto matched_track_id = matched_track.getObjectID();

                std::optional<edm4eic::TrackSegment> matched_track_segment = std::nullopt;
                for (auto track_segment : tracks) {
                    if (track_segment.getTrack().getObjectID() == matched_track_id) {
                        matched_track_segment = track_segment;
                        break;
                    }
                }

                if (!matched_track_segment.has_value() || matched_track_segment->points_size() == 0) {
                    std::cout << "Could not find a valid matching track segment for the track" << std::endl;
                    continue;
                }

                auto matched_cluster = match.getCluster();
                auto track_pt = edm4hep::utils::magnitudeTransverse(matched_track_segment->getPoints()[0].momentum);
                auto track_eta = edm4hep::utils::eta(matched_track_segment->getPoints()[0].position);
                auto track_phi = edm4hep::utils::angleAzimuthal(matched_track_segment->getPoints()[0].position);
                auto track_x = matched_track_segment->getPoints()[0].position.x;
                auto track_y = matched_track_segment->getPoints()[0].position.y;
                auto track_z = matched_track_segment->getPoints()[0].position.z;
                auto track_r = std::hypot(track_x, track_y);

                auto cluster_eta = edm4hep::utils::eta(matched_cluster.getPosition());
                auto cluster_phi = edm4hep::utils::angleAzimuthal(matched_cluster.getPosition());
                auto cluster_E = matched_cluster.getEnergy();
                auto cluster_x = matched_cluster.getPosition().x;
                auto cluster_y = matched_cluster.getPosition().y;
                auto cluster_z = matched_cluster.getPosition().z;
                auto cluster_r = std::hypot(cluster_x, cluster_y);

                auto dEta = std::abs(track_eta - cluster_eta);
                auto dPhi = std::abs(track_phi - cluster_phi);
                auto dR = std::hypot(dEta, dPhi);
                auto dx = track_x - cluster_x;
                auto dy = track_y - cluster_y;
                auto dz = track_z - cluster_z;
                auto dr = track_r - cluster_r;

                track_cluster_dRs[collection]->Fill(track_pt, dR);
                track_cluster_dR_etas[collection]->Fill(track_eta, dR);
                track_cluster_dR_phis[collection]->Fill(track_phi, dR);
                track_cluster_dEtas[collection]->Fill(track_pt, dEta);
                track_cluster_dEta_phis[collection]->Fill(track_phi, dEta);
                track_cluster_dPhis[collection]->Fill(track_pt, dPhi);
                track_cluster_dPhi_etas[collection]->Fill(track_eta, dPhi);
                track_cluster_etas[collection]->Fill(track_eta, cluster_eta);
                track_cluster_phis[collection]->Fill(track_phi, cluster_phi);
                track_cluster_Es[collection]->Fill(track_pt, cluster_E);

                track_cluster_dxdy[collection]->Fill(dx, dy);
                track_cluster_dr[collection]->Fill(dr);
                track_cluster_dz[collection]->Fill(dz);

                num_matches_trk_pts[collection]->Fill(track_pt, matches.size());
                num_matches_clstr_Es[collection]->Fill(cluster_E, matches.size());
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
