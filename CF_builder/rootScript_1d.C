Double_t gaussian(Double_t *x, Double_t *par) {
    Double_t arg = -1 * (x[0] * par[1] / 0.197) * (x[0] * par[1] / 0.197); // 1 - R
    Double_t fitval = 1 + par[0] * TMath::Exp(arg); // 0 - lambda
    return fitval;
}
//  graph, multigraph, legend, m_style, m_size, m_color, l_color, title
void make_style(TGraphErrors* graph, TMultiGraph* multigraph, TLegend* legend, int m_style, double m_size, int m_color, int l_color, std::string title){
    graph->SetMarkerStyle(m_style);
    graph->SetMarkerSize(m_size);
    graph->SetMarkerColor(m_color);
    graph->SetLineColor(l_color);
    graph->SetTitle(title.c_str());
    graph->GetXaxis()->SetTitleSize(0.05);
    graph->GetYaxis()->SetTitleSize(0.05);
    legend->AddEntry(graph, title.c_str(), "lp");
    multigraph->Add(graph, "lp");
}

void rootScript_1d() {
    std::unique_ptr<TFile> inputFile(TFile::Open("/home/ubuntu/Temir/smash/build/data/script/OUT/stphys/output.root", "READ"));

    const std::array<std::string, 4> centralityNames = {"0-10", "10-30", "30-50", "50-80"};
    const std::array<std::string, 4> ktNames = {"0.15-0.25", "0.25-0.35", "0.35-0.45", "0.45-0.60"};
    const std::array<double, 5> ktvalues = {0.15, 0.25, 0.35, 0.45, 0.6};
    const std::array<int, 4> colors = {kRed, kBlue, kMagenta, kGreen};

    TCanvas* c_1d_CF_at_min = new TCanvas("c_1d_CF_at_min", "c_1d_CF_at_min", 900, 700); c_1d_CF_at_min->Divide(2, 2, 0.001, 0.001); // 2.1
    TCanvas* c_1d_CF_4_center = new TCanvas("k_t_const", "k_t = [0.15; 0.25] and 4 centrality classes", 900, 700); // 2.2.1
    TCanvas* c_1d_CF_4_kt = new TCanvas("0-10 centrality", "b = [0-10] and 4 different kt", 900, 700); // // 2.2.2
    TCanvas* c_all_1d_CF = new TCanvas("c_all_1d_CF", "c_all_1d_CF", 900, 700);  c_all_1d_CF->Divide(4, 4, 0, 0);
    TCanvas* c_Rinv_vs_kt = new TCanvas(); // 2.3.1
    TCanvas* c_lambda_vs_kt = new TCanvas(); // 2.3.2

    TLegend* l_Rinv = new TLegend(0.5, 0.7, 0.9, 0.9);
    TLegend* l_lambda = new TLegend(0.5, 0.7, 0.9, 0.9);
    TLegend* l_CF_4_center = new TLegend(0.5, 0.7, 0.9, 0.9);
    TLegend* l_CF_4_kt = new TLegend(0.5, 0.7, 0.9, 0.9);

    TMultiGraph* mg_Rinv = new TMultiGraph();
    TMultiGraph* mg_lambda = new TMultiGraph();

    TF1* fit = new TF1("fit", gaussian, 0., 0.4, 2);
    fit->SetParameter(0, 0.5);  fit->SetParLimits(0, 0., 1.);
    fit->SetParameter(1, 4.5);  fit->SetParLimits(1, 0., 10.);

    for (uint i = 0; i < centralityNames.size(); i++) {
        TGraphErrors* g_tmp_Rinv = new TGraphErrors();
        TGraphErrors* g_tmp_lambda = new TGraphErrors();

        for (uint j = 0; j < ktNames.size(); j++) {
            gStyle->SetOptStat(0);
            std::string suffix = centralityNames[i] + "_" + ktNames[j] + "_GeVc";

            std::string name = "CF(q_{inv})_" + suffix;
            TH1D* hist_C_wei = new TH1D(name.c_str(), name.c_str(), 100, 0., 1.);
            hist_C_wei->Divide((TH1D*)inputFile->Get(("h_A_wei_q_inv_" + suffix).c_str()), (TH1D*)inputFile->Get(("h_A_q_inv_" + suffix).c_str()));

            std::cout << "__________" << "current_hist_#" << 4*i + j + 1 << "__________" << std::endl;

            
            
            c_all_1d_CF->cd(4 * i + j + 1);
            hist_C_wei->SetTitle(("CF(q_{inv}) " + centralityNames[i] + " % "+ ktNames[j] + "GeV/c" ";q_{inv}; CF").c_str());
            hist_C_wei->GetYaxis()->SetTitleSize(0.05);
            hist_C_wei->GetXaxis()->SetTitleSize(0.05);
            hist_C_wei->GetYaxis()->SetRangeUser(0.8, 2);
            hist_C_wei->GetXaxis()->SetRangeUser(0., 0.3); 
            fit->SetLineColor(colors[(i+j)%4]);
            hist_C_wei->SetLineColor(colors[(i+j)%4]);
            hist_C_wei->SetMarkerColor(colors[(i+j)%4]);
            hist_C_wei->SetMarkerStyle(20 + j);
            hist_C_wei->SetMarkerSize(0.8);
            hist_C_wei->GetXaxis()->SetTitleOffset(0.9);
            hist_C_wei->Draw("P");

            if(j == 0){
                c_1d_CF_4_kt->cd();
                hist_C_wei->Draw("P SAME");
                l_CF_4_kt->AddEntry(hist_C_wei, ("CF at centr " + centralityNames[i] + "%").c_str(), "lp");
            }
            if(i == 0){
                c_1d_CF_4_center->cd();
                hist_C_wei->Draw("P SAME");
                l_CF_4_center->AddEntry(hist_C_wei, ("CF at k_{t}" + ktNames[j] + "GeV/c").c_str(), "lp");
            }
            
            if (i == 0 && j == 0)   {
                TH1D* h_A_min_kt_center = new TH1D(*(TH1D*)inputFile->Get(("h_A_q_inv_" + suffix).c_str()));
                h_A_min_kt_center->SetTitle(("A(q_{inv})_" + suffix).c_str());
                h_A_min_kt_center->GetYaxis()->SetTitleSize(0.05);   h_A_min_kt_center->GetXaxis()->SetTitleSize(0.05);
                c_1d_CF_at_min->cd(1);  h_A_min_kt_center->Draw("P");

                TH1D* h_A_wei_min_kt_center = new TH1D(*(TH1D*)inputFile->Get(("h_A_wei_q_inv_" + suffix).c_str()));
                h_A_wei_min_kt_center->SetTitle(("A_{wei}(q_{inv})_" + suffix).c_str());
                h_A_wei_min_kt_center->GetYaxis()->SetTitleSize(0.05);   h_A_wei_min_kt_center->GetXaxis()->SetTitleSize(0.05);
                c_1d_CF_at_min->cd(2);  h_A_wei_min_kt_center->Draw("P");
                TH1D* h_C_wei_min_kt_center = new TH1D(*hist_C_wei);
                h_C_wei_min_kt_center->SetName("h_C_wei");   c_1d_CF_at_min->cd(3); h_C_wei_min_kt_center->Draw("P");
            }

            const double xval = (ktvalues[j+1] + ktvalues[j])/2;
            const double xerr = ktvalues[j+1] - xval;
            
            hist_C_wei->Fit(fit, "RM");
            g_tmp_Rinv->SetPoint(j, xval, fit->GetParameter(1));
            g_tmp_Rinv->SetPointError(j, xerr, fit->GetParError(1));
            g_tmp_lambda->SetPoint(j, xval, fit->GetParameter(0));
            g_tmp_lambda->SetPointError(j, xerr, fit->GetParError(0));
            
            std::cout << "____________hist_done_#" << 4 * i + j + 1 << "___________" << std::endl;
        }

        make_style(g_tmp_Rinv, mg_Rinv, l_Rinv, 21, 1, colors[i], colors[i], ("R_{inv} " + centralityNames[i] + " %"));
        make_style(g_tmp_lambda, mg_lambda, l_lambda, 20, 1, colors[i], colors[i], ("#lambda " + centralityNames[i] + " %"));
    }

    mg_Rinv->SetTitle("R_{inv}(k_{t});k_{t} [GeV/c]; R_{inv} [fm]");
    mg_Rinv->GetXaxis()->SetTitleSize(0.05);
    mg_Rinv->GetYaxis()->SetTitleSize(0.05);
    c_Rinv_vs_kt->cd();
    mg_Rinv->Draw("ALP");
    l_Rinv->Draw("SAME");

    mg_lambda->SetTitle("#lambda(k_{t});k_{t} [GeV/c]; #lambda");
    mg_lambda->GetXaxis()->SetTitleSize(0.05);
    mg_lambda->GetYaxis()->SetTitleSize(0.05);
    c_lambda_vs_kt->cd();
    mg_lambda->Draw("ALP");
    l_lambda->Draw("SAME");

    /*TCanvas* c_1d_CF_4_center = new TCanvas("k_t_const", "k_t = [0.15; 0.25] and 4 centrality classes", 900, 700); // 2.2.1
    TCanvas* c_1d_CF_4_kt = new TCanvas("0-10 centrality", "b = [0-10] and 4 different kt", 900, 700); // // 2.2.2*/

    TPaveText *t = new TPaveText(0.2, 0.92, 0.8, 1.0, "brNDC"); // left-up
    t->AddText("CF at k_t = [0.15; 0.25] GeV/c and 4 centrality classes");

    c_1d_CF_4_kt->cd();
    l_CF_4_kt->Draw("SAME");
    t->Draw();

    TPaveText *t2 = new TPaveText(0.2, 0.92, 0.8, 1.0, "brNDC"); // left-up
    t2->AddText("CF at centrality = [0-10] and 4 different kt");
    c_1d_CF_4_center->cd();
    l_CF_4_center->Draw("SAME");
    t2->Draw();

    const char* outputDir = "output_histograms_1d";
    gSystem->Exec(Form("mkdir %s", outputDir));

    const char* format = "png";
    c_1d_CF_4_center->SaveAs(Form("%s/%s.%s", outputDir, "c_1d_CF_4_center", format));
    c_1d_CF_4_kt->SaveAs(Form("%s/%s.%s", outputDir, "c_1d_CF_4_kt", format));
    c_all_1d_CF->SaveAs(Form("%s/%s.%s", outputDir, "c_all_1d_CF", format));
    c_Rinv_vs_kt->SaveAs(Form("%s/%s.%s", outputDir, "c_Rinv_vs_kt", format));
    c_lambda_vs_kt->SaveAs(Form("%s/%s.%s", outputDir, "c_lambda_vs_kt", format));
    c_1d_CF_at_min->SaveAs(Form("%s/%s.%s", outputDir, "c_1d_CF_at_min", format));
}