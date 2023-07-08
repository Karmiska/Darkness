using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../common.sharpmake.cs")]
//[module: Sharpmake.Include("../darkness-externals/eastl.sharpmake.cs")]

[Generate]
public class DarknessShared : DarknessStaticLibrary
{
    public DarknessShared()
    {
        Name = "DarknessShared";
        SourceRootPath = @"[project.SharpmakeCsPath]";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.IncludePaths.Add(@"[project.SourceRootPath]/include");

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        //conf.AddPublicDependency<EAStl>(target);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
    }
}
