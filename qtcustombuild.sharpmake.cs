using System;
using Sharpmake;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Text;
using ProjConfiguration = Sharpmake.Project.Configuration;

public class AdditionalDefinition
{
    public Sharpmake.Platform Platform;
    public Sharpmake.DevEnv DevEnv;
    public Strings Defines;

    public AdditionalDefinition(Sharpmake.Platform platform, Sharpmake.DevEnv dev, params string[] defines)
    {
        Platform = platform;
        DevEnv = dev;
        Defines = new Strings(defines);
    }
}

public class QtSharpmakeMocTool
{
    // Mapping of target name to all the files that should generate a moc call.
    public Dictionary<ProjConfiguration, List<MocSourceAndTargetFile>> MocTargetsPerConfiguration = new Dictionary<ProjConfiguration, List<MocSourceAndTargetFile>>();
    // Mapping of target name to all the files that should generate a rcc call.
    public Dictionary<ProjConfiguration, List<RccSourceAndTargetFile>> RccTargetsPerConfiguration = new Dictionary<ProjConfiguration, List<RccSourceAndTargetFile>>();
    // Mapping of target name to all the files that should generate a uic call.
    public Dictionary<ProjConfiguration, List<UicSourceAndTargetFile>> UicTargetsPerConfiguration = new Dictionary<ProjConfiguration, List<UicSourceAndTargetFile>>();
    // Files that should be moc'd but should not be compiled alone (they will be included in another cpp file).
    public Strings ExcludeMocFromCompileRegex = new Strings();
    // Files that should not be moc'd, skip scanning them.   They may have a Q_OBJECT, but it's fake.
    public Strings ExcludeMocRegex = new Strings();
    // A way to give defines to moc.
    public List<AdditionalDefinition> AdditionalDefines = new List<AdditionalDefinition>();

    public QtSharpmakeMocTool()
    {
        //AdditionalDefines.Add(new AdditionalDefinition(Sharpmake.Platform.win64 | Sharpmake.Platform.win32, Sharpmake.DevEnv.vs2012, "WIN32", "_MSC_VER=1700"));
        //AdditionalDefines.Add(new AdditionalDefinition(Sharpmake.Platform.win64 | Sharpmake.Platform.win32, Sharpmake.DevEnv.vs2015, "WIN32", "_MSC_VER=1900"));
        //AdditionalDefines.Add(new AdditionalDefinition(Sharpmake.Platform.win64 | Sharpmake.Platform.win32, Sharpmake.DevEnv.vs2017, "WIN32", "_MSC_VER=1910"));
    }

    // Stores the source file and target file of a moc operation.
    public class MocSourceAndTargetFile : ProjConfiguration.CustomFileBuildStep
    {
        // The true input file.
        public string SourceFile;
        // Intermediate file used for a custom build step to produce the output from the input when the input is a source file.
        public string IntermediateFile;
        // True if source file, false if header file.
        public bool IsCPPFile;
        // List of includes to use.
        public Strings IncludePaths = new Strings();
        // List of force-includes
        public Strings ForceIncludes = new Strings();
        // Defines
        public string CombinedDefines = "";

        public MocSourceAndTargetFile(string targetName, string mocExe, string baseOutputFolder, string outputFolder, string sourceFile)
        {
            Executable = mocExe;
            SourceFile = Util.GetCapitalizedPath(sourceFile);
            KeyInput = SourceFile;
            IsCPPFile = sourceFile.EndsWith(".cpp", StringComparison.InvariantCultureIgnoreCase);
            if (IsCPPFile)
            {
                // Put this one up one level, as it's just a boot strap file.
                IntermediateFile = baseOutputFolder + Path.GetFileNameWithoutExtension(sourceFile) + ".inl";
                Output = outputFolder + Path.GetFileNameWithoutExtension(sourceFile) + ".moc";
                // BFF only.
                Filter = ProjectFilter.BFFOnly;
            }
            else
            {
                ForceIncludes.Add(sourceFile);
                Output = outputFolder + "moc_" + Path.GetFileNameWithoutExtension(sourceFile) + ".cpp";
            }

            Description = string.Format("Moc {0} {1}", targetName, Path.GetFileName(sourceFile));
            ExecutableArguments = "[input] -o [output]";
        }

        // Makes a Vcxproj rule from a BFFOnly rule.
        protected MocSourceAndTargetFile(MocSourceAndTargetFile reference)
        {
            Filter = ProjectFilter.ExcludeBFF;
            Executable = reference.Executable;
            // Input is the intermediate file.
            KeyInput = reference.IntermediateFile;
            // We also depend on the actual input file.
            AdditionalInputs.Add(reference.KeyInput);
            Output = reference.Output;
            Description = reference.Description;

            SourceFile = Util.GetCapitalizedPath(reference.SourceFile);
            IntermediateFile = reference.IntermediateFile;
            IsCPPFile = reference.IsCPPFile;

            IncludePaths.AddRange(reference.IncludePaths);
            ForceIncludes.AddRange(reference.ForceIncludes);
            CombinedDefines = reference.CombinedDefines;
        }

        // We get built too late to handle the initial resolve (as we need the files built afterwards), but we get the other two events.
        public override ProjConfiguration.CustomFileBuildStepData MakePathRelative(Resolver resolver, Func<string, bool, string> MakeRelativeTool)
        {
            var relativeData = base.MakePathRelative(resolver, MakeRelativeTool);

            // These are command line relative.
            Strings RelativeIncludePaths = new Strings();
            foreach (string key in IncludePaths)
                RelativeIncludePaths.Add(MakeRelativeTool(key, true));
            // These should be compiler relative instead of command line relative, but generally they're the same.
            Strings RelativeForceIncludes = new Strings();
            foreach (string key in ForceIncludes)
                RelativeForceIncludes.Add(MakeRelativeTool(key, false));

            RelativeIncludePaths.InsertPrefix("-I");
            RelativeForceIncludes.InsertPrefix("-f");
            //relativeData.ExecutableArguments = CombinedDefines + " " + RelativeIncludePaths.JoinStrings(" ") + " " + RelativeForceIncludes.JoinStrings(" ") + " " + relativeData.ExecutableArguments;
            relativeData.ExecutableArguments = CombinedDefines + " " + RelativeIncludePaths.JoinStrings(" ") + " " + relativeData.ExecutableArguments;

            return relativeData;
        }
    }

    // Needed for vcx when the input is a source file, we need to specify a different rule based on an intermediate file.
    // MocSourceAndTargetFile already setups up this data, we just need a non-bff rule.
    public class MocVcxprojBuildStep : MocSourceAndTargetFile
    {
        public MocVcxprojBuildStep(MocSourceAndTargetFile reference)
            : base(reference)
        {
        }
    }

    // Stores the source file and target file of a rcc operation
    public class RccSourceAndTargetFile : ProjConfiguration.CustomFileBuildStep
    {
        public RccSourceAndTargetFile(string targetName, string rccExe, string outputFolder, string sourceFile)
        {
            Executable = rccExe;
            KeyInput = Util.GetCapitalizedPath(sourceFile);
            string ResourceName = Path.GetFileNameWithoutExtension(KeyInput);
            Output = outputFolder + ResourceName + ".rcc";

            Description = string.Format("Rcc {0} {1}", targetName, Path.GetFileName(KeyInput));
            //ExecutableArguments = "-name " + ResourceName + " [input] -o [output]";
            ExecutableArguments = "-binary [input] -o [output]";
        }
    }

    // Stores the source file and target file of a uic operation
    public class UicSourceAndTargetFile : ProjConfiguration.CustomFileBuildStep
    {
        public UicSourceAndTargetFile(string targetName, string uicExe, string outputFolder, string sourceFile)
        {
            Executable = uicExe;
            KeyInput = Util.GetCapitalizedPath(sourceFile);
            Output = outputFolder + "ui_" + Path.GetFileNameWithoutExtension(KeyInput) + ".h";
            Description = string.Format("Uic {0} {1}", targetName, Path.GetFileName(KeyInput));
            ExecutableArguments = "[input] -o [output]";
        }
    }

    bool FileIsPrecompiledHeader(string file, ProjConfiguration conf)
    {
        return (conf.PrecompHeader != null && file.EndsWith(conf.PrecompHeader, StringComparison.InvariantCultureIgnoreCase))
                    || (conf.PrecompSource != null && file.EndsWith(conf.PrecompSource, StringComparison.InvariantCultureIgnoreCase));
    }

    static int GetIndexMatchedAtEnd(string fileBuffer, string stringToFind)
    {
        int len = stringToFind.Length;
        int indexOfMatch = len > 0 ? len - 1 : 0;
        for (; indexOfMatch > 0; --indexOfMatch)
        {
            if (fileBuffer.EndsWith(stringToFind.Substring(0, indexOfMatch)))
                return indexOfMatch;
        }
        return 0;
    }

    async Task<bool> FileContainsQObject(string file)
    {
        try
        {
            const int numBytesPerPage = 0x1000;
            using (StreamReader sourceStream = new StreamReader(new FileStream(file,
                    FileMode.Open, FileAccess.Read, FileShare.Read,
                    bufferSize: numBytesPerPage, useAsync: true)))
            {
                string[] stringsToFind = new string[2];
                stringsToFind[0] = "Q_OBJECT";
                stringsToFind[1] = "Q_GADGET";
                int[] fractionsMatched = new int[2];
                fractionsMatched[0] = 0;
                fractionsMatched[1] = 0;

                char[] buffer = new char[numBytesPerPage];
                int numRead;
                while ((numRead = await sourceStream.ReadAsync(buffer, 0, buffer.Length)) != 0)
                {
                    string text = new string(buffer);
                    // If we partially matched the previous block, see if we match the rest.
                    for (int i = 0; i < fractionsMatched.Length; ++i)
                    {
                        if (fractionsMatched[i] != 0)
                        {
                            if (text.StartsWith(stringsToFind[i].Substring(fractionsMatched[i])))
                                return true;
                        }

                        if (text.Contains(stringsToFind[i]))
                            return true;
                        fractionsMatched[i] = GetIndexMatchedAtEnd(text, stringsToFind[i]);
                    }
                }

                return false;
            }
        }
        catch (Exception ex)
        {
            System.Console.WriteLine("While looking at file {0} encountered exception {1}, not mocing the file!", file, ex.Message);
            return false;
        }
    }

    // Call this from Project::ExcludeOutputFiles() to find the list of files we need to moc.
    // This is after resolving files, but before filtering them, and before they get mapped to
    // configurations, so this is a good spot to add additional files.
    public void GenerateListOfFilesToMoc(Project project, string sharedFolder, string QTExecFolder)
    {
        string mocExe = QTExecFolder + "moc.exe";
        string rccExe = QTExecFolder + "rcc.exe";
        string uicExe = QTExecFolder + "uic.exe";

        // Filter all the files by the filters we've already specified, so we don't moc a file that's excluded from the solution.
        List<Regex> filters = project.SourceFilesExcludeRegex.Select(filter => new Regex(filter, RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase)).ToList();
        filters.AddRange(ExcludeMocRegex.Select(filter => new Regex(filter, RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase)));

        var preFilteredFiles = project.ResolvedSourceFiles.Where(file => !filters.Any(filter => filter.IsMatch(file)) && !project.Configurations.Any(conf => FileIsPrecompiledHeader(file, conf))).ToList();

        // Async load all the source files and look for Q_OBJECT that we want to keep.
        var answerSetTask = Task.WhenAll(preFilteredFiles.Select(async file => new { file = file, runMoc = await FileContainsQObject(file) }));
        // Compile a list of qrc and ui files.
        Strings qrcFiles = new Strings(preFilteredFiles.Where(file => file.EndsWith(".qrc", StringComparison.InvariantCultureIgnoreCase)));
        Strings uiFiles = new Strings(preFilteredFiles.Where(file => file.EndsWith(".ui", StringComparison.InvariantCultureIgnoreCase)));
        // Wait for the moc files.
        answerSetTask.Wait();
        var filterPass = answerSetTask.Result;
        // These are the files we want to moc.
        Strings FilteredResolvedSourceFiles = new Strings(filterPass.Where(result => result.runMoc).Select(result => result.file));

        // Compile a list of files where we don't want to compile the moc output.
        List<Regex> filesToExclude = ExcludeMocFromCompileRegex.Select(filter => new Regex(filter, RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase)).ToList();


        foreach (ProjConfiguration conf in project.Configurations)
        {
            // Setup exclusions.
            string QTMocOutputBase = Path.GetDirectoryName(conf.IntermediatePath);
            string targetName = conf.Target.Name;
            string outputFolder = sharedFolder;// QTMocOutputBase + @"\qt\" + targetName + @"\";

            // We make the current output folder included directly so you can use the same #include directive to get the correct cpp file.
            conf.IncludePrivatePaths.Add(sharedFolder);

            // We need to exclude the generation files folder from the build on all targets except our own.
            string rootFolderForRegex = Util.GetCapitalizedPath(conf.ProjectPath);
            string outputRegex = Util.PathGetRelative(rootFolderForRegex, outputFolder);
            outputRegex = outputRegex.Replace("..\\", "").Replace("\\", "\\\\") + @"\\";
            foreach (ProjConfiguration confToExclude in project.Configurations)
            {
                if (confToExclude == conf || confToExclude.ProjectFullFileNameWithExtension != conf.ProjectFullFileNameWithExtension)
                    continue;
                //confToExclude.SourceFilesBuildExcludeRegex.Add(outputRegex);
            }

            // Build a list of all files to moc in this configuration.
            var mocTargets = new List<MocSourceAndTargetFile>();
            foreach (string file in FilteredResolvedSourceFiles)
            {
                var target = new MocSourceAndTargetFile(targetName, mocExe, sharedFolder, outputFolder, file);
                mocTargets.Add(target);
                if (filesToExclude.Any(filter => filter.IsMatch(file)))
                {
                    conf.SourceFilesBuildExcludeRegex.Add(Path.GetFileName(target.Output));
                }
                if(target.IsCPPFile)
                {
                    mocTargets.Add(new MocVcxprojBuildStep(target));
                }
            }
            if (mocTargets.Count > 0)
            {
                MocTargetsPerConfiguration.Add(conf, mocTargets);
            }

            if (qrcFiles.Count > 0)
            {
                RccTargetsPerConfiguration.Add(conf, qrcFiles.Select(file => new RccSourceAndTargetFile(targetName, rccExe, sharedFolder, file)).ToList());
            }
            if (uiFiles.Count > 0)
            {
                UicTargetsPerConfiguration.Add(conf, uiFiles.Select(file => new UicSourceAndTargetFile(targetName, uicExe, sharedFolder, file)).ToList());
            }
        }

        // Add all the new source files to the project file.
        foreach (var values in MocTargetsPerConfiguration)
        {
            foreach (var target in values.Value)
            {
                values.Key.CustomFileBuildSteps.Add(target);
                project.ResolvedSourceFiles.Add(target.Output);
                if (target.IsCPPFile)
                    project.ResolvedSourceFiles.Add(target.IntermediateFile);
            }
        }

        foreach (var values in RccTargetsPerConfiguration)
        {
            foreach (var target in values.Value)
            {
                values.Key.CustomFileBuildSteps.Add(target);
                project.ResolvedSourceFiles.Add(target.Output);
            }
        }

        foreach (var values in UicTargetsPerConfiguration)
        {
            foreach (var target in values.Value)
            {
                values.Key.CustomFileBuildSteps.Add(target);
                project.ResolvedSourceFiles.Add(target.Output);
            }
        }
    }

    private void CreateIntermediateFile(string sourceFile, string intermediateFile)
    {
        // Create the intermediate file if it doesn't already exist.   Visual studio seems to ignore the custom build step unless the file already exists.
        if (!File.Exists(intermediateFile))
        {
            try
            {
                string directory = Path.GetDirectoryName(intermediateFile);
                if (!Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                }
                if (!File.Exists(intermediateFile))
                {
                    StreamWriter writer = File.CreateText(intermediateFile);
                    writer.WriteLineAsync(sourceFile).ContinueWith(a => writer.Close());
                    System.Console.WriteLine("  Created {0}", intermediateFile);
                }
            }
            catch (IOException e)
            {
                // Sharing violation is fine, it means we're about to create the file on another thread.
                const int SharingViolation = 0x20;
                if ((e.HResult & 0xFFFF) != SharingViolation)
                {
                    Console.WriteLine("Unable to generate intermediate file {0}: {1}", intermediateFile, e);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Unable to generate intermediate file {0}: {1}", intermediateFile, e);
            }
        }
    }

    private void GenerateMocFileStepsForConfiguration(ProjConfiguration conf)
    {
        // Build a set of custom build steps from the source-target pairs.
        List<MocSourceAndTargetFile> mocTargets;
        if (!MocTargetsPerConfiguration.TryGetValue(conf, out mocTargets))
            return;

        // Copy the defines and add -D in front of them.
        Strings confDefines = new Strings(conf.Defines.Select(define => define.Replace(" ", "")));
        foreach (var additionalDefine in AdditionalDefines)
        {
            if ((conf.Target.GetPlatform() & additionalDefine.Platform) != 0 && (conf.Target.GetFragment<DevEnv>() & additionalDefine.DevEnv) != 0)
            {
                confDefines.AddRange(additionalDefine.Defines);
            }
        }
        confDefines.InsertPrefix("-D");
        string combinedDefines = confDefines.JoinStrings(" ");

        // Combine all the different includes into a single string pool.
        Strings confIncludes = new Strings(conf.IncludePaths);
        confIncludes.AddRange(conf.DependenciesIncludePaths);
        confIncludes.AddRange(conf.IncludePrivatePaths);
        // Quote the include strings, if need be.
        List<string> includeValues = confIncludes.Values;
        foreach (string path in includeValues)
        {
            if (path.Contains(' '))
            {
                confIncludes.UpdateValue(path, "\"" + path + "\"");
            }
        }

        Strings precompiledHeader = new Strings();

        // Build the string we need to pass to moc for all calls.
        if (conf.PrecompHeader != null)
        {
            // If we have a precompiled header, we need the new cpp file to include this also.
            // Technically we don't need to do this if the file is in ExcludeMocFromCompileRegex
            precompiledHeader.Add(conf.PrecompHeader);
        }

        // Apply these settings to all Moc targets.
        foreach (var target in mocTargets)
        {
            target.CombinedDefines = combinedDefines;
            target.IncludePaths.AddRange(confIncludes);
            if(!target.IsCPPFile)
                target.ForceIncludes.AddRange(precompiledHeader);
        }
    }

    // Call this in Project::PostLink().   We will build a list of custom build steps based on the resolved includes and defines.
    // At this point all of our includes and defines have been resolved, so now we can compute the arguments to moc.
    public void GenerateMocFileSteps(Project project)
    {
        foreach (ProjConfiguration conf in project.Configurations)
        {
            // Compute all the define and include parameters for this configuration.
            GenerateMocFileStepsForConfiguration(conf);
        }
    }
}
