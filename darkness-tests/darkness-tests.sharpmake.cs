using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../darkness-engine/darkness-engine.sharpmake.cs")]

[Generate]
public class DarknessTests : DarknessExecutable
{
    public DarknessTests()
    {
        Name = "DarknessTests";
        SourceRootPath = @"[project.SharpmakeCsPath]/src";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
        conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);

        conf.Defines.Add("_WIN32");

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.IncludePaths.Add(@"[project.SourceRootPath]/.");

        conf.AddPublicDependency<DarknessEngine>(target);
        conf.Options.Add(Options.Vc.Linker.SubSystem.Console);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
    }
}
