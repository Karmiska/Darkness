#include "Simplygon.h"
#include "SimplygonSDK.h"
#include "platform/Environment.h"
#include "tools/PathTools.h"

#include <fstream>
#include <algorithm>

#include <execution>
#include <functional>

#ifdef _WIN32

#include <SimplygonSDKLoader.h>
#include <windows.h>
#include <process.h>

#elif defined(__linux__) || defined(__APPLE__)

#include <SimplygonSDKNixLoader.h>
#include <pthread.h>
#include <unistd.h>

#ifndef MAX_PATH
#if defined(__linux__)
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <sys/syslimits.h>
#endif
#define MAX_PATH PATH_MAX
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#endif

SimplygonSDK::ISimplygonSDK *sg = NULL;

namespace resource_task
{
    Simplygon::Simplygon()
    {
#ifdef _WIN32
        auto exeFilePath = engine::getExecutableDirectory();
        auto exePath = engine::pathClean(engine::pathExtractFolder(exeFilePath));
        auto libPath = engine::pathJoin(exePath, "SimplygonSDKRuntimeReleasex64.dll");
        auto license = getLicenseText();


        // if this fails with -10005, it means that it's a licensing issue.
        // it does not necessarily mean that you have a bad license how ever.
        // possible fixes:
        //   - update the SDKAPI files in darkness-externals/SimplygonAPI, 
        //     either do a clean build or 
        //     override the two dlls (SimplygonSDKCLIReleasex64.dll, SimplygonSDKRuntimeReleasex64.dll) in darkness-resourcetask\bin\win64\debug or release.
        //   - if that doesn't help, it might be that for some reason the Simplygon license in use has been used on another machine by
        //     accident and is now under 48h lock down. wait it out or generate a new license on a different user.

        // if this fails with -10024, it means that you've fucked up your manifest file.
        // it is under darkness-resourcetask\data\Compatibility.manifest
        // and should be included in the Simplygon SharpMake project configuration
        int initval = SimplygonSDK::Initialize(&sg);// , libPath.c_str(), license.c_str());

#elif defined(__linux__)
        std::ifstream t("LicensePathHere");
        engine::string licenceStr((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        int initval = SimplygonSDK::Initialize(&sg, "./SimplygonSDKNixRuntime.so", licenceStr.c_str());
#elif defined(__APPLE__)
        std::ifstream t("LicensePathHere");
        engine::string licenceStr((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        int initval = SimplygonSDK::Initialize(&sg, "./SimplygonSDKNixRuntime.dylib", licenceStr.c_str());
#endif
    }

    engine::string Simplygon::getLicenseText() const
    {
        auto exeFilePath = engine::getExecutableDirectory();
        auto exePath = engine::pathClean(engine::pathExtractFolder(exeFilePath));
        auto licensePath = engine::pathClean(engine::pathJoin(exePath, "..\\..\\..\\data\\Simplygon_8_license.dat"));

        engine::string licenseText = "";

        std::ifstream licenseFile;
        licenseFile.open(licensePath.c_str(), std::ios::in);
        if (licenseFile.is_open())
        {
            std::string line;
            while (std::getline(licenseFile, line))
            {
                licenseText += line.c_str();
            }
            licenseFile.close();
        }
        return licenseText;
    }

    Simplygon::~Simplygon()
    {
        SimplygonSDK::Deinitialize();
    }

    SimplygonSDK::spPackedGeometryData meshFromModel(engine::ModelCpu& model)
    {
        SimplygonSDK::spPackedGeometryData geom = sg->CreatePackedGeometryData();
        
        //geom->AddMaterialIds();
        geom->AddNormals();
        geom->AddTangents(0);

        // Array with vertex-coordinates. Will contain 3 real-values for each vertex in the geometry.
        SimplygonSDK::spRealArray vertex = geom->GetCoords();
        SimplygonSDK::spRealArray normal = geom->GetNormals();
        SimplygonSDK::spRealArray tangent = geom->GetTangents(0);
        SimplygonSDK::spRealArray bitangent = geom->GetBitangents(0);
        SimplygonSDK::spRidArray index = geom->GetVertexIds();

        // Array with triangle-data. Will contain 3 ids for each corner of each triangle, so the triangles know what vertices to use.
        auto vertex_count = model.vertex.size();
        auto triangle_count = model.index.size() / 3;
        auto corner_count = model.index.size();

        // Set vertex- and triangle-counts for the Geometry. 
        // NOTE: The number of vertices and triangles has to be set before vertex- and triangle-data is loaded into the GeometryData.
        geom->SetVertexCount(static_cast<unsigned int>(vertex_count));
        geom->SetTriangleCount(static_cast<unsigned int>(triangle_count));

        normal->SetTupleCount(static_cast<unsigned int>(vertex_count));
        tangent->SetTupleCount(static_cast<unsigned int>(vertex_count));
        bitangent->SetTupleCount(static_cast<unsigned int>(vertex_count));
        
        index->SetTupleCount(static_cast<unsigned int>(corner_count));

        // add vertex-coordinates to the Geometry. Each tuple contains the 3 coordinates for each vertex. x, y and z values.
        for (int i = 0; i < vertex_count; ++i)
        {
            vertex->SetTuple(i, &model.vertex[i]);
            normal->SetTuple(i, &model.normal[i]);
            tangent->SetTuple(i, &model.tangent[i]);
        }

        // Must add texture channel before adding data to it. 
        for (int i = 0; i < model.uv.size(); ++i)
        {
            geom->AddTexCoords(i);
            SimplygonSDK::spRealArray texcoords = geom->GetTexCoords(i);
            texcoords->SetTupleCount(static_cast<unsigned int>(vertex_count));
            auto tupleSize = texcoords->GetTupleSize();
            auto tupleIndex = 0;
            for (int v = 0; v < vertex_count; ++v)
            {
                texcoords->SetItem(tupleIndex + 0, model.uv[i][v].x);
                texcoords->SetItem(tupleIndex + 1, model.uv[i][v].y);
                tupleIndex += tupleSize;
            }
        }

        for (int i = 0; i < model.color.size(); ++i)
        {
            geom->AddColors(i);
            SimplygonSDK::spRealArray colorarr = geom->GetColors(i);
            colorarr->SetTupleCount(static_cast<unsigned int>(model.color[i].size()));
            auto tupleSize = colorarr->GetTupleSize();
            auto tupleIndex = 0;
            auto tupleIncrement = tupleSize;
            for (int a = 0; a < model.color[i].size(); ++a)
            {
                colorarr->SetItem(tupleIndex + 0, model.color[i][a].x);
                colorarr->SetItem(tupleIndex + 1, model.color[i][a].y);
                colorarr->SetItem(tupleIndex + 2, model.color[i][a].z);
                colorarr->SetItem(tupleIndex + 3, model.color[i][a].w);
                tupleIndex += tupleIncrement;
            }
        }

        // Add triangles to the Geometry. Each triangle-corner contains the id for the vertex that corner uses.
        // SetTuple can also be used, but since the TupleSize of the vertex_ids array is 1, it would make no difference.
        // (This since the vertex_ids array is corner-based, not triangle-based.)
        for (int i = 0; i < corner_count; ++i)
        {
            index->SetItem(i, model.index[i]);
        }

        return geom;
    }

    engine::ModelCpu modelFromScene(SimplygonSDK::spScene scene, uint32_t colorCount, uint32_t uvCount, engine::BoundingBox boundingBox)
    {
        engine::ModelCpu currentLod;

        SimplygonSDK::spSceneMesh topmesh = SimplygonSDK::Cast<SimplygonSDK::ISceneMesh>(scene->GetRootNode()->GetChild(0));
        SimplygonSDK::spGeometryData leftHandedGeom = topmesh->GetGeometry()->NewCopy(true);
        //leftHandedGeom->ConvertHandedness();
        SimplygonSDK::spPackedGeometryData packed_geom = leftHandedGeom->NewPackedCopy();
        //SimplygonSDK::spGeometryData packed_geom = topmesh->GetGeometry()->NewCopy(true);

        // fetch various data to be able to set up a new dxmesh
        const DWORD triCount = packed_geom->GetTriangleCount();
        const DWORD vertCount = packed_geom->GetVertexCount();

        uint32_t NumberOfVertices = packed_geom->GetVertexCount();
        uint32_t NumberOfFaces = packed_geom->GetTriangleCount();

        SimplygonSDK::spRidArray VertexIDs = packed_geom->GetVertexIds();
        SimplygonSDK::spRealArray Coords = packed_geom->GetCoords();
        SimplygonSDK::spRealArray Normals = packed_geom->GetNormals();

        SimplygonSDK::spRealArray BiTangents;
        SimplygonSDK::spRealArray Tangents;

        if (packed_geom->GetTangents(0))
            Tangents = packed_geom->GetTangents(0);

        if (packed_geom->GetBitangents(0))
            BiTangents = packed_geom->GetBitangents(0);


        SimplygonSDK::spRealData vertices_xyz = sg->CreateRealData();
        SimplygonSDK::spRealData normals_xyz = sg->CreateRealData();
        SimplygonSDK::spRealData tangents_xyz = sg->CreateRealData();
        SimplygonSDK::spRealData uv_xy = sg->CreateRealData();

        for (uint32_t i = 0; i < NumberOfVertices; ++i)
        {
            Coords->GetTuple(i, vertices_xyz);
            Normals->GetTuple(i, normals_xyz);
            Tangents->GetTuple(i, tangents_xyz);

            currentLod.vertex.emplace_back(engine::Vector3f{ vertices_xyz[0], vertices_xyz[1], vertices_xyz[2] });
            currentLod.normal.emplace_back(engine::Vector3f{ normals_xyz[0], normals_xyz[1], normals_xyz[2] });
            currentLod.tangent.emplace_back(engine::Vector3f{ tangents_xyz[0], tangents_xyz[1], tangents_xyz[2] });
        }

        for (uint32_t i = 0; i < colorCount; ++i)
        {
            SimplygonSDK::spRealArray colors = packed_geom->GetColors(i);
            auto count = colors->GetTupleCount();
            auto tupleSize = colors->GetTupleSize();
            auto tupleIndex = 0;
            auto tupleIncrement = tupleSize;

            currentLod.color.resize(currentLod.color.size() + 1);
            currentLod.color.back().resize(count);
            for (int a = 0; a < count; ++a)
            {
                engine::Vector4f color;
                color.x = colors->GetItem(tupleIndex + 0);
                color.y = colors->GetItem(tupleIndex + 1);
                color.z = colors->GetItem(tupleIndex + 2);
                color.w = colors->GetItem(tupleIndex + 3);
                currentLod.color[i][a] = color;
                tupleIndex += tupleIncrement;
            }
        }

        for (int i = 0; i < uvCount; ++i)
        {
            SimplygonSDK::spRealArray texcoords = packed_geom->GetTexCoords(i);
            currentLod.uv.emplace_back(engine::vector<engine::Vector2f>{});
            engine::vector<engine::Vector2f>& uvtrg = currentLod.uv.back();

            auto count = texcoords->GetTupleCount();
            auto tupleSize = texcoords->GetTupleSize();
            auto tupleIndex = 0;
            uvtrg.resize(count);
            for (uint32_t v = 0; v < count; ++v)
            {
                engine::Vector2f value;
                value.x = texcoords->GetItem(tupleIndex + 0);
                value.y = texcoords->GetItem(tupleIndex + 1);
                tupleIndex += tupleSize;
                uvtrg[v] = value;
            }
        }

        for (uint32_t i = 0; i < NumberOfFaces * 3; ++i)
        {
            currentLod.index.emplace_back(VertexIDs->GetItem(i));
        }

        currentLod.boundingBox = boundingBox;

        return currentLod;
    }

    float distance(engine::Vector2f screenSize, float pixelSize, float fov, float radius)
    {
        // The pixel size of the geometry compared to the number
        // of pixels used on the screen.
        auto screen_ratio = pixelSize / screenSize.y;
        // Normalized distance to the 'screen' if the height of
        // the screen is 1.
        float screen_distance = 1.0 / (tan(DEG_TO_RAD * (fov / 2.0f)));
        // The view-angle of the bounding sphere rendered on screen.
        float bsphere_angle = atan(screen_ratio / screen_distance);
        // The distance (along the view vector) to the center of
        // the bounding sphere.
        return radius / sin(bsphere_angle);
    }

    float pixelSize(engine::Vector2f screenSize, float distance, float fov, float radius)
    {
        // The view-angle of the bounding sphere rendered on screen.
        float bsphere_angle = asin(radius / distance);
        // This assumes the near clipping plane is at a
        // distance of 1.
        float geom_view_height = tan(bsphere_angle);
        // The size of (half) the screen if the near clipping
        // plane is at a distance of 1.
        float screen_view_height = tan(DEG_TO_RAD * (fov / 2.0f));
        // The ratio of the geometry's screen size compared to
        // the actual size of the screen.
        float view_ratio = geom_view_height / screen_view_height;
        // Multiply by the number of pixels on the screen.
        return view_ratio * screenSize.y;
    }

    Simplygon::PackedGeometry Simplygon::generateLod(
        uint32_t lodCount,
        engine::ModelCpu& model)
    {
        //Create a scene and a SceneMesh node with the geometry
        SimplygonSDK::spScene scene = sg->CreateScene();
        auto mm = meshFromModel(model);
        SimplygonSDK::spGeometryData lodCopy = mm->NewUnpackedCopy();
        scene->GetRootNode()->CreateChildMesh(lodCopy);

        

        // Add progress observer
        //reductionProcessor->AddObserver(&progressObserver, SG_EVENT_PROGRESS);
        PackedGeometry output;

        auto halfSize = model.boundingBox.halfSize();
        auto screenSize = engine::Vector2f{ 1920.0f, 1080.0f };

        // size based list
        auto closestSize = 600.0f;
        auto furthestSize = 5.0f;
        auto fov = 60.0f;

        auto range = closestSize - furthestSize;
        auto piece = range / (lodCount - 1);
        
        auto closestDistance = distance(screenSize, closestSize, fov, halfSize);
        auto furthestDistance = distance(screenSize, furthestSize, fov, halfSize);
        auto distRange = furthestDistance - closestDistance;
        auto distPiece = distRange / (lodCount - 1);

        auto lerp = [](float a, float b, float phase)->float
        {
            return a + ((b - a) * phase);
        };

        engine::vector<uint32_t> screenSizeTargets;
        engine::vector<float> screenDistanceTargets;
        for (int i = 0; i < lodCount; ++i)
        {
            float lodSize = closestSize - (static_cast<float>(i) * piece);
            float lodDistance = closestDistance + (static_cast<float>(i) * distPiece);
            float distanceSize = pixelSize(screenSize, lodDistance, fov, halfSize);
            screenSizeTargets.emplace_back(lerp(lodSize, distanceSize, 0.85f));
            screenDistanceTargets.emplace_back(distance(screenSize, screenSizeTargets.back(), fov, halfSize));
        }

        engine::vector<int> subMeshList(lodCount);
        for (int lodLevel = 0; lodLevel < lodCount; ++lodLevel)
        {
            subMeshList[lodLevel] = lodLevel;
        }

        output.lods.resize(lodCount);
        std::for_each(
            std::execution::par_unseq,
            subMeshList.begin(),
            subMeshList.end(),
            [&](int lodLevel)
        {
            SimplygonSDK::spScene lodScene = scene->NewCopy();

            SimplygonSDK::spReductionProcessor reductionProcessor = sg->CreateReductionProcessor();
            reductionProcessor->SetScene(lodScene);
            SimplygonSDK::spReductionSettings reductionSettings = reductionProcessor->GetReductionSettings();
            // set reduction settings
            {
#if 0
                reductionSettings->SetReductionHeuristics(SimplygonSDK::SG_REDUCTIONHEURISTICS_CONSISTENT); //Choose between "fast" and "consistent" processing.

                // The normal calculation settings deal with the normal-specific reduction settings
                SimplygonSDK::spNormalCalculationSettings normalSettings = reductionProcessor->GetNormalCalculationSettings();
                normalSettings->SetReplaceNormals(false); //If true, this will turn off normal handling in the reducer and recalculate them all afterwards instead.
                //normalSettings->SetHardEdgeAngleInRadians( 3.14159f*70.0f/180.0f ); //If the normals are recalculated, this sets the hard-edge angle.

                // The actual reduction triangle target are controlled by these settings
                reductionSettings->SetStopCondition(SimplygonSDK::SG_STOPCONDITION_ANY); //The reduction stops when either of the targets is reached
                reductionSettings->SetReductionTargets(SimplygonSDK::SG_REDUCTIONTARGET_ONSCREENSIZE);//The max deviation target determines when to stop the reduction. It is set in the loop below.

#else
                reductionSettings->SetKeepSymmetry(true); //Try, when possible to reduce symmetrically
                reductionSettings->SetUseAutomaticSymmetryDetection(true); //Auto-detect the symmetry plane, if one exists. Can, if required, be set manually instead.
                reductionSettings->SetUseHighQualityNormalCalculation(true); //Drastically increases the quality of the LODs normals, at the cost of extra processing time.
                reductionSettings->SetReductionHeuristics(SimplygonSDK::SG_REDUCTIONHEURISTICS_CONSISTENT); //Choose between "fast" and "consistent" processing. Fast will look as good, but may cause inconsistent 

                // The actual reduction triangle target are controlled by these settings
                reductionSettings->SetStopCondition(SimplygonSDK::SG_STOPCONDITION_ANY);//The reduction stops when any of the targets below is reached
                reductionSettings->SetReductionTargets(SimplygonSDK::SG_REDUCTIONTARGET_ALL);//Selects which targets should be considered when reducing
                reductionSettings->SetTriangleRatio(0.5); //Targets at 50% of the original triangle count
                reductionSettings->SetTriangleCount(10); //Targets when only 10 triangle remains
                reductionSettings->SetMaxDeviation(SimplygonSDK::REAL_MAX); //Targets when an error of the specified size has been reached. As set here it never happens.
                reductionSettings->SetOnScreenSize(50); //Targets when the LOD is optimized for the selected on screen pixel size

                // The repair settings object contains settings to fix the geometries
                SimplygonSDK::spRepairSettings repairSettings = reductionProcessor->GetRepairSettings();
                repairSettings->SetTjuncDist(0.0f); //Removes t-junctions with distance 0.0f
                repairSettings->SetWeldDist(0.0f); //Welds overlapping vertices

                // The normal calculation settings deal with the normal-specific reduction settings
                SimplygonSDK::spNormalCalculationSettings normalSettings = reductionProcessor->GetNormalCalculationSettings();
                normalSettings->SetReplaceNormals(false); //If true, this will turn off normal handling in the reducer and recalculate them all afterwards instead.
#endif
            }

            // Run the actual processing. After this, the set geometry will have been reduced according to the settings
            reductionSettings->SetOnScreenSize(screenSizeTargets[lodLevel]);
            reductionProcessor->RunProcessing();

            output.lods[lodLevel] = modelFromScene(lodScene, model.color.size(), model.uv.size(), model.boundingBox);
        });
        return output;
    }
}
