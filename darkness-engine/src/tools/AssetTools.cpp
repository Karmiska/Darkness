#include "tools/AssetTools.h"
#include "tools/PathTools.h"
#include <cctype>

using namespace engine;

namespace engine
{
    AssetNotificationService GlobalAssetNotificationService;

    bool isModelFormat(const string& filepath)
    {
        string fileExtension = pathExtractExtension(filepath);
        std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (fileExtension == "fbx") return true;
        if (fileExtension == "bvh") return true;
        if (fileExtension == "dae") return true;
        if (fileExtension == "gltf" || fileExtension == "glb") return true;
        if (fileExtension == "blend") return true;
        if (fileExtension == "3ds") return true;
        if (fileExtension == "ase") return true;
        if (fileExtension == "obj") return true;
        if (fileExtension == "ifc") return true;
        if (fileExtension == "xgl" || fileExtension == "zgl") return true;
        if (fileExtension == "ply") return true;
        if (fileExtension == "dxf") return true;
        if (fileExtension == "lwo") return true;
        if (fileExtension == "lws") return true;
        if (fileExtension == "lxo") return true;
        if (fileExtension == "stl") return true;
        if (fileExtension == "x") return true;
        if (fileExtension == "ac") return true;
        if (fileExtension == "ms3d") return true;
        if (fileExtension == "cob" || fileExtension == "scn") return true;
        return false;
    }

    bool isImageFormat(const string& filepath)
    {
        string fileExtension = pathExtractExtension(filepath);
        std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (fileExtension == "bmp") return true;
        if (fileExtension == "cut") return true;
        if (fileExtension == "dds") return true;
        if (fileExtension == "exr") return true;
        if (fileExtension == "raw") return true;
        if (fileExtension == "gif") return true;
        if (fileExtension == "hdr") return true;
        if (fileExtension == "ico") return true;
        if (fileExtension == "iff") return true;
        if (fileExtension == "jbig") return true;
        if (fileExtension == "jng") return true;
        if (fileExtension == "jpg") return true;
        if (fileExtension == "mng") return true;
        if (fileExtension == "pcx") return true;
        if (fileExtension == "pbm") return true;
        if (fileExtension == "pgm") return true;
        if (fileExtension == "ppm") return true;
        if (fileExtension == "pfm") return true;
        if (fileExtension == "png") return true;
        if (fileExtension == "pict") return true;
        if (fileExtension == "psd") return true;
        if (fileExtension == "raw") return true;
        if (fileExtension == "ras") return true;
        if (fileExtension == "sgi") return true;
        if (fileExtension == "targa") return true;
        if (fileExtension == "tiff") return true;
        if (fileExtension == "tif") return true;
        if (fileExtension == "tga") return true;
        if (fileExtension == "wbmp") return true;
        if (fileExtension == "webp") return true;
        if (fileExtension == "xbm") return true;
        if (fileExtension == "xpm") return true;
        return false;
    }

    bool isPrefabFormat(const string& filepath)
    {
        return pathExtractExtension(filepath) == "prefab";
    }

    void AssetNotificationService::registerListener(void* client, std::function<void(const AssetNotification&)> notification)
    {
        m_clients[client] = notification;
    }

    void AssetNotificationService::unregisterListener(void* client)
    {
        m_clients.erase(client);
    }

    void AssetNotificationService::sendNotification(const AssetNotification& notification)
    {
        for (auto&& client : m_clients)
        {
            client.second(notification);
        }
    }

}
