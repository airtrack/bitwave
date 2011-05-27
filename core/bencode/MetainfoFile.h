#ifndef METAINFO_FILE_H
#define METAINFO_FILE_H

#include "BenTypes.h"
#include "../../base/BaseTypes.h"
#include "../../sha1/Sha1Value.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bitwave {
namespace core {
namespace bentypes {

    class MetainfoFileExeception : public BaseException
    {
    public:
        explicit MetainfoFileExeception(const char *w) : BaseException(w) { }
    };

    // All strings encoded by UTF-8 in this class
    class MetainfoFile : private NotCopyable
    {
    public:
        struct FileInfo
        {
            long long length;
            std::vector<std::string> path;
        };

        explicit MetainfoFile(const char *filepath);

        void GetAnnounce(std::vector<std::string> *announce) const;

        bool IsSingleFile() const;

        // return file name when IsSingleFile is true, otherwise
        // return a directory name, this just advisory.
        std::string Name() const;

        std::size_t PieceLength() const;
        std::size_t PiecesCount() const;
        Sha1Value GetPieceSha1(std::size_t index) const;

        long long Length() const;
        void GetFiles(std::vector<FileInfo> *files) const;

        // return raw info value buffer, first is begin, second is end
        std::pair<const char *, const char *> GetRawInfoValue() const;

    private:
        bool PrepareBasicData();
        void GetTheFile(std::vector<FileInfo> *files) const;
        void GetFileList(std::vector<FileInfo> *files) const;

        BenTypesStreamBuf metafilebuf_;
        std::tr1::shared_ptr<BenType> metainfo_;
        BenString *ann_;
        BenList *annlist_;
        BenDictionary *infodic_;
        BenString *pieces_;
    };

} // namespace bentypes
} // namespace core
} // namespace bitwave

#endif // METAINFO_FILE_H
