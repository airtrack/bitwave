#include "BitWave.h"
#include "BitData.h"
#include "BitService.h"
#include "BitCreator.h"
#include "BitController.h"
#include "BitRepository.h"
#include "BitPeerListener.h"
#include "../base/Console.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

namespace bitwave {
namespace core {

    BitNetWaveObject::BitNetWaveObject()
        : sock_initer_(),
          io_service_(),
          timer_service_(),
          resolve_service_()
    {
        io_service_.AddService(&timer_service_);
        io_service_.AddService(&resolve_service_);
        BitService::io_service = &io_service_;
    }

    BitNetWaveObject::~BitNetWaveObject()
    {
        BitService::io_service = 0;
    }

    bool BitNetWaveObject::Wave()
    {
        io_service_.Run();
        return true;
    }

    BitCoreControlObject::BitCoreControlObject()
    {
        repository_.Reset(new BitRepository);
        controller_.Reset(new BitController);

        assert(BitService::io_service);
        new_task_creator_.Reset(new BitNewTaskCreator(
                    *controller_, *BitService::io_service));

        BitService::controller = controller_.Get();
        BitService::repository = repository_.Get();
        BitService::new_task_creator = new_task_creator_.Get();

        peer_listener_.Reset(new BitPeerListener(*BitService::io_service));
    }

    BitCoreControlObject::~BitCoreControlObject()
    {
        BitService::controller = 0;
        BitService::repository = 0;
        BitService::new_task_creator = 0;
    }

    bool BitCoreControlObject::Wave()
    {
        controller_->Process();
        return true;
    }

    BitConsoleShowerObject::BitConsoleShowerObject()
        : console_(new Console)
    {
        console_->GetCursorPos(cursor_x_, cursor_y_);
        console_->SetCursorVisible(false);
        last_show_time_ = TimeTraits::now();
    }

    BitConsoleShowerObject::~BitConsoleShowerObject()
    {
        console_->SetCursorVisible(true);
    }

    bool BitConsoleShowerObject::Wave()
    {
        NormalTimeType now_time = TimeTraits::now();
        if (now_time - last_show_time_ >= 1000)
        {
            ShowInfo(now_time);
            last_show_time_ = now_time;
        }

        return true;
    }

    void BitConsoleShowerObject::ShowInfo(const NormalTimeType& now_time)
    {
        std::vector<std::tr1::shared_ptr<BitData>> all_bitdata;
        BitService::repository->GetAllBitData(all_bitdata);

        std::size_t size = all_bitdata.size();
        if (download_bytes_.size() != size)
            download_bytes_.resize(size);
        if (upload_bytes_.size() != size)
            upload_bytes_.resize(size);

        for (std::size_t i = 0; i < size; ++i)
        {
            std::tr1::shared_ptr<BitData> bitdata = all_bitdata[i];
            NormalTimeType interval = now_time - last_show_time_;
            double download_speed = GetDownloadSpeed(bitdata, i, interval);
            double upload_speed = GetUploadSpeed(bitdata, i, interval);
            double percent = GetDownloadPercent(bitdata);
            int peer_count = bitdata->GetPeerDataSet().size();

            wchar_t str[256] = { 0 };
            swprintf(str, sizeof(str), L"download:%6.2fKB/S upload:%6.2fKB/S peer count:%4d downloaded:%6.2f%%",
                    download_speed, upload_speed, peer_count, percent);

            console_->SetCursorPos(cursor_x_, cursor_y_ + i);
            console_->Write(str, wcslen(str));
        }
    }

    double BitConsoleShowerObject::GetDownloadSpeed(
            const std::tr1::shared_ptr<BitData>& bitdata,
            std::size_t task_index,
            const NormalTimeType& interval)
    {
        long long current_download_bytes = bitdata->GetCurrentDownload();
        long long new_download_bytes = current_download_bytes - download_bytes_[task_index];
        download_bytes_[task_index] = current_download_bytes;
        double speed = static_cast<double>(new_download_bytes) * 1000 / interval;
        speed /= 1024;
        return speed;
    }

    double BitConsoleShowerObject::GetUploadSpeed(
            const std::tr1::shared_ptr<BitData>& bitdata,
            std::size_t task_index,
            const NormalTimeType& interval)
    {
        long long current_upload_bytes = bitdata->GetUploaded();
        long long new_upload_bytes = current_upload_bytes - upload_bytes_[task_index];
        upload_bytes_[task_index] = current_upload_bytes;
        double speed = static_cast<double>(new_upload_bytes) * 1000 / interval;
        speed /= 1024;
        return speed;
    }

    double BitConsoleShowerObject::GetDownloadPercent(
            const std::tr1::shared_ptr<BitData>& bitdata)
    {
        long long downloaded = bitdata->GetDownloaded();
        long long total_size = bitdata->GetTotalSize();
        double percent = static_cast<double>(downloaded) * 100 / total_size;
        percent = percent > 100.00 ? 100.00 : percent;
        return percent;
    }

    void BitWave::AddWaveObject(BitWaveObject *object)
    {
        assert(object);
        wave_objects_.push_back(object);
    }

    void BitWave::RemoveWaveObject(BitWaveObject *object)
    {
        assert(object);
        WaveObjects::iterator it = std::find(
                wave_objects_.begin(), wave_objects_.end(), object);
        if (it != wave_objects_.end())
            wave_objects_.erase(it);
    }

    void BitWave::Wave()
    {
        while (true)
        {
            for (WaveObjects::iterator it = wave_objects_.begin();
                    it != wave_objects_.end(); ++it)
            {
                if (!(*it)->Wave())
                    return ;
            }

            if (BitService::continue_run)
                BitService::continue_run = false;
            else
                ::Sleep(25);
        }
    }

} // namespace core
} // namespace bitwave
