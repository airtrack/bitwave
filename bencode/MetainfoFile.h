#ifndef _METAINFO_FILE_H_
#define _METAINFO_FILE_H_

#include "../base/BaseTypes.h"
#include "BenTypes.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bittorrent
{

    class MetainfoFileExeception : public BaseException
    {
    public:
        explicit MetainfoFileExeception(const char *w) : BaseException(w) { }
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

        // return raw info value buffer, first is begin, second is end
        std::pair<const char *, const char *> GetRawInfoValue() const;

    private:
        bool PrepareBasicData();

        std::tr1::shared_ptr<bentypes::BenType> metainfo_;
        bentypes::BenString *ann_;
        bentypes::BenList *annlist_;
        bentypes::BenDictionary *infodic_;
        bentypes::BenString *pieces_;

        // metafile raw data
        std::tr1::shared_ptr<bentypes::BenTypesStreamBuf> metafilebuf_;
    };

} // namespace bittorrent

#endif // _METAINFO_FILE_H_
