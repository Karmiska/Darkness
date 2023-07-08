using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../darkness-engine/darkness-engine.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-shared/darkness-shared.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/compressonator.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/amd-tootle.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/assimp.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/simplygon.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/fbxsdk.sharpmake.cs")]

[Generate]
public class DarknessResourceTask : DarknessExecutable
{
    public DarknessResourceTask()
    {
        Name = "DarknessResourceTask";
        SourceRootPath = @"[project.SharpmakeCsPath]/src";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Defines.Add("_WIN32");
        conf.Defines.Add("ZMQ_STATIC");
        conf.Defines.Add("NOMINMAX");

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.AddPublicDependency<DarknessEngine>(target);
        conf.AddPublicDependency<DarknessShared>(target);
        conf.AddPublicDependency<Compressonator>(target);
        conf.AddPublicDependency<AmdTootle>(target);
        conf.AddPublicDependency<AssImp>(target);
        conf.AddPublicDependency<Simplygon>(target);
        conf.AddPublicDependency<FbxSDK>(target);

        conf.AdditionalManifestFiles.Add(@"[project.SharpmakeCsPath]\data\Compatibility.manifest");

        conf.Options.Add(Options.Vc.Linker.SubSystem.Application);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
    }
}
