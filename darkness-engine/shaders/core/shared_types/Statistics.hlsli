#ifdef __cplusplus
#pragma once
#endif

struct FrameStatistics
{
    // phase 1 (prev frame)
    uint clusterFrustumCull_ClustersIn;
    uint clusterFrustumCull_ClustersOutGBuffer;
    uint clusterFrustumCull_ClustersOutAlphaClipped;
	uint clusterFrustumCull_ClustersOutTransparent;
	uint clusterFrustumCull_ClustersOutTerrain;
    
	uint expandGBufferIndexes_IndexesOut;
    uint drawDepth_Triangles;
    uint drawDepth_Count;
    
	uint expandAlphaClipIndexes_IndexesOut;
    uint drawAlphaClip_Triangles;
    uint drawAlphaClip_Count;

    uint expandTransparentIndexes_IndexesOut;
    uint drawTransparent_Triangles;
    uint drawTransparent_Count;

	uint expandTerrainIndexes_IndexesOut;
	uint drawTerrain_Triangles;
	uint drawTerrain_Count;				// 16

    // phase 2 (new frame)
    uint instanceFrustum_InstancesIn;
    uint instanceFrustum_InstancesPassed;
    uint clusterExpansion_ClustersOut;

    uint occlusionCulling_out_All;		// 20
    uint occlusionCulling_out_NotYetDrawnDepth;
    uint occlusionCulling_out_AlphaClipped;
    uint occlusionCulling_out_Transparent;
	uint occlusionCulling_out_Terrain;

    uint expandIndexes_NotYetDrawnDepthIndexesOut;	// 25
    uint drawDepth_Triangles2;
    uint drawDepth_Count2;

    uint expandIndexes_AlphaClippedIndexesOut;
    uint drawDepth_AlphaClippedTriangles;
    uint drawDepth_AlphaClippedCount;

    uint expandIndexes_TransparentIndexesOut;	// 31
    uint drawDepth_TransparentTriangles;
    uint drawDepth_TransparentCount;

	uint expandIndexes_TerrainIndexesOut;
	uint drawDepth_TerrainTriangles;
	uint drawDepth_TerrainCount;			// 36

    uint4 padding1;
    uint4 padding2;
    uint4 padding3;
    uint2 padding4;
};
