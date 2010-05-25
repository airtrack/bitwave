#ifndef _METAINFO_FILE_H_
#define _METAINFO_FILE_H_

#include "../base/BaseTypes.h"
#include "BenTypes.h"
#include <vector>
#include <string>

namespace bittorrent
{
    class OpenFileException : public BaseException
    {
    public:
        explicit OpenFileException(const char *w) : BaseException(w) { }
    };

    // All strings encoded by UTF-8 in this class
    class MetainfoFile
    {
    public:
        struct FileInfo
        {
            int length;
            std::vector<std::string> path;
        };

        explicit MetainfoFile(const char *filepath);

        void GetAnnounce(std::vector<std::string> *announce) const;

        bool IsSingleFile() const;
        // return file name when IsSingleFile is true, otherwise
        // return a directory name, this just advisory.
        std::string Name() const;

        int PieceLength() const;
        std::size_t PiecesCount() const;
        std::string Pieces(std::size_t index) const;

        int Length() const;
        void Files(std::vector<FileInfo> *files) const;

    private:
        bool CheckValid();
        std::tr1::shared_ptr<bentypes::BenType> metainfo_;
        bentypes::BenString *ann_;
        bentypes::BenList *annlist_;
        bentypes::BenDictionary *infodic_;
        bentypes::BenString *pieces_;
    };

} // namespace bittorrent

#endif // _METAINFO_FILE_H_