#include "BitDownloadingInfo.h"
#include "BitData.h"
#include <algorithm>

namespace bitwave {
namespace core {

    BitDownloadingInfo::BitDownloadingInfo(
            const std::tr1::shared_ptr<BitData>& bitdata)
        : downloaded_(bitdata->GetPieceMap()),
          need_download_(bitdata->GetPieceCount()),
          downloading_(bitdata->GetPieceCount())
    {
        UpdateNeedDownload(bitdata);
    }

    void BitDownloadingInfo::MarkDownloading(std::size_t piece_index)
    {
        downloading_.MarkPiece(piece_index);
        std::for_each(observers_.begin(), observers_.end(),
                std::tr1::bind(&Observer::DownloadingNewPiece,
                    std::tr1::placeholders::_1, piece_index));
    }

    void BitDownloadingInfo::MarkDownloadComplete(std::size_t piece_index)
    {
        downloading_.UnMarkPiece(piece_index);
        downloaded_.MarkPiece(piece_index);
        std::for_each(observers_.begin(), observers_.end(),
                std::tr1::bind(&Observer::CompleteNewPiece,
                    std::tr1::placeholders::_1, piece_index));
    }

    void BitDownloadingInfo::DownloadingFailed(std::size_t piece_index)
    {
        downloading_.UnMarkPiece(piece_index);
        std::for_each(observers_.begin(), observers_.end(),
                std::tr1::bind(&Observer::DownloadingFailed,
                    std::tr1::placeholders::_1, piece_index));
    }

    void BitDownloadingInfo::UpdateNeedDownload(
            const std::tr1::shared_ptr<BitData>& bitdata)
    {
        const BitData::DownloadFiles& files = bitdata->GetFilesInfo();
        BitData::DownloadFiles::const_iterator it = files.begin();
        BitData::DownloadFiles::const_iterator end = files.end();

        long long file_begin = 0;
        std::size_t piece_length = bitdata->GetPieceLength();
        for (; it != end; ++it)
        {
            long long file_end = file_begin + it->length;
            if (it->is_download)
            {
                std::size_t piece_index = static_cast<std::size_t>(
                        file_begin / piece_length);
                std::size_t end_piece_index = static_cast<std::size_t>(
                        ceil(file_end / static_cast<double>(piece_length)));

                for (; piece_index < end_piece_index; ++piece_index)
                    need_download_.MarkPiece(piece_index);
            }
            file_begin = file_end;
        }
    }

} // namespace core
} // namespace bitwave
