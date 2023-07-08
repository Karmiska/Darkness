using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../darkness-shared/darkness-shared.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/libzmq.sharpmake.cs")]

[Generate]
public class DarknessCorePlugins : DarknessDynamicLibrary
{
    public DarknessCorePlugins()
    {
        Name = "DarknessCorePlugins";
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

        conf.AddPublicDependency<DarknessShared>(target);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
        if ((target.Platform != Platform.durango) &&
            (target.Platform != Platform.android))
        {
            conf.AddPublicDependency<ZeroMQ>(target);
        }
    }

    [Configure(Blob.Blob)]
    public override void ConfigureBlob(Configuration conf, Target target)
    {
        conf.IsBlobbed = false;
        conf.IncludeBlobbedSourceFiles = false;
    }
}
