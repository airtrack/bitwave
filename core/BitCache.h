#ifndef BIT_CACHE_H
#define BIT_CACHE_H

#include "BitFile.h"
#include "BitPieceSha1Calc.h"
#include "../base/BaseTypes.h"
#include "../sha1/Sha1Value.h"
#include <functional>
#include <memory>
#include <map>

namespace bittorrent {
namespace core {

    namespace bentypes {
        class MetainfoFile;
    } // namespace bentypes

    class BitData;
    class BitPiece;
    class BitPieceMap;
    class BitDownloadDispatcher;

    class BitCache : private NotCopyable
    {
    public:
        // bool param is a flag of read success or not, const char * param
        // is the read data, the value is invalid when read success, otherwise
        // the value is 0
        typedef std::tr1::function<void (bool, const char *)> ReadCallback;

        explicit BitCache(const std::tr1::shared_ptr<BitData>& bitdata);

        bool IsInfoHashEqual(const Sha1Value& info_hash) const
        {
            return info_hash_ == info_hash;
        }

        void Read(std::size_t piece_index,
                  std::size_t begin_of_piece,
                  std::size_t length,
                  const ReadCallback& callback);

        void Write(std::size_t piece_index,
                   std::size_t begin_of_piece,
                   std::size_t length,
                   const char *block);

        void ProcessCache();

    private:
        struct AsyncReadData
        {
            ReadCallback callback;
            std::size_t begin_of_piece;
            std::size_t length;

            AsyncReadData(const ReadCallback& cb,
                          std::size_t bop,
                          std::size_t len)
                : callback(cb),
                  begin_of_piece(bop),
                  length(len)
            {
            }
        };

        typedef std::tr1::shared_ptr<BitPiece> PiecePtr;
        typedef std::map<std::size_t, PiecePtr> CachePiece;
        typedef std::multimap<std::size_t, AsyncReadData> AsyncReadOps;

        PiecePtr FetchNewPiece();

        CachePiece::iterator GetOldestPiece();

        CachePiece::iterator InsertNewPiece(std::size_t piece_index);

        void ReadBlock(std::size_t piece_index,
                       std::size_t begin_of_piece,
                       std::size_t length,
                       const ReadCallback& callback);

        void AsyncReadBlock(std::size_t piece_index,
                            std::size_t begin_of_piece,
                            std::size_t length,
                            const ReadCallback& callback);

        void ReadCacheBlock(CachePiece::iterator it,
                            std::size_t begin_of_piece,
                            std::size_t length,
                            const ReadCallback& callback);

        void ProcessAsyncReadOps();

        void CompleteAsyncReadOps(CachePiece::iterator it);

        void WriteBlock(std::size_t piece_index,
                        std::size_t begin_of_piece,
                        std::size_t length,
                        const char *block);

        void AsyncCheckPiece(CachePiece::iterator it);

        void ProcessAsyncCheckPiece();

        void CompleteAsyncCheckPiece(std::size_t piece_index,
                                     const Sha1Value& sha1);

        void AsyncWritePiece(CachePiece::iterator it);

        void ProcessAsyncWritePiece();

        void CompleteAsyncWritePiece(std::size_t piece_index);

        const std::size_t piece_length_;
        const BitPieceMap& piece_map_;
        const Sha1Value info_hash_;
        const bentypes::MetainfoFile *metainfo_file_;
        std::tr1::shared_ptr<BitDownloadDispatcher> download_dispatcher_;
        CachePiece cache_piece_;

        BitFile file_;
        AsyncReadOps async_read_ops_;
        BitPieceSha1Calc piece_sha1_calc_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_CACHE_H
