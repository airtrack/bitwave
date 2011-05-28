#include "MetainfoFile.h"
#include "../BitException.h"
#include <assert.h>
#include <algorithm>
#include <functional>
#include <iterator>

namespace bitwave {
namespace core {
namespace bentypes {

    MetainfoFile::MetainfoFile(const char *filepath)
        : metafilebuf_(filepath),
          metainfo_(GetBenObject(metafilebuf_)),
          ann_(0),
          annlist_(0),
          infodic_(0),
          pieces_(0)
    {
        if (!metainfo_ || !PrepareBasicData())
        {
            std::string info = std::string("invalid torrent file: ") + filepath;
            throw MetainfoFileExeception(info);
        }
    }

    void MetainfoFile::GetAnnounce(std::vector<std::string> *announce) const
    {
        assert(announce);
        if (annlist_)
        {
            announce->reserve(annlist_->size());

            std::vector<BenList *> vl;
            annlist_->AllElementPtr(&vl);

            std::vector<BenString *> vs;
            std::for_each(vl.begin(), vl.end(),
                std::bind2nd(
                    std::mem_fun(&BenList::AllElementPtr<BenString>), &vs));

            std::transform(vs.begin(), vs.end(),
                std::back_inserter(*announce),
                    std::mem_fun(&BenString::std_string));
        }
        else
        {
            announce->push_back(ann_->std_string());
        }
    }

    bool MetainfoFile::IsSingleFile() const
    {
        BenInteger *length = infodic_->ValueBenTypeCast<BenInteger>("length");
        return length != 0;
    }

    std::string MetainfoFile::Name() const
    {
        BenString *name = infodic_->ValueBenTypeCast<BenString>("name");
        if (name)
            return name->std_string();
        return std::string();
    }

    std::size_t MetainfoFile::PieceLength() const
    {
        BenInteger *pl = infodic_->ValueBenTypeCast<BenInteger>("piece length");
        if (pl)
            return static_cast<std::size_t>(pl->GetValue());
        return 0;
    }

    std::size_t MetainfoFile::PiecesCount() const
    {
        return pieces_->length() / 20;
    }

    Sha1Value MetainfoFile::GetPieceSha1(std::size_t index) const
    {
        const char *data = pieces_->data() + index * 20;
        const unsigned *sha1 = reinterpret_cast<const unsigned *>(data);
        return Sha1Value(sha1);
    }

    long long MetainfoFile::Length() const
    {
        BenInteger *length = infodic_->ValueBenTypeCast<BenInteger>("length");
        if (length)
            return length->GetValue();
        return 0;
    }

    void MetainfoFile::GetFiles(std::vector<FileInfo> *files) const
    {
        if (IsSingleFile())
            GetTheFile(files);
        else
            GetFileList(files);
    }

    std::pair<const char *, const char *> MetainfoFile::GetRawInfoValue() const
    {
        const char *begin = metafilebuf_.iter_data(infodic_->GetSrcBufBegin());
        const char *end = metafilebuf_.iter_data(infodic_->GetSrcBufEnd());
        return std::make_pair(begin, end);
    }

    bool MetainfoFile::PrepareBasicData()
    {
        BenDictionary *dic = dynamic_cast<BenDictionary *>(metainfo_.get());
        if (!dic) return false;

        ann_ = dic->ValueBenTypeCast<BenString>("announce");
        if (!ann_) return false;

        annlist_ = dic->ValueBenTypeCast<BenList>("announce-list");

        infodic_ = dic->ValueBenTypeCast<BenDictionary>("info");
        if (!infodic_) return false;

        pieces_ = infodic_->ValueBenTypeCast<BenString>("pieces");
        if (!pieces_) return false;

        if (pieces_->length() % 20) return false;

        return true;
    }

    void MetainfoFile::GetTheFile(std::vector<FileInfo> *files) const
    {
        assert(files);
        FileInfo the_file;
        the_file.length = Length();
        the_file.path.push_back(Name());
        files->push_back(the_file);
    }

    void MetainfoFile::GetFileList(std::vector<FileInfo> *files) const
    {
        assert(files);
        BenList *fs = infodic_->ValueBenTypeCast<BenList>("files");
        if (!fs)
            return ;
        files->reserve(fs->size());

        std::string base_path = Name();
        std::vector<BenDictionary *> vd;
        fs->AllElementPtr(&vd);
        for (std::vector<BenDictionary *>::iterator it = vd.begin(); it != vd.end(); ++it)
        {
            BenDictionary *file = *it;
            BenInteger *len = file->ValueBenTypeCast<BenInteger>("length");
            if (!len) continue;

            FileInfo file_info;
            file_info.length = len->GetValue();

            BenList *pathlist = file->ValueBenTypeCast<BenList>("path");
            if (!pathlist) continue;

            // reserve for base path and file path
            file_info.path.reserve(pathlist->size() + 1);
            file_info.path.push_back(base_path);

            std::vector<BenString *> vs;
            pathlist->AllElementPtr(&vs);

            std::transform(vs.begin(), vs.end(),
                    std::back_inserter(file_info.path),
                    std::mem_fun(&BenString::std_string));

            files->push_back(file_info);
        }
    }

} // namespace bentypes
} // namespace core
} // namespace bitwave
