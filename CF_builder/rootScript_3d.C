Double_t CF_fit_3d(Double_t *q, Double_t *par){
    const Double_t hc2 = (0.197*0.197);
    Double_t fitval = 1 + par[3] * TMath::Exp(( -(q[0]*par[0] * q[0]*par[0])
                                                -(q[1]*par[1] * q[1]*par[1])
                                                -(q[2]*par[2] * q[2]*par[2]) )/ hc2);
    return  fitval;// 3 - lambda 0,1,2 - R_o,s,l;
}

void draw_mg_on_c(TCanvas* canvas, TMultiGraph* multigraph, TLegend* legend, std::string title, int padIndex,
                  double minY, double maxY, std::string titleY){
            //-0.3, 4.3, "k_{t} GeV/c", 2
    multigraph->GetXaxis()->SetRangeUser(-0.3, 4.3);    multigraph->GetXaxis()->SetTitle("k_{t} [GeV/c]");
    multigraph->GetYaxis()->SetRangeUser(minY, maxY);   multigraph->GetYaxis()->SetTitle(titleY.c_str());
    multigraph->SetTitle(title.c_str());

    canvas->cd(padIndex);
    multigraph->Draw("alp");
    legend->Draw("SAME");
}
void draw_g_on_c(TCanvas* canvas, TMultiGraph* multigraph, TGraphErrors* graph, TLegend* legend,
                 uint column, uint rows, int color, std::string title){
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.4);
    graph->SetMarkerColor(color);
    graph->SetLineColor(color);
    graph->SetTitle(title.c_str());
    legend->AddEntry(graph, title.c_str(), "lp");

    canvas->cd(column + 4*rows);
    graph->Draw("alp");
    legend->Draw("SAME");
    multigraph->Add(graph, "lp");
}

void ProjectAndDraw2D(std::unique_ptr<TFile>& inputFile, TCanvas* c_axis1_vs_axis2, TCanvas* c_2DCF_min, uint i, uint j, uint pad,
                      std::string axis1_vs_axis2, std::string LCMS1, std::string LCMS2, std::string& suffix, uint padIndex){ // TCanvas* c_2DCF_min, uint i, uint j, uint pad
    TH3D* tmp_h_A_3d = new TH3D(*(TH3D*)inputFile->Get(("h3d_A_q_" + suffix).c_str()));
    TH3D* tmp_h_A_wei_3d = new TH3D(*(TH3D*)inputFile->Get(("h3d_A_wei_q_" + suffix).c_str()));

    if (axis1_vs_axis2 == "xy") {
        tmp_h_A_wei_3d->GetZaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetZaxis()->SetRangeUser(-0.05, 0.05);
    } else if (axis1_vs_axis2 == "xz") {
        tmp_h_A_wei_3d->GetYaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetYaxis()->SetRangeUser(-0.05, 0.05);
    } else if (axis1_vs_axis2 == "yz") {
        tmp_h_A_wei_3d->GetXaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetXaxis()->SetRangeUser(-0.05, 0.05);
    }

    std::string name = "CF_q_{" + LCMS1 + "_vs_" + LCMS2 + "}" + suffix;
    TH2D* h2d_projection_A = (TH2D*)tmp_h_A_3d->Project3D(axis1_vs_axis2.c_str());
    TH2D* h2d_projection_A_wei = (TH2D*)tmp_h_A_wei_3d->Project3D(axis1_vs_axis2.c_str());
    TH2D* h2d_tmp_C = new TH2D(name.c_str(), name.c_str(), 80, -0.4, 0.4,
                                                           80, -0.4, 0.4);
    h2d_tmp_C->Divide(h2d_projection_A_wei, h2d_projection_A);
    h2d_tmp_C->GetYaxis()->SetRangeUser(-0.13, 0.13); h2d_tmp_C->GetYaxis()->SetTitle(("q_{" + LCMS1 + "} [GeV/c]").c_str());
    h2d_tmp_C->GetXaxis()->SetRangeUser(-0.13, 0.13); h2d_tmp_C->GetXaxis()->SetTitle(("q_{" + LCMS2 + "} [GeV/c]").c_str());
    h2d_tmp_C->GetZaxis()->SetRangeUser(0.95, 1.3);
    h2d_tmp_C->GetYaxis()->SetTitleOffset(0.8);

    c_axis1_vs_axis2->cd(padIndex);
    h2d_tmp_C->Draw("colz");
    if(i == 0 && j == 0){
        c_2DCF_min->cd(pad);
        h2d_tmp_C->Draw("colz");
    }
}

void ProjectAndDraw1D(std::unique_ptr<TFile>& inputFile, TCanvas* canvas, TCanvas* canvas1, TF3* fit3d,
                      uint i, uint j, uint pad, std::string& axis, std::string& LCMS, std::string& suffix) {
    TH3D* tmp_h_A_3d = new TH3D(*(TH3D*)inputFile->Get(("h3d_A_q_" + suffix).c_str()));
    TH3D* tmp_h_A_wei_3d = new TH3D(*(TH3D*)inputFile->Get(("h3d_A_wei_q_" + suffix).c_str()));

    TH3D* denominator = new TH3D("denom", "denom", 80, -0.4, 0.4,
                                                   80, -0.4, 0.4,
                                                   80, -0.4, 0.4);
    denominator->SetDirectory(nullptr);
    TH3D* numerator = new TH3D("numer", "numer", 80, -0.4, 0.4,
                                                 80, -0.4, 0.4,
                                                 80, -0.4, 0.4);
    numerator->SetDirectory(nullptr);

    for (int xbin = 1; xbin <= 80; xbin++) {
        for (int ybin = 1; ybin <= 80; ybin++) {
            for (int zbin = 1; zbin <= 80; zbin++) {
                denominator->SetBinContent(xbin, ybin, zbin, 1.);
                denominator->SetBinError(xbin, ybin, zbin, 0);
                double xval = numerator->GetXaxis()->GetBinCenter(xbin);
                double zval = numerator->GetZaxis()->GetBinCenter(zbin);
                double yval = numerator->GetYaxis()->GetBinCenter(ybin);
                double value = fit3d->Eval(xval, yval, zval);
                numerator->SetBinContent(xbin, ybin, zbin, value);
                numerator->SetBinError(xbin, ybin, zbin, 0);
            }
        }
    }

    if (LCMS == "out") {
        tmp_h_A_wei_3d->GetYaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_wei_3d->GetZaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetYaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetZaxis()->SetRangeUser(-0.05, 0.05);
        denominator->GetYaxis()->SetRangeUser(-0.05, 0.05);
        denominator->GetZaxis()->SetRangeUser(-0.05, 0.05);
        numerator->GetYaxis()->SetRangeUser(-0.05, 0.05);
        numerator->GetZaxis()->SetRangeUser(-0.05, 0.05);
    } else if (LCMS == "side") {
        tmp_h_A_wei_3d->GetXaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_wei_3d->GetZaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetXaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetZaxis()->SetRangeUser(-0.05, 0.05);
        denominator->GetXaxis()->SetRangeUser(-0.05, 0.05);
        denominator->GetZaxis()->SetRangeUser(-0.05, 0.05);
        numerator->GetXaxis()->SetRangeUser(-0.05, 0.05);
        numerator->GetZaxis()->SetRangeUser(-0.05, 0.05);
    } else if (LCMS == "long") {
        tmp_h_A_wei_3d->GetXaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_wei_3d->GetYaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetXaxis()->SetRangeUser(-0.05, 0.05);
        tmp_h_A_3d->GetYaxis()->SetRangeUser(-0.05, 0.05);
        denominator->GetXaxis()->SetRangeUser(-0.05, 0.05);
        denominator->GetYaxis()->SetRangeUser(-0.05, 0.05);
        numerator->GetXaxis()->SetRangeUser(-0.05, 0.05);
        numerator->GetYaxis()->SetRangeUser(-0.05, 0.05);
    }

    TH1D* h_1d_projection_A = (TH1D*)tmp_h_A_3d->Project3D(axis.c_str());
    TH1D* h_1d_projection_A_wei = (TH1D*)tmp_h_A_wei_3d->Project3D(axis.c_str());
    TH1D* h_1d_denominator = (TH1D*)denominator->Project3D(axis.c_str());
    TH1D* h_1d_numerator = (TH1D*)numerator->Project3D(axis.c_str());
    TH1D* h_1d_fit = new TH1D(("h_1d_fit_{" + LCMS + "}" + suffix).c_str(), ("h_1d_fit_{" + LCMS + "}" + suffix).c_str(), 80, -0.4, 0.4);
    TH1D* h_1d_tmp_C = new TH1D(("h3d_C_q_{" + LCMS + "}" + suffix).c_str(), ("h3d_C_q_{" + LCMS + "}" + suffix).c_str(), 80, -0.4, 0.4);

    h_1d_tmp_C->Divide(h_1d_projection_A_wei, h_1d_projection_A);
    h_1d_fit->Divide(h_1d_numerator, h_1d_denominator);
    
    h_1d_tmp_C->GetYaxis()->SetRangeUser(0.8, 1.9);  h_1d_tmp_C->GetYaxis()->SetTitle(("CF_{" + LCMS + "}").c_str());
    h_1d_tmp_C->GetXaxis()->SetRangeUser(-0.2, 0.2); h_1d_tmp_C->GetXaxis()->SetTitle(("q_{" + LCMS + "} [GeV/c]").c_str());

    const std::array<int, 4> colors = {kRed, kBlue, kMagenta, kGreen};
    h_1d_fit->SetLineColor(colors[(i+j)%4]);
    h_1d_fit->SetLineWidth(4);
    h_1d_tmp_C->SetLineColor(colors[(i+j)%4]);
    h_1d_tmp_C->SetMarkerColor(colors[(i+j)%4]);
    h_1d_tmp_C->SetMarkerStyle(20 + (i+j)%4);
    h_1d_tmp_C->GetXaxis()->SetTitleSize(0.05);
    h_1d_tmp_C->GetYaxis()->SetTitleSize(0.05);
    h_1d_tmp_C->GetXaxis()->SetTitleOffset(0.8);

    canvas->cd(4*i + j + 1);
    h_1d_tmp_C->Draw("LP");
    h_1d_fit->Draw("L SAME");

    if(i == 0 && j == 0)  {
        canvas1->cd(pad);
        h_1d_tmp_C->Draw("LP");
        h_1d_fit->Draw("L SAME");
    }
}   

void rootScript_3d() {
    std::unique_ptr<TFile> inputFile(TFile::Open("/home/ubuntu/Temir/smash/build/data/script/OUT/stphys/output.root", "READ"));

    const std::array<std::string, 4> centralityNames = {"0-10", "10-30", "30-50", "50-80"};
    const std::array<std::string, 4> ktNames = {"0.15-0.25", "0.25-0.35", "0.35-0.45", "0.45-0.60"};
    const std::array<double, 5> ktvalues = {0.15, 0.25, 0.35, 0.45, 0.6};
    const std::array<int, 4> colors = {kRed, kBlue, kMagenta, kGreen};
    std::array<std::string, 3> axis = {"x", "y", "z"};
    std::array<std::string, 3> LCMS = {"out", "side", "long"};

    TCanvas* c_1DCF_min = new TCanvas("c_1DCF_min", "multipads9", 900, 700);    c_1DCF_min->Divide(2, 2, 0.001, 0.001);
    TCanvas* c_2DCF_min = new TCanvas("c_2DCF_min", "multipads10", 900, 700);    c_2DCF_min->Divide(2, 2, 0.001, 0.001);

    TCanvas* c_all_3d_CF = new TCanvas("c_all_3d_CF", "multipads2", 900, 700);  c_all_3d_CF->Divide(4, 4, 0.001, 0.001); // 2.4.1
    TCanvas* c_ROSL_lambda = new TCanvas("c_ROSL_lambda", "multipads3", 900, 700);  c_ROSL_lambda->Divide(4, 4, 0.001, 0.001);
    TCanvas* c_out_side_long_lambda = new TCanvas("c_out_side_long_lambda", "multipads5", 900, 700);    c_out_side_long_lambda->Divide(2, 2, 0.001, 0.001); // 2.4.2

    TCanvas* c_all_3d_CF_proj_out = new TCanvas("c_all_3d_CF_proj_out", "multipads6", 900, 700);    c_all_3d_CF_proj_out->Divide(4, 4, 0.001, 0.001);
    TCanvas* c_all_3d_CF_proj_side = new TCanvas("c_all_3d_CF_proj_side", "multipads7", 900, 700);  c_all_3d_CF_proj_side->Divide(4, 4, 0.001, 0.001);
    TCanvas* c_all_3d_CF_proj_long = new TCanvas("c_all_3d_CF_proj_long", "multipads8", 900, 700);  c_all_3d_CF_proj_long->Divide(4, 4, 0.001, 0.001);

    TCanvas* c_2DCF_out_vs_side = new TCanvas("c_2DCF_out_vs_side", "multipads10", 900, 700);     c_2DCF_out_vs_side->Divide(4, 4, 0.01, 0.01);
    TCanvas* c_2DCF_out_vs_long = new TCanvas("c_2DCF_out_vs_long", "multipads11", 900, 700);     c_2DCF_out_vs_long->Divide(4, 4, 0.01, 0.01);
    TCanvas* c_2DCF_side_vs_long = new TCanvas("c_2DCF_side_vs_long", "multipads12", 900, 700);   c_2DCF_side_vs_long->Divide(4, 4, 0.01, 0.01);

    TLegend* l_Rout = new TLegend(0.5, 0.7, 0.9, 0.9);
    TLegend* l_Rside = new TLegend(0.5, 0.7, 0.9, 0.9);
    TLegend* l_Rlong = new TLegend(0.5, 0.7, 0.9, 0.9);
    TLegend* l_lambda = new TLegend(0.5, 0.7, 0.9, 0.9);

    TMultiGraph* mg_Rout = new TMultiGraph();
    TMultiGraph* mg_Rside = new TMultiGraph();
    TMultiGraph* mg_Rlong = new TMultiGraph();
    TMultiGraph* mg_lambda_3d = new TMultiGraph();

    TF3* fit3d = new TF3("fit3d", CF_fit_3d, -0.4, 0.4,
                                             -0.4, 0.4,
                                             -0.4, 0.4, 4);
    fit3d->SetParameter(0, 5.5); fit3d->SetParLimits(0, 0., 7.);
    fit3d->SetParameter(1, 4.); fit3d->SetParLimits(1, 0., 7.);
    fit3d->SetParameter(2, 3.5); fit3d->SetParLimits(2, 0., 7.);
    fit3d->SetParameter(3, 0.8); fit3d->SetParLimits(3, 0.5, 0.95);


    for (uint i = 0; i < centralityNames.size(); i++) {
        TGraphErrors* g_tmp_Rout = new TGraphErrors();
        TGraphErrors* g_tmp_Rside = new TGraphErrors();
        TGraphErrors* g_tmp_Rlong = new TGraphErrors();
        TGraphErrors* g_tmp_lambda3d = new TGraphErrors();

        for (uint j = 0; j < ktNames.size(); j++) {
            gStyle->SetOptStat(0);
            std::string suffix = centralityNames[i] + "_" + ktNames[j] + "_GeVc";

            std::string name = "h3d_C_q_" + suffix;
            TH3D* hists_A_3d = (TH3D*)inputFile->Get(("h3d_A_q_" + suffix).c_str());
            TH3D* hists_A_wei_3d = (TH3D*)inputFile->Get(("h3d_A_wei_q_" + suffix).c_str());
            TH3D* hists_C_wei_3d = new TH3D(name.c_str(), name.c_str(), 80, -0.4, 0.4,
                                                                        80, -0.4, 0.4,
                                                                        80, -0.4, 0.4);
            hists_C_wei_3d->Divide(hists_A_wei_3d, hists_A_3d);

            std::cout << "__________" << "current_hist_#" << 4*i + j + 1 << "__________" << std::endl;

            const double xval = (ktvalues[j+1] + ktvalues[j])/2;
            const double xerr = ktvalues[j+1] - xval;

            hists_C_wei_3d->Fit(fit3d, "RM");

            g_tmp_Rout->SetPoint(j, xval, fit3d->GetParameter(0));
            g_tmp_Rout->SetPointError(j, xerr, fit3d->GetParError(0));
            g_tmp_Rside->SetPoint(j, xval, fit3d->GetParameter(1));
            g_tmp_Rside->SetPointError(j, xerr, fit3d->GetParError(1));
            g_tmp_Rlong->SetPoint(j, xval, fit3d->GetParameter(2));
            g_tmp_Rlong->SetPointError(j, xerr, fit3d->GetParError(2));
            g_tmp_lambda3d->SetPoint(j, xval, fit3d->GetParameter(3));
            g_tmp_lambda3d->SetPointError(j, xerr, fit3d->GetParError(3));

            c_all_3d_CF->cd(4 * i + j + 1);
            gPad->SetTickx(2);
            hists_C_wei_3d->GetXaxis()->SetRangeUser(-0.4, 0.4);
            hists_C_wei_3d->GetYaxis()->SetRangeUser(-0.4, 0.4);
            hists_C_wei_3d->GetZaxis()->SetRangeUser(-0.4, 0.4);
            hists_C_wei_3d->GetXaxis()->SetTitleOffset(0.8);
            hists_C_wei_3d->Draw();

            // PROJECTION START 
            ProjectAndDraw1D(inputFile, c_all_3d_CF_proj_out , c_1DCF_min, fit3d, i, j, 1, axis[0], LCMS[0], suffix);
            ProjectAndDraw1D(inputFile, c_all_3d_CF_proj_side, c_1DCF_min, fit3d, i, j, 2, axis[1], LCMS[1], suffix);
            ProjectAndDraw1D(inputFile, c_all_3d_CF_proj_long, c_1DCF_min, fit3d, i, j, 3, axis[2], LCMS[2], suffix);
             // TCanvas* c_2DCF_min, uint i, uint j, uint pad
            ProjectAndDraw2D(inputFile, c_2DCF_out_vs_side, c_2DCF_min, i, j, 1, axis[0] + axis[1], LCMS[0], LCMS[1], suffix, 4 * i + j + 1);
            ProjectAndDraw2D(inputFile, c_2DCF_out_vs_long, c_2DCF_min, i, j, 2, axis[0] + axis[2], LCMS[0], LCMS[2], suffix, 4 * i + j + 1);
            ProjectAndDraw2D(inputFile, c_2DCF_side_vs_long, c_2DCF_min, i, j, 3, axis[1] + axis[2], LCMS[1],  LCMS[2], suffix, 4 * i + j + 1);
            // PROJECTION END
            
            std::cout << "____________hist_done_#" << 4 * i + j + 1 << "___________" << std::endl;
        }

        if(i == 2){
            g_tmp_Rout->RemovePoint(3);
            g_tmp_Rside->RemovePoint(3);
            g_tmp_Rlong->RemovePoint(3);
            g_tmp_lambda3d->RemovePoint(3);
        }
        if(i == 3){
            g_tmp_Rout->RemovePoint(3);     g_tmp_Rout->RemovePoint(2);         g_tmp_Rout->RemovePoint(1);
            g_tmp_Rside->RemovePoint(3);    g_tmp_Rside->RemovePoint(2);        g_tmp_Rside->RemovePoint(1);
            g_tmp_Rlong->RemovePoint(3);    g_tmp_Rlong->RemovePoint(2);        g_tmp_Rlong->RemovePoint(1);
            g_tmp_lambda3d->RemovePoint(3); g_tmp_lambda3d->RemovePoint(2);     g_tmp_lambda3d->RemovePoint(1);
        }
        /*void draw_g_on_c(canvas, multigraph, graph, legend, column, rows, color, title){*/ 
        draw_g_on_c(c_ROSL_lambda, mg_Rout, g_tmp_Rout, l_Rout, 1, i, colors[i], ("R_{out} " + centralityNames[i] + " %"));
        draw_g_on_c(c_ROSL_lambda, mg_Rside, g_tmp_Rside, l_Rside, 2, i, colors[i], ("R_{side} " + centralityNames[i] + " %"));
        draw_g_on_c(c_ROSL_lambda, mg_Rlong, g_tmp_Rlong, l_Rlong, 3, i, colors[i], ("R_{long} " + centralityNames[i] + " %"));
        draw_g_on_c(c_ROSL_lambda, mg_lambda_3d, g_tmp_lambda3d, l_lambda, 4, i, colors[i], ("#lambda_{3D} " + centralityNames[i] + " %"));
        c_ROSL_lambda->cd(1);
        l_Rout->Draw("SAME");
        c_ROSL_lambda->cd(2);
        l_Rside->Draw("SAME");
        c_ROSL_lambda->cd(3);
        l_Rlong->Draw("SAME");
        c_ROSL_lambda->cd(4);
        l_lambda->Draw("SAME");
    }
    /*void draw_mg_on_c(TCanvas* canvas, TMultiGraph* multigraph, TLegend* legend, std::string title, int padIndex, double minY, double maxY, std::string titleY){*/
    draw_mg_on_c(c_out_side_long_lambda, mg_Rout, l_Rout, "R_{out}", 1, 2, 7, "R_{" + LCMS[0] + "} [fm]");
    draw_mg_on_c(c_out_side_long_lambda, mg_Rside, l_Rside, "R_{side}", 2, 2, 5, "R_{" + LCMS[1] + "} [fm]");
    draw_mg_on_c(c_out_side_long_lambda, mg_Rlong, l_Rlong, "R_{long}", 3, 2, 5, "R_{" + LCMS[2] + "} [fm]");
    draw_mg_on_c(c_out_side_long_lambda, mg_lambda_3d, l_lambda, "#lambda", 4, 0.6, 1.2, "#lambda");

    const char* outputDir = "output_histograms_3d";
    const char* format = "png";
    gSystem->Exec(Form("mkdir %s", outputDir));

    c_all_3d_CF->SaveAs(Form("%s/%s.%s", outputDir, "c_all_3d_CF", format));
    c_out_side_long_lambda->SaveAs(Form("%s/%s.%s", outputDir, "c_out_side_long_lambda", format));

    c_ROSL_lambda->SaveAs(Form("%s/%s.%s", outputDir, "c_ROSL_lambda", format));
    c_1DCF_min->SaveAs(Form("%s/%s.%s", outputDir, "c_1DCF_min", format));
    c_2DCF_min->SaveAs(Form("%s/%s.%s", outputDir, "c_2DCF_min", format));

    c_all_3d_CF_proj_out->SaveAs(Form("%s/%s.%s", outputDir, "c_all_3d_CF_proj_out", format));
    c_all_3d_CF_proj_side->SaveAs(Form("%s/%s.%s", outputDir, "c_all_3d_CF_proj_side", format));
    c_all_3d_CF_proj_long->SaveAs(Form("%s/%s.%s", outputDir, "c_all_3d_CF_proj_long", format));

    c_2DCF_out_vs_side->SaveAs(Form("%s/%s.%s", outputDir, "c_2DCF_out_vs_side", format));
    c_2DCF_out_vs_long->SaveAs(Form("%s/%s.%s", outputDir, "c_2DCF_out_vs_long", format));
    c_2DCF_side_vs_long->SaveAs(Form("%s/%s.%s", outputDir, "c_2DCF_side_vs_long", format));
}