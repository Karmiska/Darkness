using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../darkness-engine/darkness-engine.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/qt.sharpmake.cs")]
[module: Sharpmake.Include("../qtcustombuild.sharpmake.cs")]

[Generate]
public class DarknessResourceHost : DarknessExecutable
{
    public DarknessResourceHost()
    {
        Name = "DarknessResourceHost";
        SourceRootPath = @"[project.SharpmakeCsPath]/src";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
        conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);

        conf.Defines.Add("_WIN32");
        conf.Defines.Add("ZMQ_STATIC");
        conf.Defines.Add("NOMINMAX");
        conf.Defines.Add("QT_SHARED");

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.AddPublicDependency<DarknessEngine>(target);

        conf.Options.Add(Options.Vc.Linker.SubSystem.Application);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
    }
}
