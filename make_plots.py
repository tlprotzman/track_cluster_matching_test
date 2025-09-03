import ROOT
from datetime import datetime

# Configure root
ROOT.gStyle.SetOptStat(0)
canvas_width = 800
canvas_height = 600

text = ROOT.TLatex()
text.SetNDC()
text.SetTextFont(42)
text.SetTextSize(0.04)
text.SetTextAlign(11)  # align left

# Globals to hold canvases etc
canvases = []
stacks = []
legends = []

particles = ['e-']

files = {}
for particle in particles:
    files[particle] = ROOT.TFile(f'output/matching_performance_{particle}.root', 'read')

match_collections = ['EcalBarrelTrackClusterMatches',
                     'EcalEndcapNTrackClusterMatches',
                     'EcalEndcapPTrackClusterMatches',
                     'HcalBarrelTrackClusterMatches',
                     'HcalEndcapNTrackClusterMatches',
                     'LFHCALTrackClusterMatches']

cluster_collections = ['EcalBarrelClusters',
                       'EcalEndcapNClusters',
                       'EcalEndcapPClusters',
                       'HcalBarrelClusters',
                       'HcalEndcapNClusters',
                       'LFHCALClusters']

# Create a canvas for the title page
working_canvas = ROOT.TCanvas('working_canvas', 'Title Page', canvas_width, canvas_height)
canvases.append(working_canvas)

# Draw the title
working_canvas.cd()
text.DrawLatex(0.3, 0.6, 'Track Cluster Matching Performance')
# Draw the date and time
current_datetime = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
text.DrawLatex(0.3, 0.5, f'Generated on: {current_datetime}')
text.DrawLatex(0.3, 0.4, 'July 2025 Simulation Campaign, Single particle guns')

# Save the title page as an image or PDF if needed
working_canvas.Update()
working_canvas.SaveAs('matching_performance.pdf(')
working_canvas.Clear()

for particle in particles:
    file = files[particle]
    text.DrawLatex(0.1, 0.9, f'Particle: {particle}')
    working_canvas.Update()
    working_canvas.SaveAs('matching_performance.pdf')
    working_canvas.Clear()
    
    # Plot the input track spectra
    pt_spectra = file.Get(f'track_pt_distribution')
    p_spectra = file.Get(f'track_p_distribution')

    working_canvas.Divide(2, 1)
    working_canvas.cd(1)
    pt_spectra.SetTitle(f'Track pT distribution for {particle};pT [GeV];Entries')
    pt_spectra.SetLineColor(ROOT.kBlue)
    pt_spectra.Draw('HIST')
    ROOT.gPad.SetLogy()
    working_canvas.cd(2)
    p_spectra.SetTitle(f'Track p distribution for {particle};p [GeV];Entries')
    p_spectra.SetLineColor(ROOT.kBlue)
    p_spectra.Draw('HIST')
    ROOT.gPad.SetLogy()
    working_canvas.Update()
    working_canvas.SaveAs('matching_performance.pdf')
    working_canvas.Clear()

    track_location = file.Get('track_eta_phi_distribution')
    track_location.SetTitle(f'Track eta-phi location for {particle};#eta;#phi [rad];Entries')
    track_location.Draw('COLZ')
    ROOT.gPad.SetLogz()
    working_canvas.Update()
    working_canvas.SaveAs('matching_performance.pdf')
    working_canvas.Clear()

    # Plot the calorimeter spectra
    working_canvas.Divide(3, 2)
    for i, cluster_collection in enumerate(cluster_collections):
        working_canvas.cd(i + 1)
        cluster_spectra = file.Get(f'cluster_E_distribution_{cluster_collection}')
        cluster_spectra.SetTitle(f'{cluster_collection} energy distribution for {particle};Energy [GeV];Entries')
        cluster_spectra.SetLineColor(ROOT.kBlue)
        cluster_spectra.Draw('HIST')
        ROOT.gPad.SetLogy()
    working_canvas.Update()
    working_canvas.SaveAs('matching_performance.pdf')
    working_canvas.Clear()

    working_canvas.Divide(3, 2)
    for i, cluster_collection in enumerate(cluster_collections):
        working_canvas.cd(i + 1)
        cluster_location = file.Get(f'cluster_eta_phi_distribution_{cluster_collection}')
        cluster_location.SetTitle(f'{cluster_collection} eta-phi location for {particle};#eta;#phi [rad];Entries')
        cluster_location.Draw('COLZ')
        ROOT.gPad.SetLogz()
    working_canvas.Update()
    working_canvas.SaveAs('matching_performance.pdf')
    working_canvas.Clear()

    # Plot the matching performance
    for i, match_collection in enumerate(match_collections):
        # text.DrawLatex(0.1, 0.9, f'Backwards matching performance: {particle}')   # Add later
        # working_canvas.Update()
        # working_canvas.SaveAs('matching_performance.pdf')
        # working_canvas.Clear()

        dR = file.Get(f'track_cluster_dR_{match_collection}')
        dEta = file.Get(f'track_cluster_dEta_{match_collection}')
        dPhi = file.Get(f'track_cluster_dPhi_{match_collection}')
        dxdy = file.Get(f'track_cluster_dxdy_{match_collection}')
        dr = file.Get(f'track_cluster_dr_{match_collection}')
        dz = file.Get(f'track_cluster_dz_{match_collection}')
        dRdeta = file.Get(f'track_cluster_dR_vs_eta_{match_collection}')
        dRdphi = file.Get(f'track_cluster_dR_vs_phi_{match_collection}')
        
        working_canvas.Divide(4, 2)
        working_canvas.cd(1)
        dR.SetTitle(f'{match_collection} #DeltaR for {particle};p_{{T}};#DeltaR;Entries')
        dR.SetLineColor(ROOT.kBlue)
        dR.Draw('colz')
        ROOT.gPad.SetLogz()

        working_canvas.cd(2)
        dEta.SetTitle(f'{match_collection} #Delta#eta for {particle};p_{{T}};#Delta#eta;Entries')
        dEta.SetLineColor(ROOT.kBlue)
        dEta.Draw('colz')
        ROOT.gPad.SetLogz()

        working_canvas.cd(3)
        dPhi.SetTitle(f'{match_collection} #Delta#phi for {particle};p_{{T}};#Delta#phi;Entries')
        dPhi.SetLineColor(ROOT.kBlue)
        dPhi.Draw('colz')
        ROOT.gPad.SetLogz()

        working_canvas.cd(4)
        dRdeta.SetTitle(f'{match_collection} #DeltaR vs #eta for {particle};#eta;#DeltaR;Entries')
        dRdeta.SetLineColor(ROOT.kBlue)
        dRdeta.Draw('colz')
        ROOT.gPad.SetLogz()

        working_canvas.cd(5)
        dxdy.SetTitle(f'{match_collection} dxdy for {particle};dx;dy;Entries')
        dxdy.SetLineColor(ROOT.kBlue)
        dxdy.Draw('colz')
        ROOT.gPad.SetLogz()

        working_canvas.cd(6)
        dr.SetTitle(f'{match_collection} dr for {particle};dr;Entries')
        dr.SetLineColor(ROOT.kBlue)
        dr.Draw('hist')
        ROOT.gPad.SetLogy()

        working_canvas.cd(7)
        dz.SetTitle(f'{match_collection} dz for {particle};dz;Entries')
        dz.SetLineColor(ROOT.kBlue)
        dz.Draw('hist')
        ROOT.gPad.SetLogy()

        working_canvas.cd(8)
        dRdphi.SetTitle(f'{match_collection} #DeltaR vs #phi for {particle};#phi;#DeltaR;Entries')
        dRdphi.SetLineColor(ROOT.kBlue)
        dRdphi.Draw('colz')
        ROOT.gPad.SetLogz()

        working_canvas.Update()
        working_canvas.SaveAs('matching_performance.pdf')
        working_canvas.Clear()

    
    


working_canvas.SaveAs('matching_performance.pdf)')

