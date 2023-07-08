using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../darkness-engine/darkness-engine.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-ui/darkness-ui.sharpmake.cs")]

[Generate]
public class DarknessEditorV2 : DarknessExecutable
{
    public DarknessEditorV2()
    {
        Name = "DarknessEditorV2";
        SourceRootPath = @"[project.SharpmakeCsPath]";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/include");

        conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
        conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);

        conf.Defines.Add("_WIN32");

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.AddPublicDependency<DarknessEngine>(target);
        conf.AddPublicDependency<DarknessUi>(target);
        conf.Options.Add(Options.Vc.Linker.SubSystem.Application);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
    }
}
