#include "MetainfoFile.h"
#include <assert.h>
#include <algorithm>
#include <functional>
#include <iterator>

namespace bittorrent {
namespace core {

    using namespace bentypes;

    MetainfoFile::MetainfoFile(const char *filepath)
        : metainfo_(),
          ann_(0),
          annlist_(0),
          infodic_(0),
          pieces_(0),
          metafilebuf_(new BenTypesStreamBuf(filepath))
    {
        metainfo_ = GetBenObject(*metafilebuf_);
        if (!metainfo_ || !PrepareBasicData())
        {
            std::string info = std::string("invalid file: ") + filepath;
            throw MetainfoFileExeception(info.c_str());
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

    int MetainfoFile::PieceLength() const
    {
        BenInteger *pl = infodic_->ValueBenTypeCast<BenInteger>("piece length");
        if (pl)
            return pl->GetValue();
        return 0;
    }

    std::size_t MetainfoFile::PiecesCount() const
    {
        return pieces_->length() / 20;
    }

    std::string MetainfoFile::Pieces(std::size_t index) const
    {
        return pieces_->std_string().substr(index * 20, 20);
    }

    int MetainfoFile::Length() const
    {
        BenInteger *length = infodic_->ValueBenTypeCast<BenInteger>("length");
        if (length)
            return length->GetValue();
        return 0;
    }

    void MetainfoFile::Files(std::vector<FileInfo> *files) const
    {
        assert(files);
        BenList *fs = infodic_->ValueBenTypeCast<BenList>("files");
        if (!fs) return ;

        files->reserve(fs->size());

        std::vector<BenDictionary *> vd;
        fs->AllElementPtr(&vd);
        for (std::vector<BenDictionary *>::iterator it = vd.begin(); it != vd.end(); ++it)
        {
            BenDictionary *file = *it;
            BenInteger *len = file->ValueBenTypeCast<BenInteger>("length");
            if (!len) continue;

            FileInfo obj;
            obj.length = len->GetValue();

            BenList *pathlist = file->ValueBenTypeCast<BenList>("path");
            if (!pathlist) continue;

            obj.path.reserve(pathlist->size());
            std::vector<BenString *> vs;
            pathlist->AllElementPtr(&vs);

            std::transform(vs.begin(), vs.end(),
                std::back_inserter(obj.path),
                    std::mem_fun(&BenString::std_string));

            files->push_back(obj);
        }
    }

    std::pair<const char *, const char *> MetainfoFile::GetRawInfoValue() const
    {
        const char *begin = metafilebuf_->iter_data(infodic_->GetSrcBufBegin());
        const char *end = metafilebuf_->iter_data(infodic_->GetSrcBufEnd());
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

} // namespace core
} // namespace bittorrent
