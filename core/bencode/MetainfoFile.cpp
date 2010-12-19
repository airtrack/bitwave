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
        return (*infodic_)["length"];
    }

    std::string MetainfoFile::Name() const
    {
        if (infodic_->find("name") == infodic_->end()) return std::string();

        BenString *name = dynamic_cast<BenString *>((*infodic_)["name"].get());
        if (!name) return std::string();

        return name->std_string();
    }

    int MetainfoFile::PieceLength() const
    {
        if (infodic_->find("piece length") == infodic_->end()) return 0;

        BenInteger *pl = dynamic_cast<BenInteger *>((*infodic_)["piece length"].get());
        if (!pl) return 0;

        return pl->GetValue();
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
        if (infodic_->find("length") == infodic_->end()) return 0;

        BenInteger *length = dynamic_cast<BenInteger *>((*infodic_)["length"].get());
        if (!length) return 0;

        return length->GetValue();
    }

    void MetainfoFile::Files(std::vector<FileInfo> *files) const
    {
        assert(files);
        if (infodic_->find("files") == infodic_->end()) return ;

        BenList *fs = dynamic_cast<BenList *>((*infodic_)["files"].get());
        if (!fs) return ;

        files->reserve(fs->size());

        std::vector<BenDictionary *> vd;
        fs->AllElementPtr(&vd);
        for (std::vector<BenDictionary *>::iterator it = vd.begin(); it != vd.end(); ++it)
        {
            BenDictionary *file = *it;
            if (file->find("length") == file->end()) continue;
            BenInteger *len = dynamic_cast<BenInteger *>((*file)["length"].get());
            if (!len) continue;

            FileInfo obj;
            obj.length = len->GetValue();

            if (file->find("path") == file->end()) continue;
            BenList *pathlist = dynamic_cast<BenList *>((*file)["path"].get());
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
        return std::make_pair(infodic_->GetSrcBufBegin(), infodic_->GetSrcBufEnd());
    }

    bool MetainfoFile::PrepareBasicData()
    {
        BenDictionary *dic = dynamic_cast<BenDictionary *>(metainfo_.get());
        if (!dic) return false;

        if (dic->find("announce") == dic->end()) return false;
        ann_ = dynamic_cast<BenString *>((*dic)["announce"].get());
        if (!ann_) return false;

        if (dic->find("announce-list") != dic->end())
            annlist_ = dynamic_cast<BenList *>((*dic)["announce-list"].get());

        if (dic->find("info") == dic->end()) return false;
        infodic_ = dynamic_cast<BenDictionary *>((*dic)["info"].get());
        if (!infodic_) return false;

        if (infodic_->find("pieces") == infodic_->end()) return false;
        pieces_ = dynamic_cast<BenString *>((*infodic_)["pieces"].get());
        if (!pieces_) return false;

        if (pieces_->length() % 20) return false;

        return true;
    }

} // namespace core
} // namespace bittorrent
