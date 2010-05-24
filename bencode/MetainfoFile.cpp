#include "MetainfoFile.h"
#include <stdio.h>
#include <assert.h>

namespace bittorrent
{
    using namespace bentypes;

    MetainfoFile::MetainfoFile(const char *filepath)
        : metainfo_(),
          annlist_(0),
          infodic_(0),
          pieces_(0)
    {
        FILE *file = fopen(filepath, "rb");
        if (!file)
        {
            std::string info = std::string("can not open file: ") + filepath;
            throw OpenFileException(info.c_str());
        }

        fseek(file, 0, SEEK_END);
        std::size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);
        BenTypesStreamBuf buf(file, size);
        fclose(file);

        metainfo_ = GetBenObject(buf);
        if (!metainfo_ || !CheckValid())
        {
            std::string info = std::string("invalid file: ") + filepath;
            throw OpenFileException(info.c_str());
        }
    }

    void MetainfoFile::GetAnnounce(std::vector<std::string> *announce) const
    {
        assert(announce);
        announce->reserve(annlist_->size());
        for (BenList::const_iterator it = annlist_->begin(); it != annlist_->end(); ++it)
        {
            BenString *ann = dynamic_cast<BenString *>(it->get());
            if (ann)
                announce->push_back(ann->std_string());
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
        for (BenList::iterator it = fs->begin(); it != fs->end(); ++it)
        {
            BenDictionary *file = dynamic_cast<BenDictionary *>(it->get());
            if (file)
            {
                if (file->find("length") == file->end()) continue;
                BenInteger *len = dynamic_cast<BenInteger *>((*file)["length"].get());
                if (!len) continue;

                FileInfo obj;
                obj.length = len->GetValue();

                if (file->find("path") == file->end()) continue;
                BenList *pathlist = dynamic_cast<BenList *>((*file)["path"].get());
                if (!pathlist) continue;

                obj.path.reserve(pathlist->size());
                for (BenList::iterator i = pathlist->begin(); i != pathlist->end(); ++i)
                {
                    BenString *path = dynamic_cast<BenString *>(i->get());
                    if (!path || !path->length()) continue;

                    obj.path.push_back(path->std_string());
                }

                files->push_back(obj);
            }
        }
    }

    bool MetainfoFile::CheckValid()
    {
        BenDictionary *dic = dynamic_cast<BenDictionary *>(metainfo_.get());
        if (!dic) return false;

        if (dic->find("announce") == dic->end()) return false;
        annlist_ = dynamic_cast<BenList *>((*dic)["announce"].get());
        if (!annlist_) return false;

        if (dic->find("info") == dic->end()) return false;
        infodic_ = dynamic_cast<BenDictionary *>((*dic)["info"].get());
        if (!infodic_) return false;

        if (infodic_->find("pieces") == infodic_->end()) return false;
        pieces_ = dynamic_cast<BenString *>((*infodic_)["pieces"].get());
        if (!pieces_) return false;

        if (pieces_->length() % 20) return false;

        return true;
    }
} // namespace bittorrent