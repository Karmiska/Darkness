using System.IO;
using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("../darkness-engine/darkness-engine.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/qt.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/compressonator.sharpmake.cs")]
[module: Sharpmake.Include("../darkness-externals/freeimage.sharpmake.cs")]
[module: Sharpmake.Include("../qtcustombuild.sharpmake.cs")]

[Generate]
class DarknessResourceClient : DarknessExecutable
{
    public QtSharpmakeMocTool mocTool;
    // Path the qt executables
    public string QTExeFolder;
    // Path for generated QT files that don't depend on compiler parameters.
    public string QTSharedFolder;
    // Path to QT
    public string QTPath;

    protected override void ExcludeOutputFiles()
    {
        base.ExcludeOutputFiles();
        mocTool.GenerateListOfFilesToMoc(this, QTSharedFolder, QTExeFolder);
    }

    // At this point all of our includes and defines have been resolved, so now we can compute the arguments to moc.
    public override void PostLink()
    {
        mocTool.GenerateMocFileSteps(this);
        base.PostLink();
    }

    public DarknessResourceClient()
    {
        Name = "DarknessResourceClient";
        SourceRootPath = @"[project.SharpmakeCsPath]/src";
        AdditionalSourceRootPaths.Add(@"[project.SharpmakeCsPath]/data");

        QTSharedFolder = @"[project.SharpmakeCsPath]\moc\";
        QTPath = Qt.getQtPath();
        QTExeFolder = @"[project.QTPath]\bin\";

        mocTool = new QtSharpmakeMocTool();
        //mocTool.ExcludeMocFromCompileRegex.Add("floatcosanglespinbox.h");
        //mocTool.ExcludeMocFromCompileRegex.Add("privatewidget.h");

        SourceFilesExtensions.Add(".qrc", ".ui");

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

        conf.AddPublicDependency<DarknessEngine>(target);
        conf.AddPublicDependency<Qt>(target);
        conf.AddPublicDependency<Compressonator>(target);
        conf.AddPublicDependency<FreeImage>(target);

        conf.TargetPath = @"[project.SharpmakeCsPath]\bin\[target.Platform]\[conf.Name]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\ide\[target.DevEnv]";
        conf.ProjectFileName = @"[project.Name].[target.DevEnv]";

        conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]\moc\DarknessResourceClient.rcc");

        conf.Options.Add(Options.Vc.Linker.SubSystem.Application);
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
    }
}
