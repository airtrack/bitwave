#include "../core/bencode/MetainfoFile.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <iostream>

using namespace bittorrent;
using namespace bittorrent::core;

struct OutPutString : public std::unary_function<std::string, void>
{
    OutPutString(std::ostream& os) : os_(os) { }

    void operator () (const std::string& str) const
    {
        os_ << str << '\n';
    }

    std::ostream& os_;
};

struct OutPutFileInfo : public std::unary_function<MetainfoFile::FileInfo, void>
{
    OutPutFileInfo(std::ostream& os) : os_(os) { }

    void operator () (const MetainfoFile::FileInfo& info) const
    {
        for (std::vector<std::string>::const_iterator
            it = info.path.begin(); it != info.path.end(); ++it)
        {
            os_ << '\\' << *it;
        }
        os_ << '\t' << info.length << '\n';
    }

    std::ostream& os_;
};

int main()
{
    try
    {
        std::string filename;
        std::cin >> filename;

        MetainfoFile torrentfile(filename.c_str());
        std::ofstream infofile(std::string(filename + "_result.txt").c_str());

        infofile << "is single file: ";
        if (torrentfile.IsSingleFile())
            infofile << "true\n";
        else
            infofile << "false\n";

        infofile << "name: " << torrentfile.Name() << '\n';
        infofile << "piece length: " << torrentfile.PieceLength() << '\n';
        infofile << "piece count: " << torrentfile.PiecesCount() << '\n';
        infofile << "length: " << torrentfile.Length() << '\n';
        infofile << '\n';

        std::vector<std::string> announce;
        torrentfile.GetAnnounce(&announce);
        infofile << "announce:\n";
        std::for_each(announce.begin(), announce.end(), OutPutString(infofile));

        std::vector<MetainfoFile::FileInfo> files;
        torrentfile.Files(&files);
        infofile << "\nfiles: \n";
        std::for_each(files.begin(), files.end(), OutPutFileInfo(infofile));
    }
    catch (const BaseException& be)
    {
        std::cout << be.what() << std::endl;
    }

    return 0;
}
