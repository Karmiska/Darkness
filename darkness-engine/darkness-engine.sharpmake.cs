using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../common.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-shared/darkness-shared.sharpmake.cs")]

[module: Sharpmake.Include("../darkness-externals/bzip2.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/bullet3.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/gainput.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/imgui.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/libzmq.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/protobuf.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/rapidjson.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/zstd.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/winpixeventruntime.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/directx.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/vulkan.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/winsock.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/flann.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/openal.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/freeimage.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/nvidia-aftermath.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/dxrhelpers.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/metis.sharpmake.cs")]
//[module: Sharpmake.Include("../darkness-externals/rynx/rynx_projects.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/freetype.sharpmake.cs")]
[module: Sharpmake.Include("../protocustombuild.sharpmake.cs")]

[Generate]
public class DarknessEngine : DarknessStaticLibrary
{
    public protobuild.SharpmakeProtobufTool protoTool;

    public string protoReleaseExe;
    public string protoDebugExe;

    public string ProtoDestinationFolder;
    public string ProtoSourceFolder;
    // Path to QT
    public string ProtoPath;

    protected override void ExcludeOutputFiles()
    {
        base.ExcludeOutputFiles();
        protoTool.GenerateListOfFilesToGenerateProtos(this, ProtoDestinationFolder, ProtoSourceFolder, protoReleaseExe, protoDebugExe);
    }

    // At this point all of our includes and defines have been resolved, so now we can compute the arguments to moc.
    public override void PostLink()
    {
        protoTool.GenerateMocFileSteps(this);
        base.PostLink();
    }

    public DarknessEngine()
    {
        Name = "DarknessEngine";
        SourceRootPath = @"[project.SharpmakeCsPath]";
        AdditionalSourceRootPaths.Add(@"[project.SharpmakeCsPath]\protocols");
        SourceFilesExclude.Add("private-src/platform/window/OsxWindow.cpp");
        SourceFilesExtensions.Add(".py");
        SourceFilesExtensions.Add(".hlsl");
        SourceFilesExtensions.Add(".hlsli");

        ProtoSourceFolder = @"[project.SharpmakeCsPath]\protocols\";
        ProtoDestinationFolder = @"[project.SharpmakeCsPath]\include\protocols\";

        protoReleaseExe = @"[project.SharpmakeCsPath]\..\darkness-externals\generated\protoc\output\win64\release\protoc.exe";
        protoDebugExe = @"[project.SharpmakeCsPath]\..\darkness-externals\generated\protoc\output\win64\debug\protoc.exe";

        protoTool = new protobuild.SharpmakeProtobufTool();
        SourceFilesExtensions.Add(".proto");
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        var excludedFolders = new List<string>();

        var DX12Enabled = true;
        var VulkanEnabled = true;


        if(DX12Enabled)
        {
            //excludedFolders.Add("vulkan");
            conf.Defines.Add("GRAPHICS_API_DX12");
            conf.Defines.Add("USE_DMLX=0");
            conf.AddPublicDependency<Directx>(target);
            conf.AddPublicDependency<WinPIXEventRuntime>(target);
            conf.Defines.Add("USE_PIX");
            conf.ExportDefines.Add("USE_PIX");
        }

        if (VulkanEnabled)
        {
            //excludedFolders.Add("dx12");
            conf.Defines.Add("GRAPHICS_API_VULKAN");
            conf.Defines.Add("VK_USE_PLATFORM_WIN32_KHR");
            conf.AddPublicDependency<Vulkan>(target);
        }


        excludedFolders.Add("codegen");
        if (target.Platform == Platform.durango)
        {
            excludedFolders.Add("network");
            excludedFolders.Add("windows");
            excludedFolders.Add("osx");
            excludedFolders.Add("resources");
            conf.Defines.Add("_XBOX_ONE");
            conf.Defines.Add("_TITLE");
        }
        if (target.Platform == Platform.win64)
        {
            excludedFolders.Add("durango");
            excludedFolders.Add("osx");
        }

        conf.SourceFilesBuildExcludeRegex.Add(@"\.*\\(" + string.Join("|", excludedFolders.ToArray()) + @")\\");

        //conf.Options.Add(Options.CSharp.TreatWarningsAsErrors.Enabled);

        conf.IncludePaths.Add(@"[project.SourceRootPath]/.");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/include");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/private-include");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/../darkness-shared/include");
        
        conf.Defines.Add("_ENABLE_EXTENDED_ALIGNED_STORAGE");
        conf.Defines.Add("UNICODE");
        conf.Defines.Add("_UNICODE");
        conf.Defines.Add("NOMINMAX");

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.AddPublicDependency<protoc>(target);
        conf.AddPublicDependency<DarknessShared>(target);
        conf.AddPublicDependency<Bullet>(target);
        conf.AddPublicDependency<BZip2>(target);
        conf.AddPublicDependency<ImGui>(target);
        conf.AddPublicDependency<FreeType>(target);

        //conf.AddPublicDependency<Rynx>(target);

        if ((target.Platform != Platform.durango) &&
            (target.Platform != Platform.android))
        {
            conf.AddPublicDependency<Gainput>(target);
            conf.AddPublicDependency<ZeroMQ>(target);
            conf.AddPublicDependency<libprotobuf>(target);
            conf.AddPublicDependency<WinSock>(target);
            conf.AddPublicDependency<OpenAL>(target);
            conf.AddPublicDependency<NVidiaAftermath>(target);
            conf.AddPublicDependency<DXRHelpers>(target);
            conf.AddPublicDependency<Metis>(target);
        }
        conf.AddPublicDependency<RapidJSON>(target);
        conf.AddPublicDependency<Zstd>(target);
        conf.AddPublicDependency<Flann>(target);
        conf.AddPublicDependency<FreeImage>(target);

    }
}
