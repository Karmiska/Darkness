using System;
using Sharpmake;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Text;
using ProjConfiguration = Sharpmake.Project.Configuration;

namespace protobuild
{
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

    public class SharpmakeProtobufTool
    {
        // Mapping of target name to all the files that should generate a moc call.
        public Dictionary<ProjConfiguration, List<ProtoSourceAndTargetFile>> MocTargetsPerConfiguration = new Dictionary<ProjConfiguration, List<ProtoSourceAndTargetFile>>();
        // Files that should be moc'd but should not be compiled alone (they will be included in another cpp file).
        public Strings ExcludeMocFromCompileRegex = new Strings();
        // Files that should not be moc'd, skip scanning them.   They may have a Q_OBJECT, but it's fake.
        public Strings ExcludeMocRegex = new Strings();
        // A way to give defines to moc.
        public List<AdditionalDefinition> AdditionalDefines = new List<AdditionalDefinition>();

        public SharpmakeProtobufTool()
        {
            //AdditionalDefines.Add(new AdditionalDefinition(Sharpmake.Platform.win64 | Sharpmake.Platform.win32, Sharpmake.DevEnv.vs2012, "WIN32", "_MSC_VER=1700"));
            //AdditionalDefines.Add(new AdditionalDefinition(Sharpmake.Platform.win64 | Sharpmake.Platform.win32, Sharpmake.DevEnv.vs2015, "WIN32", "_MSC_VER=1900"));
            //AdditionalDefines.Add(new AdditionalDefinition(Sharpmake.Platform.win64 | Sharpmake.Platform.win32, Sharpmake.DevEnv.vs2017, "WIN32", "_MSC_VER=1910"));
        }

        // Stores the source file and target file of a moc operation.
        public class ProtoSourceAndTargetFile : ProjConfiguration.CustomFileBuildStep
        {
            // The true input file.
            public string SourceFile;
            // True if source file, false if header file.
            public bool IsProtoFile;
            // List of includes to use.
            public Strings IncludePaths = new Strings();
            // List of force-includes
            public Strings ForceIncludes = new Strings();
            // Defines
            public string CombinedDefines = "";

            public ProtoSourceAndTargetFile(
                string targetName, 
                string protocExe, 
                string baseOutputFolder, 
                string outputFolder, 
                string sourceFile,
                string sourceFilePath,
                string sourceFileName)
            {
                Executable = protocExe;
                SourceFile = Util.GetCapitalizedPath(sourceFile);
                KeyInput = SourceFile;
                IsProtoFile = sourceFile.EndsWith(".proto", StringComparison.InvariantCultureIgnoreCase);

                if (IsProtoFile)
                {
                    ForceIncludes.Add(sourceFile);
                    Output = Path.GetFullPath(outputFolder + Path.ChangeExtension(sourceFileName, ".pb.h"));
                }

                Description = string.Format("Proto compile {0} {1}", targetName, Path.GetFileName(sourceFile));
                ExecutableArguments = "--proto_path=" + sourceFilePath.Replace(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + " " + sourceFileName + " --cpp_out=" + Path.GetFullPath(outputFolder).Replace(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            }

            // Makes a Vcxproj rule from a BFFOnly rule.
            protected ProtoSourceAndTargetFile(ProtoSourceAndTargetFile reference)
            {
                Filter = ProjectFilter.ExcludeBFF;
                Executable = reference.Executable;
                // Input is the intermediate file.
                //KeyInput = reference.IntermediateFile;
                // We also depend on the actual input file.
                AdditionalInputs.Add(reference.KeyInput);
                Output = reference.Output;
                Description = reference.Description;

                SourceFile = Util.GetCapitalizedPath(reference.SourceFile);
                //IntermediateFile = reference.IntermediateFile;
                IsProtoFile = reference.IsProtoFile;

                IncludePaths.AddRange(reference.IncludePaths);
                ForceIncludes.AddRange(reference.ForceIncludes);
                CombinedDefines = reference.CombinedDefines;
            }

            // We get built too late to handle the initial resolve (as we need the files built afterwards), but we get the other two events.
            public override ProjConfiguration.CustomFileBuildStepData MakePathRelative(Resolver resolver, Func<string, bool, string> MakeRelativeTool)
            {
                return base.MakePathRelative(resolver, MakeRelativeTool);
            }
        }

        // Needed for vcx when the input is a source file, we need to specify a different rule based on an intermediate file.
        // ProtoSourceAndTargetFile already setups up this data, we just need a non-bff rule.
        public class MocVcxprojBuildStep : ProtoSourceAndTargetFile
        {
            public MocVcxprojBuildStep(ProtoSourceAndTargetFile reference)
                : base(reference)
            {
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

        private bool FileIsProtoFile(string file)
        {
            if (file.EndsWith(".proto", StringComparison.InvariantCultureIgnoreCase))
                return true;
            else
                return false;
        }


        private string commonAncestor(List<string> paths)
        {
            var minimumLength = paths.Min(x => x.Length);
            int commonChars;
            for (commonChars = 0; commonChars < minimumLength; commonChars++)
            {
                if (paths.Select(x => x[commonChars]).Distinct().Count() > 1)
                {
                    break;
                }
            }
            return paths[0].Substring(0, commonChars);
        }

        // Call this from Project::ExcludeOutputFiles() to find the list of files we need to moc.
        // This is after resolving files, but before filtering them, and before they get mapped to
        // configurations, so this is a good spot to add additional files.
        public void GenerateListOfFilesToGenerateProtos(Project project, string protoDstFolder, string protoSrcFolder, string ProtoReleaseExe, string ProtoDebugExe)
        {
            string protocExe = "";
            if (Util.FileExists(ProtoReleaseExe))
            {
                protocExe = ProtoReleaseExe;
            }
            else if (Util.FileExists(ProtoDebugExe))
            {
                protocExe = ProtoDebugExe;
            }

            // Filter all the files by the filters we've already specified, so we don't moc a file that's excluded from the solution.
            List<Regex> filters = project.SourceFilesExcludeRegex.Select(filter => new Regex(filter, RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase)).ToList();
            filters.AddRange(ExcludeMocRegex.Select(filter => new Regex(filter, RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase)));

            var preFilteredFiles = project.ResolvedSourceFiles.Where(file => !filters.Any(filter => filter.IsMatch(file)) && !project.Configurations.Any(conf => FileIsPrecompiledHeader(file, conf))).ToList();

            // Async load all the source files and look for Q_OBJECT that we want to keep.
            //var answerSetTask = Task.WhenAll(preFilteredFiles.Select(async file => new { file = file, runMoc = await FileIsProtoFile(file) }));
            // Wait for the moc files.
            //answerSetTask.Wait();
            //var filterPass = answerSetTask.Result;

            Strings FilteredResolvedSourceFiles = new Strings();
            foreach(string pff in preFilteredFiles)
            {
                if (FileIsProtoFile(pff))
                    FilteredResolvedSourceFiles.Add(pff);
            }

            // These are the files we want to moc.
            //Strings FilteredResolvedSourceFiles = new Strings(filterPass.Where(result => result.runMoc).Select(result => result.file));

            // Compile a list of files where we don't want to compile the moc output.
            List<Regex> filesToExclude = ExcludeMocFromCompileRegex.Select(filter => new Regex(filter, RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase)).ToList();


            foreach (ProjConfiguration conf in project.Configurations)
            {
                // Setup exclusions.
                string QTMocOutputBase = Path.GetDirectoryName(conf.IntermediatePath);
                string targetName = conf.Target.Name;
                string outputFolder = protoDstFolder;

                // We make the current output folder included directly so you can use the same #include directive to get the correct cpp file.
                conf.IncludePrivatePaths.Add(outputFolder);

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
                var mocTargets = new List<ProtoSourceAndTargetFile>();
                foreach (string file in FilteredResolvedSourceFiles)
                {
                    string fileName;
                    string pathName;
                    Util.PathSplitFileNameFromPath(file, out fileName, out pathName);
                    string absPathName = Path.GetFullPath(pathName);
                    string absSrcPathName = Path.GetFullPath(protoSrcFolder);
                    string commonAbsPath = commonAncestor(new List<string>(new string[] { absPathName, absSrcPathName }));
                    string outputLongFolder = outputFolder + pathName.Substring(commonAbsPath.Length, pathName.Length - commonAbsPath.Length) + Path.DirectorySeparatorChar;

                    if(!Util.DirectoryExists(outputLongFolder))
                    {
                        Directory.CreateDirectory(outputLongFolder);
                    }

                    var target = new ProtoSourceAndTargetFile(targetName, protocExe, protoDstFolder, outputLongFolder, file, pathName, fileName);
                    mocTargets.Add(target);
                    if (filesToExclude.Any(filter => filter.IsMatch(file)))
                    {
                        conf.SourceFilesBuildExcludeRegex.Add(Path.GetFileName(target.Output));
                    }
                    if (target.IsProtoFile)
                    {
                        mocTargets.Add(new MocVcxprojBuildStep(target));
                    }
                }
                if (mocTargets.Count > 0)
                {
                    MocTargetsPerConfiguration.Add(conf, mocTargets);
                }
            }

            // Add all the new source files to the project file.
            foreach (var values in MocTargetsPerConfiguration)
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
            List<ProtoSourceAndTargetFile> mocTargets;
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
                if (!target.IsProtoFile)
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
}