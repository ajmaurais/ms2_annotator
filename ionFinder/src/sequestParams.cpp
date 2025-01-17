//
// sequestParams.cpp
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

#include <sequestParams.hpp>

/**
 Initialize SequestParamsFile::SequestParamsFile.
 */
void seqpar::SequestParamsFile::initSmodMap()
{
	smodMap["add_C_terminus"] = "C_term";
	smodMap["add_N_terminus"] = "N_term";
	smodMap["add_G_Glycine"] = "G";
	smodMap["add_A_Alanine"] = "A";
	smodMap["add_S_Serine"] = "S";
	smodMap["add_P_Proline"] = "P";
	smodMap["add_V_Valine"] = "V";
	smodMap["add_T_Threonine"] = "T";
	smodMap["add_C_Cysteine"] = "C";
	smodMap["add_L_Leucine"] = "L";
	smodMap["add_I_Isoleucine"] = "I";
	smodMap["add_N_Asparagine"] = "N";
	smodMap["add_O_Ornithine"] = "O";
	smodMap["add_D_Aspartic_Acid"] = "D";
	smodMap["add_Q_Glutamine"] = "Q";
	smodMap["add_K_Lysine"] = "K";
	smodMap["add_E_Glutamic_Acid"] = "E";
	smodMap["add_M_Methionine"] = "M";
	smodMap["add_H_Histidine"] = "H";
	smodMap["add_F_Phenyalanine"] = "F";
	smodMap["add_R_Arginine"] = "R";
	smodMap["add_Y_Tyrosine"] = "Y";
	smodMap["add_W_Tryptophan"] = "W";
}

bool seqpar::SequestParamsFile::read(std::string _fname)
{
	if(fname.empty())
		throw std::runtime_error("fname must be specified!");
	
	std::ifstream inF(fname);
	if(!inF) return false;
	
	std::string line;
	std::vector<std::string> elems;
	while(utils::safeGetline(inF, line))
	{
		line = utils::trim(line);
		if(utils::isCommentLine(line) || line.empty())
			continue;
		
		//get amino acid modifications
		if(utils::strContains('=', line))
		{
			utils::split(line, '=', elems);
			utils::trimAll(elems);
			
			if(elems[0] == DATABSE_NAME_LINE) //get fasta file
			{
				fasta_path = elems.at(1);
			}
			else if(smodMap.find(elems[0]) != smodMap.end()) //if line contains smod for an aa
			{
				std::string modMass = elems[1];
				std::string aa = smodMap[elems[0]];
				modMass = utils::trim(modMass.substr(0, modMass.find(";")));
				aaMap[aa] = aaDB::AminoAcid(aa, 0, std::stod(modMass));
			}
			else if(elems[0] == DIFF_MOD_LINE) //if line is diffmod line
			{
				std::string diffModsLine = elems[1];
				elems.clear();
				std::istringstream iss(diffModsLine);
				for(std::string s; iss >> s; )
					elems.push_back(s);
				
				if(elems.size() > 2)
				{
					std::cerr << "Multiple diffmods detected. Use smod file with multiple diffmods." << NEW_LINE;
					return false;
				}
				aaMap["*"] = aaDB::AminoAcid("*", 0, std::stod(elems[0]));
			}
		}
	}
	
	return true;
}







