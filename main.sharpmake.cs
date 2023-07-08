using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("common.sharpmake.cs")]

[module: Sharpmake.Include("darkness-shadercompiler/darkness-shadercompiler.sharpmake.cs")]
[module: Sharpmake.Include("darkness-externals/bzip2.sharpmake.cs")]
[module: Sharpmake.Include("darkness-externals/protobuf.sharpmake.cs")]
[module: Sharpmake.Include("darkness-engine/darkness-engine.sharpmake.cs")]
//[module: Sharpmake.Include("darkness-editor/darkness-editor.sharpmake.cs")]
[module: Sharpmake.Include("darkness-editor-v2/darkness-editor-v2.sharpmake.cs")]
//[module: Sharpmake.Include("darkness-launcher/darkness-launcher.sharpmake.cs")]
[module: Sharpmake.Include("darkness-game/darkness-game.sharpmake.cs")]
[module: Sharpmake.Include("darkness-resourcehost/darkness-resourcehost.sharpmake.cs")]
[module: Sharpmake.Include("darkness-resourcetask/darkness-resourcetask.sharpmake.cs")]
//[module: Sharpmake.Include("darkness-resourceclient/darkness-resourceclient.sharpmake.cs")]
[module: Sharpmake.Include("darkness-coreplugins/darkness-coreplugins.sharpmake.cs")]
[module: Sharpmake.Include("darkness-tests/darkness-tests.sharpmake.cs")]

[Generate]
public class Darkness : DarknessSolution
{
    public Darkness()
    {
        Name = "Darkness";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionPath = @"[solution.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.SolutionFileName = @"[solution.Name].[target.DevEnv]";

        conf.AddProject<DarknessShaderCompiler>(target);

        if (target.Platform != Platform.android)
        {
            conf.AddProject<protoc>(target);
            conf.AddProject<DarknessResourceHost>(target);
            conf.AddProject<DarknessResourceTask>(target);
            //conf.AddProject<DarknessResourceClient>(target);
            //conf.AddProject<DarknessEditor>(target);
            //conf.AddProject<DarknessLauncher>(target);
            conf.AddProject<DarknessTests>(target);
            conf.AddProject<DarknessEditorV2>(target);
        }
        conf.AddProject<DarknessEngine>(target);
        conf.AddProject<DarknessGame>(target);
        conf.AddProject<DarknessCorePlugins>(target);
    }

    [Main]
    public static void SharpmakeMain(Arguments sharpmakeArgs)
    {
        sharpmakeArgs.Generate<Darkness>();
    }
}
