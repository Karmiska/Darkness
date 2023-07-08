using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../common.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-engine/darkness-engine.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-shared/darkness-shared.sharpmake.cs")]

[Generate]
public class DarknessUi : DarknessStaticLibrary
{
    public DarknessUi()
    {
        Name = "DarknessUi";
        SourceRootPath = @"[project.SharpmakeCsPath]";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/include");

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);

        conf.AddPublicDependency<DarknessEngine>(target);
        conf.AddPublicDependency<DarknessShared>(target);
    }
}
