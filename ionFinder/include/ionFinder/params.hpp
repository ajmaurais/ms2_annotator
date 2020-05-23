//
//  params.hpp
//  ionFinder
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

#include <ionFinder/ionFinder.hpp>
#include <paramsBase.hpp>
#include <utils.hpp>

namespace IonFinder{
	
	//program file locations
	std::string const PROG_USAGE_FNAME = base::PROG_MAN + "/ionFinder/usage.txt";
	std::string const PROG_HELP_FILE = base::PROG_MAN + "/ionFinder/helpFile.roff";
	std::string const DEFAULT_FILTER_FILE_NAME = "DTASelect-filter.txt";
	std::string const PEPTIDE_MOD_STATS_OFNAME = "peptide_mod_stats.tsv";
	std::string const PEPTIDE_CIT_STATS_OFNAME = "peptide_cit_stats.tsv";
	std::string const DTAFILTER_INPUT_STR = "dtafilter";
	std::string const TSV_INPUT_STR = "tsv";
	std::string const ARG_REQUIRED_STR = "Additional argument required for: ";

    //!default residues which are isobaric with a modification
    std::string const DEFAULT_AMBIGIOUS_RESIDUES = "";
	std::string const CIT_AMB_RESIDUES = "NQ";
	
	double const CIT_NL_MASS = 43.0058;
	double const DEFAULT_NEUTRAL_LOSS_MASS = CIT_NL_MASS;
	
	class Params;
	
	class Params : public base::ParamsBase{
	public:
		typedef std::map<std::string, std::string> FilterFilesType;
		
	private:
		std::string _parentDir;
		//!Contains all filter files to be read
		FilterFilesType _filterFiles;
		//!how will peptides to be searched for be supplied?
		std::string _inputMode;
		//!Default name of DTAFilter filter file to search for
		std::string _dtaFilterBase;
		//! mass of neutral loss to search for
		double _neutralLossMass;
		//! Residues which could be isobaric for _neutralLossMass
		std::string _ambigiousResidues;
		
		//! should reverse peptide matches be considered
		bool _includeReverse;
		//! Which modification statuses should be included in output?
		int _modFilter;
		//!Should annotaed spectra be printed?
		bool _printSpectraFiles;
		//!Should NL ions be search for?
		bool _calcNL;
		//! Should c terminal modifications be incluced?
		bool _includeCTermMod;
		
		//! was a input directory specified
		bool _inDirSpecified;

		//!Intensity cutoff for NL ions.
		double _minNlLabelIntensity;

		//!Label artifact NL ions in .spectrum?
		bool _labelArtifactNL;

		//! Fraction of ion intensity allowed for artifact NL ions.
		double _artifactNLIntFrac;

		//! How should the artifact NL fraction be calculated?
		std::string _artifactNLIntMode;

		//! names of folders to read
		std::vector<std::string> _inDirs;
		
		//! number of thread to use. Default is std::thread::hardware_concurrency() / 2
		unsigned int _numThread;
		
		//! path of fasta file to get modified residue numbers
		std::string _fastaFile;

		//! How to deal with peptides with multiple modifications
		int _groupMod;

		//!Should unique peptide be printed?
		bool _printPeptideUID;
		
		bool getFlist(bool force);
		unsigned int computeThreads() const;

	public:
		
		Params() : ParamsBase(PROG_USAGE_FNAME, PROG_HELP_FILE){
			_parentDir = "";
			_fastaFile = "";
			_inputMode = DTAFILTER_INPUT_STR;
			_includeReverse = false;
			_modFilter = 1;
			_printSpectraFiles = false;
			_calcNL = false;
            _artifactNLIntFrac = 0.01;
            _artifactNLIntMode = "all";
			_includeCTermMod = true;
			_dtaFilterBase = DEFAULT_FILTER_FILE_NAME;
			_neutralLossMass = DEFAULT_NEUTRAL_LOSS_MASS;
			_ambigiousResidues = DEFAULT_AMBIGIOUS_RESIDUES;
			ofname = PEPTIDE_MOD_STATS_OFNAME;
			_numThread = 1;
			_inDirSpecified = false;
            _minNlLabelIntensity = 0;
            _labelArtifactNL = true;
			_groupMod = 1;
			_printPeptideUID = false;
		}
		
		//modifiers
		bool getArgs(int, const char* const[]) override;
		
		//properties
        void printVersion(std::ostream& = std::cout) const;
		const FilterFilesType& getFilterFiles() const{
			return _filterFiles;
		}
		const std::vector<std::string>& getInputDirs() const{
			return _inDirs;
		}
		bool getIncludeReverse() const{
			return _includeReverse;
		}
		int getModFilter() const{
			return _modFilter;
		}
		bool getCalcNL() const{
			return _calcNL;
		}
		double getNeutralLossMass() const{
			return _neutralLossMass;
		}
		std::string getInputMode() const{
			return _inputMode;
		}
		std::string getAmbigiousResidues() const{
			return _ambigiousResidues;
		}
		bool getIncludeCTermMod() const {
            return _includeCTermMod;
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
		std::string getFastaFile() const{
			return _fastaFile;
		}
		double getNlIntCo() const{
		    return _minNlLabelIntensity;
		}
		bool getLabelArtifactNL() const {
		    return _labelArtifactNL;
        }
        double getArtifactNLIntFrac() const {
		    return _artifactNLIntFrac;
        }
        std::string getArtifactNLIntMode() const {
		    return _artifactNLIntMode;
        }
		int getGroupMod() const{
		    return _groupMod;
		}
		bool getPrintPeptideUID() const {
            return _printPeptideUID;
        }
	};
}

#endif /* params_hpp */
