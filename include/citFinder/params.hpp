//
//  params.hpp
//  citFinder
//
//  Created by Aaron Maurais on 12/9/18.
//  Copyright © 2018 Aaron Maurais. All rights reserved.
//

#ifndef params_hpp
#define params_hpp

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <thread>

#include <citFinder/citFinder.hpp>
#include <paramsBase.hpp>
#include <utils.hpp>

namespace CitFinder{
	
	//program file locations
	std::string const PROG_USAGE_FNAME = base::PROG_DB + "/citFinder/usage.txt";
	std::string const PROG_HELP_FILE = base::PROG_DB + "/citFinder/helpFile.txt";
	std::string const DEFAULT_FILTER_FILE_NAME = "DTASelect-filter.txt";
	
	//!default conflicting residues
	/**
	 This should probably be changed to blank string in a more general version
	 of this program.
	 */
	std::string const DEFAULT_AMBIGIOUS_RESIDUES = "NQ";
	
	double const DEFAULT_NEUTRAL_LOSS_MASS = 43.0058;
	
	class Params;
	
	class Params : public base::ParamsBase{
	public:
		typedef std::map<std::string, std::string> FilterFilesType;
		
	private:
		std::string _parentDir;
		FilterFilesType _filterFiles;
		//!Default name of DTAFilter filter file to search for
		std::string _dtaFilterBase;
		//! mass of neutral loss to search for
		double _neutralLossMass;
		std::string _ambigiousResidues;
		
		//! should reverse peptide matches be considered
		bool _includeReverse;
		//!Should annotaed spectra be printed?
		bool _printSpectraFiles;
		
		//! was a input directory specified
		bool _inDirSpecified;
		
		//! names of folders to read
		std::vector<std::string> _inDirs;
		
		//! number of thread to use. Default is std::thread::hardware_concurrency() / 2
		unsigned int _numThread;
		
		bool getFlist();
		unsigned int computeThreads() const;
		
	public:
		
		Params() : ParamsBase(PROG_USAGE_FNAME, PROG_HELP_FILE){
			_parentDir = "";
			_includeReverse = false;
			_printSpectraFiles = false;
			_dtaFilterBase = DEFAULT_FILTER_FILE_NAME;
			_neutralLossMass = DEFAULT_NEUTRAL_LOSS_MASS;
			_ambigiousResidues = DEFAULT_AMBIGIOUS_RESIDUES;
			ofname = "peptide_cit_stats.tsv";
			_numThread = computeThreads();
			_inDirSpecified = false;
		}
		
		//modifers
		bool getArgs(int, const char* const[]);
		
		//properties
		const FilterFilesType& getFilterFiles() const{
			return _filterFiles;
		}
		bool getIncludeReverse() const{
			return _includeReverse;
		}
		std::string getInputModeIndependentParentDir() const;
		double getNeutralLossMass() const{
			return _neutralLossMass;
		}
		std::string getAmbigiousResidues() const{
			return _ambigiousResidues;
		}
		std::string makeOfname() const{
			if(_inDirSpecified)
				return _wd + "/" + ofname;
			else{
				assert(_inDirs.size() == 1);
				return _inDirs.back() + "/" + ofname;
			}
		}
		bool getPrintSpectraFiles() const{
			return _printSpectraFiles;
		}
		unsigned int getNumThreads() const{
			return _numThread;
		}
		bool getInDirSpecified() const{
			return _inDirSpecified;
		}
	};
}

#endif /* params_hpp */
