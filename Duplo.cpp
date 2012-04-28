/**
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "Duplo.h"

#include <fstream>
#include <string.h>
#include <time.h>

#include "SourceFile.h"
#include "SourceLine.h"

#include "StringUtil.h"
#include "HashUtil.h"
#include "TextFile.h"
#include "ArgumentParser.h"

class Logger {
  bool m_verbose;
  std::ostream& m_os;
  public:
    explicit Logger(std::ostream& os=std::cout) :
      m_verbose(false),
      m_os(os)
    { }

    void setVerbose(bool verbose=true) {
      m_verbose = verbose;
    }

    template <typename T>
    Logger& operator<<(const T& msg) {
      if (m_verbose)
        m_os << msg;
      return *this;
    }
};

namespace {
  const int MIN_BLOCK_SIZE = 4;
  const int MIN_CHARS = 3;
  Logger logger;
}

Duplo::Duplo(const FileList& fileList) :
    m_fileList(fileList),
    m_minBlockSize(0),
    m_minChars(0),
    m_ignorePrepStuff(false),
    m_ignoreSameFilename(false),
    m_DuplicateLines(0),
    m_Xml(false)
{ }

void Duplo::reportSeq(int line1, int line2, int count, const SourceFile& pSource1, const SourceFile& pSource2, std::ostream& outFile){
    if (m_Xml)
    {
        outFile << "    <set LineCount=\"" << count << "\">" << std::endl;
        outFile << "        <block SourceFile=\"" << pSource1.getFilename() << "\" StartLineNumber=\"" << pSource1.getLine(line1).getLineNumber() << "\"/>" << std::endl;
        outFile << "        <block SourceFile=\"" << pSource2.getFilename() << "\" StartLineNumber=\"" << pSource2.getLine(line2).getLineNumber() << "\"/>" << std::endl;
        outFile << "        <lines xml:space=\"preserve\">" << std::endl;
        for(int j = 0; j < count; j++)
        {
            // replace various characters/ strings so that it doesn't upset the XML parser
            std::string tmpstr = pSource1.getLine(j+line1).getLine();

            // " --> '
            StringUtil::StrSub(tmpstr, "\'", "\"", -1);

            // & --> &amp;
            StringUtil::StrSub(tmpstr, "&amp;", "&", -1);

            // < --> &lt;
            StringUtil::StrSub(tmpstr, "&lt;", "<", -1);

            // > --> &gt;
            StringUtil::StrSub(tmpstr, "&gt;", ">", -1);

            outFile << "            <line Text=\"" << tmpstr << "\"/>" << std::endl;
            m_DuplicateLines++;
        }
        outFile << "        </lines>" << std::endl;
        outFile << "    </set>" << std::endl;
    }
    else
    {
        outFile << pSource1.getFilename() << "(" << pSource1.getLine(line1).getLineNumber() << ")" << std::endl;
	    outFile << pSource2.getFilename() << "(" << pSource2.getLine(line2).getLineNumber() << ")" << std::endl;
	    for(int j=0;j<count;j++){
		    outFile << pSource1.getLine(j+line1).getLine() << std::endl;
		    m_DuplicateLines++;
	    }
	    outFile << std::endl;
    }
}

int Duplo::process(const SourceFile& pSource1, const SourceFile& pSource2, std::ostream& outFile){
	const int m = pSource1.getNumOfLines();
	const int n = pSource2.getNumOfLines();

    const unsigned char NONE = 0;
    const unsigned char MATCH = 1;

    // Reset matrix data
    std::fill(m_pMatrix.begin(), m_pMatrix.end(), NONE);

    // Compute matrix
	for(int y=0; y<m; y++){
	    const SourceLine& pSLine = pSource1.getLine(y);
        for(int x=0; x<n; x++){
            if(pSLine.equals(pSource2.getLine(x))){
                m_pMatrix[x+n*y] = MATCH;
            }
        }
    }

	int blocks=0;

    // Scan vertical part
    for(int y=0; y<m; y++){
        unsigned int seqLen=0;
        int maxX = MIN(n, m-y);
        for(int x=0; x<maxX; x++){
            if(m_pMatrix[x+n*(y+x)] == MATCH){
                seqLen++;
            } else {
                if(seqLen >= m_minBlockSize){
                    reportSeq(y+x-seqLen, x-seqLen, seqLen, pSource1, pSource2, outFile);
                    blocks++;
                }
                seqLen=0;
            }
        }

        if(seqLen >= m_minBlockSize){
            reportSeq(m-seqLen, n-seqLen, seqLen, pSource1, pSource2, outFile);
            blocks++;
        }
    }

    // Scan horizontal part
    for(int x=1; x<n; x++){
        unsigned int seqLen=0;
        int maxY = MIN(m, n-x);
        for(int y=0; y<maxY; y++){
            if(m_pMatrix[x+y+n*y] == MATCH){
                seqLen++;
            } else {
                if(seqLen >= m_minBlockSize){
                    reportSeq(y-seqLen, x+y-seqLen, seqLen, pSource1, pSource2, outFile);
                    blocks++;
                }
                seqLen=0;
            }
        }

        if(seqLen >= m_minBlockSize){
            reportSeq(m-seqLen, n-seqLen, seqLen, pSource1, pSource2, outFile);
            blocks++;
        }
    }

	return blocks;
}

const std::string Duplo::getFilenamePart(const std::string& fullpath){
    std::string path = StringUtil::substitute('\\', '/', fullpath);

    std::string filename = path;

    std::string::size_type idx = path.rfind('/');
    if(idx != std::string::npos){
        filename = path.substr(idx+1, path.size()-idx-1);
    }

    return filename;
}

bool Duplo::isSameFilename(const SourceFile& file1, const SourceFile& file2) {
    return (getFilenamePart(file1.getFilename()) == getFilenamePart(file2.getFilename()) && m_ignoreSameFilename);
}

void Duplo::run(std::ostream& outfile) {

    if (m_Xml)
    {
        outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        outfile << "<?xml-stylesheet href=\"duplo.xsl\" type=\"text/xsl\"?>" << std::endl;
        outfile << "<duplo version=\"" << VERSION << "\">" << std::endl;
        outfile << "    <check Min_block_size=\"" << m_minBlockSize << 
            "\" Min_char_line=\"" << m_minChars << 
            "\" Ignore_prepro=\"" << (m_ignorePrepStuff ? "true" : "false") << 
            "\" Ignore_same_filename=\"" << (m_ignoreSameFilename ? "true" : "false") << "\">" << std::endl;
    }

    clock_t start, finish;
    double  duration;

    start = clock();

    // Generate matrix large enough for all files
    const int maxLinesPerFile = m_fileList.getMaxLinesPerFile();
    m_pMatrix.resize(maxLinesPerFile*maxLinesPerFile);

    int blocksTotal = 0;

    // Compare each file with each other
    size_t nSourceFiles = m_fileList.getSourceFilesRaw().size();
    for (size_t i=0; i<nSourceFiles; i++) {
        const SourceFile& sourceFile1 = m_fileList.getSourceFilesRaw()[i];
        logger << sourceFile1.getFilename();

        int blocks = 0;
        for (size_t j=i+1; j<nSourceFiles; j++) {
          const SourceFile& sourceFile2 = m_fileList.getSourceFilesRaw()[j];
            if (isSameFilename(sourceFile1, sourceFile2)) {
              continue;
            }
            blocks+=process(sourceFile1, sourceFile2, outfile);
        }

        if (blocks > 0) {
            logger << " found " << blocks << " block(s)\n";
        } else {
            logger << " nothing found\n";
        }

        blocksTotal+=blocks;
	}

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    logger << "Time: "<< duration << " seconds\n";

    if (m_Xml)
    {
        outfile << "        <summary Num_files=\"" << m_fileList.getNFiles() <<
            "\" Duplicate_blocks=\"" << blocksTotal <<
            "\" Total_lines_of_code=\"" << m_fileList.getLOCsTotal() <<
            "\" Duplicate_lines_of_code=\"" << m_DuplicateLines <<
            "\" Time=\"" << duration <<
            "\"/>" << std::endl;
        outfile << "    </check>" << std::endl;
        outfile << "</duplo>" << std::endl;
    }
    else
    {
        outfile << "Configuration: " << std::endl;
        outfile << "  Number of files: " << m_fileList.getNFiles() << std::endl;
        outfile << "  Minimal block size: " << m_minBlockSize << std::endl;
        outfile << "  Minimal characters in line: " << m_minChars << std::endl;
        outfile << "  Ignore preprocessor directives: " << m_ignorePrepStuff << std::endl;
        outfile << "  Ignore same filenames: " << m_ignoreSameFilename << std::endl;
        outfile << std::endl;
        outfile << "Results: " << std::endl;
        outfile << "  Lines of code: " << m_fileList.getLOCsTotal() << std::endl;
        outfile << "  Duplicate lines of code: " << m_DuplicateLines << std::endl;
        outfile << "  Total " << blocksTotal << " duplicate block(s) found." << std::endl << std::endl;
        outfile << "  Time: " << duration << " seconds" << std::endl;
    }
}


void showHelp() {
  std::cout << "\nNAME\n";
  std::cout << "       Duplo " << VERSION << " - duplicate source code block finder\n\n";

  std::cout << "\nSYNOPSIS\n";
  std::cout << "       duplo [OPTIONS] [-i INTPUT_FILELIST] [-o OUTPUT_FILE] [FILES]\n";

  std::cout << "\nDESCRIPTION\n";
  std::cout << "       Duplo is a tool to find duplicated code blocks in large\n";
  std::cout << "       C/C++/Java/C#/VB.Net software systems.\n\n";

  std::cout << "       -ml                 minimal block size in lines (default is " << MIN_BLOCK_SIZE << ")\n";
  std::cout << "       -mc                 minimal characters in line (default is " << MIN_CHARS << ")\n";
  std::cout << "                           lines with less characters are ignored\n";
  std::cout << "       -ip                 ignore preprocessor directives\n";
  std::cout << "       -d                  ignore file pairs with same name\n";
  std::cout << "       -xml                output file in XML\n";
  std::cout << "       -verbose            report progress\n";
  std::cout << "       -i INTPUT_FILELIST  input filelist\n";
  std::cout << "       -o OUTPUT_FILE      output file\n";
  std::cout << "\n";
  std::cout << "       Additional arguments are interpreted as input files to be analyzed.\n";

  std::cout << "\nVERSION\n";
  std::cout << "       " << VERSION << "\n";

  std::cout << "\nAUTHORS\n";
  std::cout << "       Christian M. Ammann (cammann@giants.ch)\n";
  std::cout << "       Trevor D'Arcy-Evans (tdarcyevans@hotmail.com)\n\n";
}
/**
 * Main routine
 *
 * @param argc  number of arguments
 * @param argv  arguments
 */
int main(int argc, char* argv[]) {
  ArgumentParser ap(argc, argv);
  logger.setVerbose(ap.is("-verbose"));

  const unsigned minChars = ap.getNumeric("-mc", MIN_CHARS);
  const bool ignorePrepStuff = ap.is("-ip");

  logger << "Loading and hashing files ... ";
  const std::string input_filelist = ap.getStr("-i", "");
  std::vector<std::string> files = ap.getNakedArguments();
  FileList fileList(input_filelist, minChars, ignorePrepStuff);
  for (unsigned i = 0; i<files.size(); i++) {
    fileList.addFile(files[i]);
  }
  logger << "done.\n\n";

  if (not ap.is("--help") and not (fileList.getNFiles()==0)) {
    Duplo duplo(fileList);

    // parse optional arguments
    duplo.setMinimalBlockSize(ap.getNumeric("-ml", MIN_BLOCK_SIZE));
    duplo.setMinChars(minChars);
    duplo.setIgnorePreprocessor(ignorePrepStuff);
    duplo.setIgnoreSameFilenamePairs(ap.is("-d"));
    duplo.setReportXML(ap.is("-xml"));

    std::string output_file_name = ap.getStr("-o", "");
    if (not output_file_name.empty()) {
      std::ofstream output_file(output_file_name.c_str(), std::ios::out|std::ios::binary);
      duplo.run(output_file);
    } else {
      duplo.run(std::cout);
    }

  } else {
    showHelp();
  }
}

