//
//  ms2Spectrum.cpp
//  ms2_anotator
//
//  Created by Aaron Maurais on 11/20/17.
//  Copyright © 2017 Aaron Maurais. All rights reserved.
//

#include <ms2Spectrum.hpp>

void ms2::Spectrum::writeMetaData(std::ostream& out) const
{
	assert(out);
	out << ms2::BEGIN_METADATA << NEW_LINE
		<< "precursorFile" << OUT_DELIM << precursorFile << NEW_LINE
		<< "ofname" << OUT_DELIM << getOfNameBase(_parentMs2, fullSequence) << NEW_LINE
		<< "scanNumber" << OUT_DELIM << scanNum << NEW_LINE
		<< "sequence" << OUT_DELIM << sequence << NEW_LINE
		<< "fullSequence" << OUT_DELIM << fullSequence << NEW_LINE
		<< "retTime" << OUT_DELIM << retTime << NEW_LINE
		<< "precursorCharge" << OUT_DELIM << charge << NEW_LINE
		<< "precursorInt" << OUT_DELIM << std::scientific << precursorInt << NEW_LINE;
	out.unsetf(std::ios::scientific);
	out << "precursorScan" << OUT_DELIM << precursorScan << NEW_LINE
		<< ms2::END_METADATA <<NEW_LINE << ms2::BEGIN_SPECTRUM << NEW_LINE;
}

void ms2::Spectrum::printSpectrum(std::ostream& out, bool includeMetaData) const
{
	assert(out);
	if(includeMetaData)
		writeMetaData(out);
	
	for(size_t i = 0; i < NUM_SPECTRUM_COL_HEADERS_SHORT; i++)
	{
		if(i == 0)
			out << SPECTRUM_COL_HEADERS[i];
		else out << OUT_DELIM << SPECTRUM_COL_HEADERS[i];
	}
	out << NEW_LINE;
	
	for(ionsTypeConstIt it = ions.begin(); it != ions.end(); ++it)
		out << it->getMZ() << OUT_DELIM << it->getIntensity() <<NEW_LINE;
	
	if(includeMetaData)
		out << ms2::END_SPECTRUM <<NEW_LINE;
}

void ms2::Spectrum::printLabeledSpectrum(std::ostream& out, bool includeMetaData) const
{
	assert(out);
	if(includeMetaData)
		writeMetaData(out);
	
	for(size_t i = 0; i < NUM_SPECTRUM_COL_HEADERS_LONG; i++)
	{
		if(i == 0)
			out << SPECTRUM_COL_HEADERS[i];
		else out << OUT_DELIM << SPECTRUM_COL_HEADERS[i];
	}
	out << NEW_LINE;
	
	std::streamsize ss = std::cout.precision();
	std::cout.precision(5); //set out to print 5 floating point decimal places
	for(ionsTypeConstIt it = ions.begin(); it != ions.end(); ++it)
	{
		out << std::fixed << it->getMZ() << OUT_DELIM
		<< it->getIntensity() << OUT_DELIM
		<< it->getLabel() << OUT_DELIM
		<< it->label.getIncludeLabel() << OUT_DELIM
		<< it->getIonType() << OUT_DELIM
		<< it->getIonNum() << OUT_DELIM
		<< it->getFormatedLabel() << OUT_DELIM
		<< it->label.labelLoc.getX() << OUT_DELIM
		<< it->label.labelLoc.getY() << OUT_DELIM
		<< it->label.getIncludeArrow() << OUT_DELIM
		<< it->label.arrow.beg.getX() << OUT_DELIM
		<< it->label.arrow.beg.getY() << OUT_DELIM
		<< it->label.arrow.end.getX() << OUT_DELIM
		<< it->label.arrow.end.getY() << NEW_LINE;
	}
	std::cout.precision(ss);
	
	if(includeMetaData)
		out << ms2::END_SPECTRUM <<NEW_LINE;
}

void ms2::Spectrum::clear()
{
	scanNum = 0;
	retTime = 0;
	precursorInt = 0;
	precursorFile.clear();
	precursorScan = 0;
	charge = 0;
	precursorMZ = "";
	maxInt = 0;
	ions.clear();
	ions.shrink_to_fit();
}

//itterates through all ions and returns max intensity
double ms2::Spectrum::calcMaxInt() const
{
	double ret = 0;
	for(ionsTypeConstIt it = ions.begin(); it != ions.end(); ++it)
		if(it->getIntensity() > ret)
			ret = it->getIntensity();
	
	return ret;
}

//itterates through all ions and returns max intensity
double ms2::Spectrum::calcMinInt() const
{
	double ret = maxInt;
	for(ionsTypeConstIt it = ions.begin(); it != ions.end(); ++it)
		if(it->getIntensity() < ret)
			ret = it->getIntensity();
	
	return ret;
}

void ms2::Spectrum::updateDynamicMetadata()
{
	maxInt = calcMaxInt();
	minInt = calcMinInt();
	minMZ = ions.front().getMZ();
	maxMZ = ions.back().getMZ();
	mzRange = maxMZ - minMZ;
}

void ms2::Spectrum::setLabelTop(size_t labelTop)
{
	typedef std::list<ms2::DataPoint*> listType;
	listType pointList;
	
	for(ionVecType::iterator it = ions.begin(); it != ions.end(); ++it)
		pointList.push_back(&(*it));
	
	if(pointList.size() > labelTop)
	{
		pointList.sort(ms2::DataPoint::IntComparison());
		pointList.resize(labelTop);
	}
	
	for(listType::iterator it = pointList.begin(); it != pointList.end(); ++it)
		(*it)->setTopAbundant(true);
}

void ms2::Spectrum::setMZRange(double minMZ, double maxMZ, bool _sort)
{
	//sort ions by mz
	if(_sort)
		sort(ions.begin(), ions.end(), DataPoint::MZComparison());
	
	ionsTypeIt begin = ions.begin();
	ionsTypeIt end = ions.end();
	
	//find min mz
	for(ionsTypeIt it = ions.begin(); it != ions.end(); ++it)
	{
		if(it->getMZ() <= minMZ)
		{
			begin = it;
			continue;
		}
		else break;
	}
	//find max mz
	for(ionsTypeIt it = begin; it != ions.end(); ++it)
	{
		if(it->getMZ() < maxMZ)
		{
			end = it;
			continue;
		}
		else break;
	}
	
	ions = ionVecType(begin, end);
	updateDynamicMetadata();
}

void ms2::Spectrum::removeUnlabeledIons()
{
	for(ionsTypeIt it = ions.begin(); it != ions.end();)
	{
		if(!it->getForceLabel())
			ions.erase(it);
		else ++it;
	}
	
	updateDynamicMetadata();
}

void ms2::Spectrum::removeIntensityBelow(double minInt)
{
	for(ionsTypeIt it = ions.begin(); it != ions.end();)
	{
		if(it->getIntensity() < minInt)
			ions.erase(it);
		else ++it;
	}
	
	updateDynamicMetadata();
}

void ms2::Spectrum::labelSpectrum(PeptideNamespace::Peptide& peptide,
								  const base::ParamsBase& pars,
								  bool removeUnlabeledFrags, size_t labelTop)
{
	size_t len = peptide.getNumFragments();
	size_t labledCount = 0;
	sequence = peptide.getSequence();
	fullSequence = peptide.getFullSequence();
	double _labelTolerance; // = pars.getMatchTolerance();
	bool seqPrinted = false;
	
	typedef std::list<ms2::DataPoint*> listType;
	listType rangeList;
	listType::iterator label;
	
	setLabelTop(labelTop); //determine which ions are abundant enough to considered in labeling
	std::sort(ions.begin(), ions.end(), DataPoint::MZComparison()); //sort ions by mz
	if(pars.getMZSpecified()) //set user specified mz range if specified
	{
		setMZRange(pars.getMinMZSpecified() ? pars.getMinMZ() : minMZ,
				   pars.getMaxMZSpecified() ? pars.getMaxMZ() : maxMZ,
				   false);
	}
	
	//iterate through all calculated fragment ions and label ions on spectrum if they are found
	for(size_t i = 0; i < len; i++)
	{
		/*std::string temp = peptide.getFragment(i).getIonStr(false);
		if(temp == "b11")
			std::cout << "Found!" << std::endl;*/
		
		double tempMZ = peptide.getFragmentMZ(i);
		
		rangeList.clear();
		
		//first get lowest value in range
		_labelTolerance = pars.getMatchTolerance(tempMZ);
		ionsTypeIt lowerBound = std::lower_bound(ions.begin(), ions.end(),
												 (tempMZ - (_labelTolerance)),
												 DataPoint::MZComparison());
		
		//ittreate throughout all ions above in range
		for(ionsTypeIt it = lowerBound; it != ions.end(); ++it)
		{
			if(it->getMZ() > (tempMZ + _labelTolerance))
				break;
			
			if(it->getTopAbundant())
			{
				bool inRange = utils::inRange(it->getMZ(), tempMZ, _labelTolerance);
				bool intenseEnough = (it->getIntensity() > pars.getMinLabelIntensity());
				
				if(inRange && //check that it->mz is in range
				   intenseEnough) //check that it->int is sufficiently high
					rangeList.push_back(&(*it));
			}//end of if
		}
		//else continue;
		
		if(rangeList.size() == 0)
			continue;
		
		label = rangeList.begin();
		if(rangeList.size() > 1)
		{
			//iterate through all ions within matchTolerance range to get the one with max intensity
			double tempMax = (*label)->getIntensity();
			double tempMZdiff = abs((*label)->getMZ() - tempMZ);
			for(listType::iterator it = rangeList.begin(); it != rangeList.end(); ++it)
			{
				if(pars.getMultipleMatchCompare() == "intensity" || pars.getMultipleMatchCompare() == "int"){
					if((*it)->getIntensity() > tempMax)
					{
						label = it;
						tempMax = (*label)->getIntensity();
					}
				}
				else if(pars.getMultipleMatchCompare() == "mz"){
					if(((*it)->getMZ() - tempMZ) < tempMZdiff)
					{
						//TODO: test that this works
						label = it;
						tempMZdiff = abs((*label)->getMZ() - tempMZ);
					}
				}
				else{
					throw std::runtime_error("Unknown multipleMatchCompare method!");
				}
			}
		}
		
		if((*label)->getLabeledIon() && pars.getVerbose()){
			if(!seqPrinted){
				std::cout << "In sequence: " << peptide.getSequence() << NEW_LINE;
				seqPrinted = true;
			}
			std::cout << "\tDuplicate label found: " << (*label)->getLabel() << ", " <<
			peptide.getFragmentLabel(i) << NEW_LINE;
		}
		
		//if label is not already labeled or if peptide.getFragment(i) is not a NL
		if(!(*label)->getLabeledIon() || !peptide.getFragment(i).isNL())
		{
			(*label)->setLabel(peptide.getFragmentLabel(i));
			(*label)->setFormatedLabel(peptide.getFormatedLabel(i));
			(*label)->setLabeledIon(true);
			(*label)->label.setIncludeLabel(true);
			(*label)->setIonType(peptide.getFragment(i).ionTypeToStr());
			(*label)->setIonNum(peptide.getFragment(i).getNum());
			labledCount++;
		}
		peptide.setFound(i, true);
	}//end of for
	ionPercent = (double(labledCount) / double(len)) * 100;
	
	//remove unlabeled ions if necisary
	if(!pars.getIncludeAllIons())
		removeUnlabeledIons();
	
	//remove unlabeled peptide fragmetns
	//only used for debugging
	if(removeUnlabeledFrags)
		peptide.removeUnlabeledFrags();
	
	//remove ions below specified intensity
	if(pars.getMinIntensitySpecified())
		removeIntensityBelow(pars.getMinIntensity());
	
}//end of function

void ms2::Spectrum::makePoints(labels::Labels& labs, double maxPerc,
							   double offset_x, double offset_y,
							   double x_padding, double y_padding)
{
	for(ionVecType::iterator it = ions.begin(); it != ions.end(); ++it)
	{
		if(it->getIntensity() >= maxPerc || it->getLabeledIon())
		{
			if(!it->getLabeledIon())
			{
				it->setFormatedLabel(std::to_string(it->getMZ()));
				it->setLabel(std::to_string(it->getMZ()));
				it->label.setIncludeLabel(true);
				it->label.labelLoc = geometry::Rect(it->getMZ() + offset_x,
													it->getIntensity() + offset_y, x_padding, y_padding);
			}
			else {
				it->label.labelLoc = geometry::Rect(it->getMZ() + offset_x,
													it->getIntensity() + offset_y, x_padding, y_padding);
			}
			labs.push_back_labeledPoint(&(it->label));
			labs.push_back_dataPoint(geometry::Rect(it->getMZ(), it->getIntensity() / 2, POINT_PADDING,
													it->getIntensity()));
		}
	}
}

void ms2::Spectrum::calcLabelPos()
{
	double const xPadding = mzRange / 22.31972;
	double const yPadding = 6;
	
	calcLabelPos(DEFAULT_MAX_PERC,
				 DEFAULT_X_OFFSET, DEFAULT_Y_OFFSET,
				 xPadding, yPadding);
}

void ms2::Spectrum::calcLabelPos(double maxPerc,
								 double offset_x, double offset_y,
								 double x_padding, double y_padding)
{
	//initialize points
	labels::Labels labs(minMZ, maxMZ, minInt, maxInt);
	
	makePoints(labs, maxPerc, offset_x, offset_y, x_padding, y_padding);
	labs.spaceOutAlg2();
	
	//copy corrected points to spectrum
	//copyPointsToSpectrum(points);
}

