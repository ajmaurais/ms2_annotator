//
//  aaDB.hpp
//  ms2_anotator
//
//  Created by Aaron Maurais on 11/6/17.
//  Copyright © 2017 Aaron Maurais. All rights reserved.
//

#ifndef aaDB_hpp
#define aaDB_hpp

#include <iostream>
#include <map>
#include <utils.hpp>

namespace aaDB{
	
	std::string const SMOD_END_TAG = "</staticModifications>";
	std::string const SMOD_BEGIN_TAG = "<staticModifications>";
	int const MAX_PARAM_ITERATIONS = 1000;
	
	class AminoAcid;
	class AADB;
	
	typedef std::map<std::string, AminoAcid> aminoAcidsDBType;
	
	class AminoAcid{
	private:
		std::string symbol;
		double mass;
		double modification;
	
	public:
		AminoAcid(){
			symbol = ""; mass = 0; modification = 0;
		}
		AminoAcid(std::string);
		AminoAcid(std::string _symbol, double _mass, double _modification = 0){
			symbol = _symbol;
			mass = _mass;
			modification = _modification;
		}
		~AminoAcid() {};
		
		//modifers
		void operator += (double mod){
			modification += mod;
		}
		void operator = (const AminoAcid& _add){
			symbol = _add.symbol;
			mass = _add.mass;
			modification = _add.modification;
		}
		void setMass(double _mass){
			mass = _mass;
		}
		
		//properties
		std::string getSymbol() const{
			return symbol;
		}
		double getMass() const{
			return mass + modification;
		}
		
	};//end of class

	class AADB{
	private:
		typedef aminoAcidsDBType::const_iterator itType;
		aminoAcidsDBType aminoAcidsDB;
		
		//modifers
		bool readInAADB(std::string);
		bool readInModDB(std::string, aminoAcidsDBType&);
		void addStaticMod(const aminoAcidsDBType&, bool);
		
	public:
		//constructor
		AADB(){}
		~AADB(){}
		
		//modifers
		bool initalize(std::string aaDBLoc, std::string modDBLoc, bool showWarnings = true);
		bool initalize(std::string aaDBLoc, const aminoAcidsDBType&, bool showWarnings = true);
		
		//properties
		double calcMW(std::string sequence, bool addNTerm = true, bool addCTerm = true) const;
		double getMW(std::string) const;
		double getMW(char) const;
	};//end of class
	
}//end of namespace

#endif /* aaDB_hpp */
