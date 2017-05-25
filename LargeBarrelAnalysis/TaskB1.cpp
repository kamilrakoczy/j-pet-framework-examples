/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file TaskB1.cpp
 */

#include <map>
#include <string>
#include <JPetWriter/JPetWriter.h>
#include "TaskB1.h"
using namespace std;
TaskB1::TaskB1(const char * name, const char * description):
JPetTask(name, description){}
TaskB1::~TaskB1(){}

void TaskB1::init(const JPetTaskInterface::Options&){

  fOutputEvents = new JPetTimeWindow("JPetRawSignal");

  fBarrelMap.buildMappings(getParamBank());
	// create histograms for TOT - one for each DAQ channel
	for(auto & tomb : getParamBank().getTOMBChannels()){
		
		const char * histo_name = formatUniqueChannelDescription(*(tomb.second), "TOT_");
		getStatistics().createHistogram( new TH1F(histo_name, histo_name, 4000, 20., 100.) );
	}
	// a 2D histogram for presence of leading vs trailing edge
	getStatistics().createHistogram( new TH2F("was lead and trail edge?",
						  "was lead and trail edge?;was trail edge;was lead edge",
					   2, -0.5, 1.5, 2, -0.5, 1.5)
	);
	// create histograms for TDC hits multiplicity vs PMT number
	// separately for each threshold
	for(int thr=1;thr<=kNumOfThresholds;thr++){
		int n_pmts = getParamBank().getPMsSize();
		char * histo_name = Form("HitsLeadingEdge_thr%d", thr);
		char * histo_title = Form("%s;PMT No.;No. hits", histo_name);
		getStatistics().createHistogram( new TH1F(histo_name, histo_title, n_pmts, -0.5, n_pmts-0.5) );
		
		histo_name = Form("HitsTrailingEdge_thr%d", thr);
		histo_title = Form("%s;PMT No.;No. hits", histo_name);
		getStatistics().createHistogram( new TH1F(histo_name, histo_title, n_pmts, -0.5, n_pmts-0.5) );
	}
}

void TaskB1::exec(){
	//getting the data from event in propriate format
  if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(getEvent())){
		map<int,JPetSigCh> leadSigChs;
		map<int,JPetSigCh> trailSigChs;
		map<int, JPetRawSignal> signals; 
		const unsigned int nSigChs = timeWindow->getNumberOfEvents();
		for (unsigned int i = 0; i < nSigChs; i++) {
		  const JPetSigCh & sigch = dynamic_cast<const JPetSigCh&>(timeWindow->operator[](i));
			int daq_channel = sigch.getChannel();
			if( sigch.getType() == JPetSigCh::Leading )
				leadSigChs[ daq_channel ] = sigch;
			if( sigch.getType() == JPetSigCh::Trailing )
				trailSigChs[ daq_channel ] = sigch;
		}
		// iterate over the leading-edge SigChs
		for (auto & chSigPair : leadSigChs) {
			int daq_channel = chSigPair.first;
			if( trailSigChs.count(daq_channel) != 0 ){ 
				getStatistics().getHisto2D("was lead and trail edge?").Fill(1.,1.);
				JPetSigCh & leadSigCh = chSigPair.second;
				JPetSigCh & trailSigCh = trailSigChs.at(daq_channel);
				double tot = trailSigCh.getValue() - leadSigCh.getValue();
				if( leadSigCh.getPM() != trailSigCh.getPM() ){
					ERROR("Signals from same channel point to different PMTs! Check the setup mapping!!!");
				}
				const char * histo_name = formatUniqueChannelDescription(leadSigCh.getTOMBChannel(), "TOT_");
				getStatistics().getHisto1D(histo_name).Fill( tot / 1000. );
				int pmt_number = calcGlobalPMTNumber(leadSigCh.getPM());
				getStatistics().getHisto1D(Form("HitsLeadingEdge_thr%d", leadSigCh.getThresholdNumber())).Fill(pmt_number);
				double pmt_id = trailSigCh.getPM().getID();
				signals[pmt_id].addPoint( leadSigCh );
				signals[pmt_id].addPoint( trailSigCh );
			}else{
				getStatistics().getHisto2D("was lead and trail edge?").Fill(0.,1.);
			}
		}
		// the above loop will not count cases where there was only trailing edge signal
		// count this in a separate loop
		for (const auto & chSigPair : trailSigChs) {
			int daq_channel = chSigPair.first;
			if( leadSigChs.count(daq_channel) == 0 )
				getStatistics().getHisto2D("was lead and trail edge?").Fill(1.,0.);
			int pmt_number = calcGlobalPMTNumber(chSigPair.second.getPM());
			getStatistics().getHisto1D(Form("HitsTrailingEdge_thr%d", chSigPair.second.getThresholdNumber())).Fill(pmt_number);
		}    
		for(auto & pmSignalPair : signals){
			auto & signal = pmSignalPair.second;
			const auto & pmt = getParamBank().getPM(pmSignalPair.first);
			signal.setPM(pmt);
			signal.setBarrelSlot(pmt.getBarrelSlot());
			fOutputEvents->add<JPetRawSignal>(signal);
		}
	}
}
void TaskB1::terminate(){}

const char * TaskB1::formatUniqueChannelDescription(const JPetTOMBChannel & channel, const char * prefix = "") const {
	
	int slot_number = fBarrelMap.getSlotNumber(channel.getPM().getBarrelSlot());
	int layer_number = fBarrelMap.getLayerNumber(channel.getPM().getBarrelSlot().getLayer()); 
	char side = (channel.getPM().getSide()==JPetPM::SideA ? 'A' : 'B');
	int threshold_number = channel.getLocalChannelNumber();
	
	return Form("%slayer_%d_slot_%d_side_%c_thr_%d",
		    prefix,
	     layer_number,
	     slot_number,
	     side,
	     threshold_number
	);
}

int TaskB1::calcGlobalPMTNumber(const JPetPM & pmt) const {
	
	const int number_of_sides = 2;
	
	int layer_number = fBarrelMap.getLayerNumber(pmt.getBarrelSlot().getLayer());
	int pmt_no = 0;
	for(int l=1;l<layer_number;l++){
		pmt_no += number_of_sides * fBarrelMap.getNumberOfSlots(l);
	}
	int slot_number = fBarrelMap.getSlotNumber(pmt.getBarrelSlot());
	if( pmt.getSide() == JPetPM::SideB ){
		pmt_no += fBarrelMap.getNumberOfSlots(layer_number);
	}
	pmt_no += slot_number - 1;
	
	return pmt_no;
}

void TaskB1::setParamManager(JPetParamManager* paramManager) {
	fParamManager = paramManager;
}
const JPetParamBank& TaskB1::getParamBank() const {
	return fParamManager->getParamBank();
}
