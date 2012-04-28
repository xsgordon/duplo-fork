/** \class Duplo
 * Duplo, main class
 *
 * @author  Christian Ammann (cammann@giants.ch)
 * @date  16/05/05
 *
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

#ifndef _DUPLO_H_
#define _DUPLO_H_

#include <deque>
#include <string>
#include <iostream>

#include "FileList.h"

class SourceFile;

const std::string VERSION = "0.2.0";

class Duplo {
protected:
    const FileList m_fileList;
    unsigned int m_minBlockSize;
    unsigned int m_minChars;
    bool m_ignorePrepStuff;
    bool m_ignoreSameFilename;
    int m_DuplicateLines;
    bool m_Xml;
    std::deque<unsigned char> m_pMatrix;

    void reportSeq(int line1, int line2, int count, const SourceFile& pSource1, const SourceFile& pSource2, std::ostream& outFile);
	int process(const SourceFile& pSource1, const SourceFile& pSource2, std::ostream& outFile);

    const std::string getFilenamePart(const std::string& fullpath);
    bool isSameFilename(const SourceFile& file1, const SourceFile& file2);

public:
    explicit Duplo(const FileList& fileList);
    void run(std::ostream& os);

    void setMinimalBlockSize(unsigned s) {
      m_minBlockSize = s;
    }

    void setMinChars(unsigned s) {
      m_minChars = s;
    }

    void setIgnorePreprocessor(bool s) {
      m_ignorePrepStuff = s;
    }

    void setIgnoreSameFilenamePairs(bool s) {
      m_ignoreSameFilename = s;
    }

    void setReportXML(bool s) {
      m_Xml = s;
    }
};

#endif

