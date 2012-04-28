#ifndef _FILELIST_H_
#define _FILELIST_H_

#include <vector>

#include "SourceFile.h"
#include "TextFile.h"

class FileList {
  private:
    std::vector<SourceFile> m_sourceFiles;
    unsigned m_nFiles;
    unsigned m_maxLinesPerFile;
    unsigned m_locsTotal;
    unsigned m_minChars;
    bool m_ignorePrepStuff;
  public:
    FileList(const TextFile& listOfFiles, unsigned minChars, bool ignorePrepStuff) :
      m_nFiles(0),
      m_maxLinesPerFile(0),
      m_locsTotal(0),
      m_minChars(minChars),
      m_ignorePrepStuff(ignorePrepStuff)
  {
    std::vector<std::string> lines;
    listOfFiles.readLines(lines, true);

    // Create vector with all source files
    for (unsigned i=0; i<lines.size(); i++) {
      addFile(lines[i]);
    }
  }

    unsigned getNFiles() const {
      return m_nFiles;
    }

    unsigned getMaxLinesPerFile() const {
      return m_maxLinesPerFile;
    }

    unsigned getLOCsTotal() const {
      return m_locsTotal; 
    }

    const std::vector<SourceFile>& getSourceFilesRaw() const {
      return m_sourceFiles;
    }

    void addFile(const std::string& fname) {
      if (fname.size() > 5) {
        SourceFile pSourceFile(fname, m_minChars, m_ignorePrepStuff);
        const unsigned numLines = pSourceFile.getNumOfLines();
        if (numLines > 0) {
          ++m_nFiles;
          m_sourceFiles.push_back(pSourceFile);
          m_locsTotal += numLines;
          if (m_maxLinesPerFile < numLines) {
            m_maxLinesPerFile = numLines;
          }
        }
      }
    }
};

#endif
