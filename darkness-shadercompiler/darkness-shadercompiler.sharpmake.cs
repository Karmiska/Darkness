using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../darkness-shared/darkness-shared.sharpmake.cs")]

[Generate]
public class DarknessShaderCompiler : DarknessExecutable
{
    public DarknessShaderCompiler()
    {
        Name = "DarknessShaderCompiler";
        SourceRootPath = @"[project.SharpmakeCsPath]/src";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
        conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);

        conf.Defines.Add("_WIN32");

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.IncludePaths.Add(@"[project.SourceRootPath]/../darkness-shared/include");
        conf.AddPublicDependency<DarknessShared>(target);

        conf.Options.Add(Options.Vc.Linker.SubSystem.Console);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
    }
}
