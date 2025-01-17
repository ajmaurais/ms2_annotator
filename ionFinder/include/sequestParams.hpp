//
// sequestParams.hpp
// ionFinder
// -----------------------------------------------------------------------------
// MIT License
// Copyright 2020 Aaron Maurais
// -----------------------------------------------------------------------------
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// -----------------------------------------------------------------------------
//

#ifndef sequestParams_hpp
#define sequestParams_hpp

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#include <utils.hpp>
#include <aaDB.hpp>

namespace seqpar{

	std::string const DIFF_MOD_LINE = "diff_search_options";
	std::string const DATABSE_NAME_LINE = "database_name";
	
	class SequestParamsFile;
	
	class SequestParamsFile{
	private:
		std::map<std::string,std::string> smodMap;
		std::string fname;
		aaDB::aminoAcidsDBType aaMap;
		//!Path to fasta file used for database searching
		std::string fasta_path;
		
		void initSmodMap();
	public:
		
		SequestParamsFile(){
			initSmodMap();
			fname = "";
		}
		SequestParamsFile(std::string _fname){
			initSmodMap();
			fname = _fname;
		}
		
		bool read(std::string);
		bool read(){
			return read(fname);
		}
		
		aaDB::aminoAcidsDBType getAAMap() const{
			return aaMap;
		}
	};
}

#endif /* sequestParams_hpp */
