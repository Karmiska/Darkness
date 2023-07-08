#include "ShaderPathTools.h"
#include "tools/PathTools.h"

namespace shadercompiler
{
    engine::string className(const engine::string& path)
    {
        auto basename = engine::pathExtractFilename(path);
        auto rootname = engine::pathExtractFilenameWithoutExtension(basename);
        auto ext = engine::pathExtractExtension(basename);
        auto sroot = engine::pathExtractFilenameWithoutExtension(rootname);
        auto sext = engine::pathExtractExtension(rootname);
        std::transform(sext.begin(), sext.end(), sext.begin(), ::toupper);
        return sroot + sext.c_str();
    }

    engine::string stageName(const engine::string& path)
    {
		if (path.substr(path.length() - 7, 7) == "cs.hlsl")
			return "Compute";
		if (path.substr(path.length() - 7, 7) == "vs.hlsl")
			return "Vertex";
		if (path.substr(path.length() - 7, 7) == "ps.hlsl")
			return "Pixel";
		if (path.substr(path.length() - 7, 7) == "gs.hlsl")
			return "Geometry";
		if (path.substr(path.length() - 7, 7) == "hs.hlsl")
			return "Hull";
		if (path.substr(path.length() - 7, 7) == "ds.hlsl")
			return "Domain";
		if (path.substr(path.length() - 7, 7) == "rg.hlsl")
			return "Raygeneration";
		if (path.substr(path.length() - 7, 7) == "is.hlsl")
			return "Intersection";
		if (path.substr(path.length() - 7, 7) == "ms.hlsl")
			return "Miss";
		if (path.substr(path.length() - 7, 7) == "ah.hlsl")
			return "AnyHit";
		if (path.substr(path.length() - 7, 7) == "ch.hlsl")
			return "ClosestHit";
		if (path.substr(path.length() - 8, 8) == "amp.hlsl")
			return "Amplification";
		if (path.substr(path.length() - 9, 9) == "mesh.hlsl")
			return "Mesh";
		return "Unknown";
    }

    engine::string interfacePath(const engine::string& path)
    {
		auto filename = engine::pathExtractFilenameWithoutExtension(path);
		return filename + ".h";
    }

    engine::string pipelineNameFromFilename(const engine::string& path)
    {
		auto basename = engine::pathExtractFilename(path);
		auto rootname = engine::pathExtractFilenameWithoutExtension(basename);
		auto sroot = engine::pathExtractFilenameWithoutExtension(rootname);
		return sroot;
    }
}
