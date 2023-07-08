using System.IO;
using Sharpmake;
using System.Collections.Generic;

public static class DarknessSettings
{
    public const Platform platform = Platform.win64;
    public const DevEnv devEnv = DevEnv.vs2022;
    public const Optimization optimization = Optimization.Debug | Optimization.Release;
    public const OutputType outputType = OutputType.Dll;
    public const Blob blob = Blob.NoBlob;
}

public abstract class DarknessSolution : Solution
{
    public DarknessSolution()
    {
        AddTargets(new Target(
            DarknessSettings.platform,
            DarknessSettings.devEnv,
            DarknessSettings.optimization,
            DarknessSettings.outputType,
            DarknessSettings.blob
            ));
    }

    [Configure]
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
        conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);
        conf.Options.Add(Options.Vc.General.PlatformToolset.v143);
    }
}

public abstract class DarknessBaseProject : Project
{
    public DarknessBaseProject()
    {
        AddTargets(new Target(
            DarknessSettings.platform,
            DarknessSettings.devEnv,
            DarknessSettings.optimization,
            DarknessSettings.outputType,
            DarknessSettings.blob
            ));
    }

    [Configure]
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
        // Gives a unique path for the project because Visual Studio does not
        // like shared intermediate directories.
        conf.ProjectPath = Path.Combine("[project.SharpmakeCsPath]/generated/[project.Name]");

        conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
        conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);
        conf.Options.Add(Options.Vc.Compiler.MultiProcessorCompilation.Enable);
        conf.Options.Add(Sharpmake.Options.Vc.General.WarningLevel.Level4);
        conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);
        conf.Options.Add(Options.Vc.General.PlatformToolset.v143);

        if (target.Platform == Platform.android)
        {
            conf.Options.Add(Options.Android.General.UseOfStl.GnuStl_Static);
            conf.Options.Add(Options.Android.General.ThumbMode.Thumb);
            conf.Options.Add(Options.Android.Compiler.CppLanguageStandard.Cpp11);
            conf.Options.Add(Options.Android.General.AndroidAPILevel.Android24);
        }

        switch (target.Optimization)
        {
            case Optimization.Debug:
                {
                    conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
                    break;
                }
            case Optimization.Release:
                {
                    conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
                    break;
                }
        }

        if (target.Platform == Platform.durango)
        {
            conf.Options.Add(new Options.Vc.Linker.IgnoreSpecificLibraryNames(new string[] { "advapi32.lib", "atl.lib", "atls.lib", "atlsd.lib", "atlsn.lib", "atlsnd.lib", "comctl32.lib", "comsupp.lib", "dbghelp.lib", "gdi32.lib", "gdiplus.lib", "guardcfw.lib", "kernel32.lib", "mmc.lib", "msimg32.lib", "msvcole.lib", "msvcoled.lib", "mswsock.lib", "ntstrsafe.lib", "ole2.lib", "ole2autd.lib", "ole2auto.lib", "ole2d.lib", "ole2ui.lib", "ole2uid.lib", "ole32.lib", "oleacc.lib", "oleaut32.lib", "oledlg.lib", "oledlgd.lib", "oldnames.lib", "runtimeobject.lib", "shell32.lib", "shlwapi.lib", "strsafe.lib", "urlmon.lib", "user32.lib", "userenv.lib", "uuid.lib", "wlmole.lib", "wlmoled.lib" }));

            conf.LibraryFiles.Add("pixEvt.lib");
            conf.LibraryFiles.Add("combase.lib");
            conf.LibraryFiles.Add("kernelx.lib");
            conf.LibraryFiles.Add("uuid.lib");

            string rootPath = global::System.IO.Path.GetFullPath(this.SharpmakeCsPath + @"\");
            string x1FilePath = global::System.IO.Path.Combine(rootPath, @"data\");
            var x1Files = new global::System.Collections.Generic.List<string>();

            x1Files.Add(x1FilePath + "Logo.png");
            x1Files.Add(x1FilePath + "WideLogo.png");
            x1Files.Add(x1FilePath + "StoreLogo.png");
            x1Files.Add(x1FilePath + "SplashScreen.png");
            x1Files.Add(x1FilePath + "SmallLogo.png");

            foreach (string file in x1Files)
            {
                string fileName = global::System.IO.Path.GetFileName(file);
                string targetFile = global::System.IO.Path.Combine(conf.TargetPath, fileName);
                var fileCopy = new global::System.Collections.Generic.KeyValuePair<string, string>(file, targetFile);
                conf.EventPostBuildCopies.Add(fileCopy);
            }
        }
    }

    [Configure(Blob.Blob)]
    public virtual void ConfigureBlob(Configuration conf, Target target)
    {
        conf.IsBlobbed = false;
        /*conf.ProjectName += "_Blob";
        conf.SolutionFolder = "Blob";
        conf.ProjectFileName += ".blob";*/
        conf.IncludeBlobbedSourceFiles = false;
    }
}

public abstract class DarknessStaticLibrary : DarknessBaseProject
{
    public DarknessStaticLibrary()
    {
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.Output = Configuration.OutputType.Lib;
    }

    [Configure(Blob.Blob)]
    public override void ConfigureBlob(Configuration conf, Target target)
    {
        base.ConfigureBlob(conf, target);
    }
}

public abstract class DarknessDynamicLibrary : DarknessBaseProject
{
    public DarknessDynamicLibrary()
    {
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.Output = Configuration.OutputType.Dll;
    }

    [Configure(Blob.Blob)]
    public override void ConfigureBlob(Configuration conf, Target target)
    {
        base.ConfigureBlob(conf, target);
    }
}

public abstract class DarknessExecutable : DarknessBaseProject
{
    public DarknessExecutable()
    {
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.Output = Configuration.OutputType.Exe;
    }

    [Configure(Blob.Blob)]
    public override void ConfigureBlob(Configuration conf, Target target)
    {
        base.ConfigureBlob(conf, target);
    }
}
