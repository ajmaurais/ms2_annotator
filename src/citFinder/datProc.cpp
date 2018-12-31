//
//  datProc.cpp
//  citFinder
//
//  Created by Aaron Maurais on 12/10/18.
//  Copyright © 2018 Aaron Maurais. All rights reserved.
//

#include <citFinder/datProc.hpp>

/**
 Clear PeptideFragmentsMap.
 */
void CitFinder::PeptideFragmentsMap::clear()
{
	fragmentMap.clear();
	_sequence.clear();
}

/**
 Generate fragment map from sequence.
 From a full sequence given, the sequence of all b and y ions are calculated
 and stored in the object.
 @param sequence the peptide sequence to calculate fragments from
 */
void CitFinder::PeptideFragmentsMap::populateMap(std::string sequence)
{
	_sequence = sequence;
	size_t len = _sequence.length();
	for(int i = 0; i < len; i++)
	{
		std::string beg = _sequence.substr(0, i + 1);
		std::string end = _sequence.substr(i + 1);
		
		fragmentMap["b" + std::to_string(i+1)] = beg;
		fragmentMap["y" + std::to_string(len-i-1)] = end;
	}
}

std::string CitFinder::PeptideFragmentsMap::getIonSeq(std::string searchStr) const{
	return fragmentMap.at(searchStr);
}

std::string CitFinder::PeptideFragmentsMap::getIonSeq(char b_y, int num) const{
	std::string searchStr = std::string(1, b_y) + std::to_string(num);
	return getIonSeq(searchStr);
}

void CitFinder::RichFragmentIon::calcSequence(const PeptideFragmentsMap& pepMap){
	sequence = pepMap.getIonSeq(b_y, num);
}

/**
 Align ref sequence to query sequence. To peptide sequences as strings are aligned.
 @param ref The reference sequence to search.
 @param query The sequence to search for.
 @param beg Beginning of match. If no match, left unchanged.
 @param end End of match. If no match, left unchanged.
 @return bool representing if match was found.
 */
bool CitFinder::allignSeq(const std::string& ref, const std::string& query, size_t& beg, size_t& end)
{
	//find query in match
	size_t match = ref.find(query);
	if(match == std::string::npos) return false;
	
	beg = match;
	end = match + query.length() - 1;
	
	return true;
}

void CitFinder::PeptideStats::initStats()
{
	for(IonType i = IonType::First; i != IonType::Last; ++i){
		ionTypesCount[i] = IonTypeDatType("", 0);
	}
	
	containsCit = "false";
}

void CitFinder::PeptideStats::initModLocs(const char* diffmods)
{
	size_t modLoc = std::string::npos;
	bool modFound = false;
	size_t len = fullSequence.length();
	std::string tempSeq = fullSequence;
	for(size_t i = 0; i < len; i++)
	{
		//check that current char is not a mod char
		for(const char* p = diffmods; *p; p++)
			if(tempSeq[i] == *p)
				throw std::runtime_error("Invalid peptide sequence!");
		
		//if not at second to last AA, search for diff mod
		if((i + 1) < tempSeq.length())
		{
			//iterate through diffmods
			for(const char* p = diffmods; *p; p++)
			{
				//check if next char in sequence is diff mod char
				if(tempSeq[i + 1] == *p)
				{
					modFound = true;
					modLoc = i;
					tempSeq.erase(i + 1, 1);
					break;
				}//end of if
			}//end of for
		}//end of if
		//PeptideIon temp(aminoAcidMasses->getMW(sequence[i]));
		if(modFound){
			//temp.setMod(mod, aminoAcidMasses->getMW(mod));
			modLocs.push_back(modLoc);
			//nMod++;
		}
		//aminoAcids.push_back(temp);
		modFound = false;
	}//end of for
}

void CitFinder::PeptideStats::addChar(std::string toAdd, std::string& s)
{
	if(s.empty())
		s = toAdd;
	else s += _fragDelim + toAdd;
}

/**
 Add ionStr and increment ion count for ion
 @param ionStr ion string to add
 @param ion PeptideStats::IonTypeDatType to increment
 @param inc amt to add to ion count
 */
void CitFinder::PeptideStats::incrementIonCount(std::string ionStr,
												PeptideStats::IonTypeDatType& ion,
												int inc){
	addChar(ionStr, ion.first);
	ion.second += inc;
}

/**
 Tests whether sequence contains ambiguous residues.
 @param ambResidues ambiguous residues to search for
 @param fragSeq sequence of fragment to search for ambiguous residues.
 @return True if an ambiguous residue is found.
*/
bool CitFinder::PeptideStats::containsAmbResidues(const std::string& ambResidues,
												  std::string fragSeq) const
{
	size_t len = fragSeq.length();
	for(int i = 0; i < len; i++)
		for(int j = 0; j < ambResidues.length(); j++)
			if(fragSeq[i] == ambResidues[j])
				return true;
	return false;
}

/**
 Add fragment sequence to PeptideStats.
 @param seq fragment ion to add
 @param ambResidues ambigious residues to search for.
 */
void CitFinder::PeptideStats::addSeq(const CitFinder::RichFragmentIon& seq,
									 const std::string& ambResidues)
{
	//first check if seq is found in *this sequence
	size_t beg, end;
	if(!CitFinder::allignSeq(sequence, seq.getSequence(), beg, end))
		return;
	
	//increment total fragment ions found
	std::string ionStr = seq.getIonStr(true);
	incrementIonCount(ionStr, ionTypesCount[IonType::FRAG]);
	
	//check if seq spans any modified residues
	for(auto it = modLocs.begin(); it != modLocs.end(); ++it)
	{
		//check if in span
		if(utils::inSpan(beg, end, *it))
		{
			//check if NL
			if(seq.isNL()){
				incrementIonCount(ionStr, ionTypesCount[IonType::DET_NL_FRAG]);
			}
			else
			{
				if(containsAmbResidues(ambResidues, seq.getSequence())){ //is ambModFrag
					incrementIonCount(ionStr, ionTypesCount[IonType::AMB_MOD_FRAG]);
				}
				else{ //is detFrag
					incrementIonCount(ionStr, ionTypesCount[IonType::DET_FRAG]);
				}
			}
		}
		else{
			if(seq.isNL()){ //is artifact NL frag
				incrementIonCount(ionStr, ionTypesCount[IonType::ART_NL_FRAG]);
			}
			else{ //is amg frag
				incrementIonCount(ionStr, ionTypesCount[IonType::AMB_FRAG]);
			}//end of else
		}//end of else
	}//end of for
}//end of fxn

/*void CitFinder::analyzeSequencesParallel(std::vector<Dtafilter::Scan>& scans,
										 const std::vector<PeptideNamespace::Peptide>& peptides,
										 std::vector<PeptideStats>& peptideStats,
										 const CitFinder::Params& pars)
{
	unsigned int nThreads = pars.getNumThreads();
	
	//init threads array
	std::thread* threads = new std::thread[nThreads];
	
	//split up input data for each thread
	
	
	
	delete [] threads;
}*/

/**
 
 @param peptides vector of peptides to analyze
 */
bool CitFinder::analyzeSequences(std::vector<Dtafilter::Scan>& scans,
								 const std::vector<PeptideNamespace::Peptide>& peptides,
								 std::vector<PeptideStats>& peptideStats,
								 const CitFinder::Params& pars)
{
	CitFinder::PeptideFragmentsMap fragmentMap;
	bool allSucess = true;
	
	for(auto it = peptides.begin(); it != peptides.end(); ++it)
	{
		/*if(it->getSequence() == "RHLETVDGAKVVVLVNR")
			std::cout << "Found!" << NEW_LINE;*/
		
		fragmentMap.clear();
		fragmentMap.populateMap(it->getSequence());
		//it->printFragments(std::cout);
		
		CitFinder::PeptideStats pepStat(*it); //init pepStat
		pepStat._scan = &scans[it - peptides.begin()]; //add pointer to scan
		size_t nFragments = it->getNumFragments();
		for(size_t i = 0; i < nFragments; i++)
		{
			/*if(it->getFragment(i).getIonStr(false) == "b14-43")
				std::cout << "Found!" << NEW_LINE;*/
			
			if(it->getFragment(i).getFound())
			{
				CitFinder::RichFragmentIon fragTemp(it->getFragment(i));
				try{
					fragTemp.calcSequence(fragmentMap);
				}
				catch(std::out_of_range& e){
					std::cout << "\nWarning Error finding fragment: " << it->getFragment(i).getIonStr()
					<< "for sequence: " << it->getFullSequence();
					allSucess = false;
					continue;
				}
				pepStat.addSeq(fragTemp, pars.getAmbigiousResidues());
			} //end of if
		}//end of for i
		pepStat.calcContainsCit();		
		peptideStats.push_back(pepStat);
	}//end if for it
	
	return allSucess;
}//end of fxn

bool CitFinder::findFragmentsParallel(const std::vector<Dtafilter::Scan>& scans,
									  std::vector<PeptideNamespace::Peptide>& peptides,
									  const CitFinder::Params& pars)
{
	unsigned int const nThreads = pars.getNumThreads();
	size_t const nScans = scans.size();
	size_t peptidePerThread = nScans / nThreads;
	if(nScans % nThreads != 0)
		peptidePerThread += 1;
	std::atomic<size_t> scansIndex(0);
	
	//init threads
	std::vector<std::thread> threads;
	bool* sucsses = new bool[nThreads];
	
	//split up input data for each thread
	std::vector<PeptideNamespace::Peptide>* splitPeptides = new std::vector<PeptideNamespace::Peptide>[nThreads];
	size_t begNum, endNum, numInThread;
	unsigned int threadIndex = 0;
	for(size_t i = 0; i < nScans; i += peptidePerThread)
	{
		begNum = i;
		endNum = (begNum + peptidePerThread > nScans ? nScans : begNum + peptidePerThread);
		numInThread = endNum - begNum;

		//spawn thread
		assert(threadIndex < nThreads);
		splitPeptides[threadIndex] = std::vector<PeptideNamespace::Peptide>();
		threads.push_back(std::thread(CitFinder::findFragments_threadSafe, std::ref(scans), begNum, endNum,
									  std::ref(splitPeptides[threadIndex]), std::ref(pars),
									  sucsses + threadIndex, std::ref(scansIndex)));
		threadIndex++;
	}
	
	//spawn progress function
	threads.push_back(std::thread(CitFinder::findFragmentsProgress, std::ref(scansIndex), nScans,
								  nThreads, PROGRESS_SLEEP_TIME));
	
	//join threads
	for(auto it = threads.begin(); it != threads.end(); ++it){
		it->join();
	 }
	
	//concat split peptieds into one vector
	peptides.clear();
	for(unsigned int i = 0; i < nThreads; i++){
		if(!sucsses[i])
			return false;
		peptides.insert(peptides.end(), splitPeptides[i].begin(), splitPeptides[i].end());
	}
	
	delete [] splitPeptides;
	delete [] sucsses;
	return true;
}

/**
 Prints progress bar durring findFragmentsParallel
 @param scansIndex current scan index
 @param count total number of scans to seach for
 @param sleepTime time before next update is printed in seconds
 */
void CitFinder::findFragmentsProgress(std::atomic<size_t>& scansIndex, size_t count,
									  unsigned int nThread, int sleepTime)
{
	size_t lastIndex = scansIndex.load();
	size_t curIndex = lastIndex;
	int noChangeIterations = 0;
	
	std::cout << "\nSearching ms2s for neutral loss ions using " << nThread << " thread(s)...\n";
	do{
		curIndex = scansIndex.load();
		
		if(lastIndex == curIndex)
			noChangeIterations++;
		else noChangeIterations = 0;
		
		if(noChangeIterations > CitFinder::MAX_PROGRESS_ITTERATIONS)
			return;
		
		utils::printProgress(float(curIndex) / float(count));
		std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
		
		lastIndex = curIndex;
	} while(scansIndex < count);
	utils::printProgress(float(scansIndex.load()) / float(count));
	std::cout << NEW_LINE;
	std::cout << "Done!" << NEW_LINE;
}

/**
 Find peptide fragment ions in ms2 files.
 @param scans Populated vector of scan objects to search for
 @param peptides empty vector of peptides to be filled from data in scans.
 @param pars CitFinder params object.
 @return true if sucessful
 */
bool CitFinder::findFragments(const std::vector<Dtafilter::Scan>& scans,
							  std::vector<PeptideNamespace::Peptide>& peptides,
							  CitFinder::Params& pars)
{
	bool* sucess = new bool(false);
	std::atomic<size_t> scansIndex;
	CitFinder::findFragments_threadSafe(scans, 0, scans.size(), peptides, pars,
										sucess, scansIndex);
	return *sucess;
}

/**
 Find peptide fragment ions in ms2 files.
 Function is thread safe.
 @param peptides empty vector of peptides to be filled from data in scans.
 @param beg index of begining of scan vector
 @param end index of end of scan vector
 @param pars CitFinder params object.
 @param sucess set to true if function was sucessful
 */
void CitFinder::findFragments_threadSafe(const std::vector<Dtafilter::Scan>& scans,
							  const size_t beg, const size_t end,
							  std::vector<PeptideNamespace::Peptide>& peptides,
							  const CitFinder::Params& pars,
							  bool* sucess, std::atomic<size_t>& scansIndex)
{
	*sucess = false;
	//std::vector<PeptideNamespace::Peptide> peptidesTemp;
	//peptidesTemp.reserve(scans.size());
	std::map<std::string, ms2::Ms2File> ms2Map;
	std::string curSample;
	std::string curWD;
	aaDB::AADB aminoAcidMasses;
	
	for(size_t i = beg; i < end; i++)
	{
		//read ms2 files if it hasn't been done yet
		if(curSample != scans[i].getSampleName())
		{
			//read ms2 files
			ms2Map.clear();
			//first get current wd name
			curSample = scans[i].getSampleName();
			curWD = utils::dirName(scans[i].getParentFile());
			
			//next get names of all ms2 files in dir
			std::vector<std::string> ms2FileNames;
			if(!utils::ls(curWD.c_str(), ms2FileNames, ".ms2")){
				throw std::runtime_error("Error reading ms2 files!");
				return;
			}
			
			//finally read each ms2 file into map
			for(auto it2 = ms2FileNames.begin(); it2 != ms2FileNames.end(); ++it2)
			{
				ms2Map[*it2] = ms2::Ms2File();
				if(!ms2Map[*it2].read(curWD + "/" + *it2)){
					throw std::runtime_error("Error reading ms2 files!");
					return;
				}
			}
			
			//re-init Peptide::AminoAcidMasses for each sample
			curWD = pars.getWD() + "/" + scans[i].getSampleName();
			std::string spFname = curWD + "/sequest.params";
			
			//read sequest params file and init aadb
			PeptideNamespace::initAminoAcidsMasses(pars, spFname, aminoAcidMasses);
		}//end if
		
		//initialize peptide object for current scan
		peptides.push_back(PeptideNamespace::Peptide(scans[i].getSequence()));
		peptides.back().initialize(pars, aminoAcidMasses);
		
		//calculate neutral loss combinations
		int nMods = peptides.back().getNumMod();
		double nlMass = pars.getNeutralLossMass();
		std::vector<double> neutralLossIons;
		for(int i = 1; i <= nMods; i++)
			neutralLossIons.push_back(i * nlMass);
		
		//add neutral loss fragments to current peptide
		peptides.back().addNeutralLoss(neutralLossIons);
		
		//load spectrum
		ms2::Spectrum spectrum;
		if(!ms2Map[utils::baseName(scans[i].getParentFile())].getScan(scans[i].getScanNum(), spectrum)){
			throw std::runtime_error("Error reading scan!");
			return;
		}
		
		spectrum.labelSpectrum(peptides.back(), pars);
		
		//print spectra file
		if(pars.getPrintSpectraFiles())
		{
			std::string dirNameTemp = (pars.getInDirSpecified() ? pars.getWD() : curWD) + "/spectraFiles";
			if(!utils::dirExists(dirNameTemp))
				if(!utils::mkdir(dirNameTemp.c_str())){
					throw std::runtime_error("Failed to make dir: " + dirNameTemp);
					return;
				}
			spectrum.normalizeIonInts(100);
			spectrum.calcLabelPos();
			
			std::string temp = dirNameTemp + "/" + utils::baseName(scans[i].getOfname());
			std::ofstream outF((temp).c_str());
			if(!outF){
				throw std::runtime_error("Failed to write spectrum!");
				return;
			}
			spectrum.printLabeledSpectrum(outF, true);
		}
		
		scansIndex++;
	} //end of for
	
	*sucess = true;
	return;
}

void CitFinder::PeptideStats::calcContainsCit()
{
	containsCit = "false";
	
	//is the peptide modified?
	if(modLocs.size() == 0) return;
	
	//is c terminal most cit modification on the c terminus?
	//modLocs.back() works because modLocs are added in the order
	//they appear in the sequence
	if(modLocs.back() == sequence.length() - 1) return;
	
	//is there more than 1 determining NLs?
	if(ionTypesCount[IonType::DET_NL_FRAG].second > 1){
		containsCit = "true";
		return;
	}
	
	//are there 1 or more determining NLs or determining frags?
	if(ionTypesCount[IonType::DET_NL_FRAG].second >= 1 ||
	   ionTypesCount[IonType::DET_FRAG].second >= 1){
		containsCit = "likely";
		return;
	}
	
	//are there 1 more ambigious fragments?
	if(ionTypesCount[IonType::AMB_FRAG].second >= 1){
		containsCit = "ambiguous";
		return;
	}
}

std::string CitFinder::PeptideStats::ionTypeToStr(const IonType& it)
{
	switch(it){
		case IonType::FRAG: return ION_TYPES_STR[0];
			break;
		case IonType::DET_FRAG: return ION_TYPES_STR[1];
			break;
		case IonType::AMB_MOD_FRAG: return ION_TYPES_STR[2];
			break;
		case IonType::DET_NL_FRAG: return ION_TYPES_STR[3];
			break;
		case IonType::AMB_FRAG: return ION_TYPES_STR[4];
			break;
		case IonType::ART_NL_FRAG: return ION_TYPES_STR[5];
			break;
		case IonType::Last: return "Last";
			break;
	}
}

/**
 Prints peptide stats to out.
 @param stats Peptide stats to print.
 @param ofname name of file to write to.
 */
bool CitFinder::printPeptideStats(const std::vector<PeptideStats>& stats, std::string ofname)
{
	//assert(out);
	std::ofstream outF (ofname);
	if(!outF) return false;
	
	typedef CitFinder::PeptideStats::IonType itcType;
	//build stat names vector
	std::vector<std::string> statNames;
	statNames.push_back("containsCit");
	for(int i = 0; i < N_ION_TYPES; i++)
		statNames.push_back("n" + std::string(1, (char)std::toupper(ION_TYPES_STR[i][0])) +
							ION_TYPES_STR[i].substr(1));
	statNames.insert(statNames.end(), ION_TYPES_STR, ION_TYPES_STR + N_ION_TYPES);
	
	std::string otherHeaders = "protein_ID parent_protein protein_description full_sequence sequence charge unique xCorr scan parent_file sample_name";
	std::vector<std::string> oHeaders;
	utils::split(otherHeaders, ' ', oHeaders);
	std::vector<std::string> headers;
	headers.reserve(statNames.size() + oHeaders.size());
	headers.insert(headers.end(), oHeaders.begin(), oHeaders.end());
	headers.insert(headers.end(), statNames.begin(), statNames.end());
	
	//print headers
	size_t len = headers.size();
	for(size_t i = 0; i < len; i++){
		if(i == 0)
			outF << headers[i];
		else outF << OUT_DELIM << headers[i];
	}
	outF << NEW_LINE;
	
	//print data
	for(auto it = stats.begin(); it != stats.end(); ++it)
	{
		//scan data
		outF << it->_scan->getParentID() <<
		OUT_DELIM << it->_scan->getParentProtein() <<
		OUT_DELIM << it->_scan->getParentDescription() <<
		OUT_DELIM << it->_scan->getFullSequence() <<
		OUT_DELIM << it->_scan->getSequence() <<
		OUT_DELIM << it->_scan->getCharge() <<
		OUT_DELIM << it->_scan->getUnique() <<
		OUT_DELIM << it->_scan->getXcorr() <<
		OUT_DELIM << it->_scan->getScanNum() <<
		OUT_DELIM << utils::baseName(it->_scan->getParentFile()) <<
		OUT_DELIM << it->_scan->getSampleName();
		
		//peptid analysis data
		outF << OUT_DELIM << it->containsCit;
		
		for(itcType i = itcType::First; i != itcType::Last; ++i)
			outF << OUT_DELIM << it->ionTypesCount.at(i).second;
		
		for(itcType i = itcType::First; i != itcType::Last; ++i)
			outF << OUT_DELIM << it->ionTypesCount.at(i).first;
		
		outF << NEW_LINE;
	}
	return true;
}
